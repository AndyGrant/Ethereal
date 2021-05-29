/******************************************************************************/
/*                                                                            */
/*    Ethereal is a UCI chess playing engine authored by Andrew Grant.        */
/*    <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>        */
/*                                                                            */
/*    Ethereal is free software: you can redistribute it and/or modify        */
/*    it under the terms of the GNU General Public License as published by    */
/*    the Free Software Foundation, either version 3 of the License, or       */
/*    (at your option) any later version.                                     */
/*                                                                            */
/*    Ethereal is distributed in the hope that it will be useful,             */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*    GNU General Public License for more details.                            */
/*                                                                            */
/*    You should have received a copy of the GNU General Public License       */
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>.   */
/*                                                                            */
/******************************************************************************/

#include <immintrin.h>
#include <stdio.h>
#include <string.h>

#include "accumulator.h"
#include "nnue.h"
#include "types.h"

#include "../bitboards.h"
#include "../board.h"
#include "../thread.h"
#include "../types.h"

extern ALIGN64 int16_t in_weights[INSIZE * KPSIZE ];
extern ALIGN64 int16_t in_biases[KPSIZE ];

static int nnue_index(Board *board, int relksq, int colour, int sq) {

    const int relsq   = relativeSquare(colour, sq);
    const int ptype   = pieceType(board->squares[sq]);
    const int pcolour = pieceColour(board->squares[sq]);

    return (640 * relksq) + (64 * (5 * (colour == pcolour) + ptype)) + (relsq);
}

static int nnue_index_delta(int piece, int relksq, int colour, int sq) {

    const int relsq   = relativeSquare(colour, sq);
    const int ptype   = pieceType(piece);
    const int pcolour = pieceColour(piece);

    return (640 * relksq) + (64 * (5 * (colour == pcolour) + ptype)) + (relsq);
}


int nnue_can_update(NNUEAccumulator *accum, Board *board) {

    NNUEAccumulator *start = board->thread->nnueStack;

    if (accum == start) return 0;

    if ((accum-1)->accurate) return 1;

    while (accum != start) {

        if (accum->accurate)
            return 1;

        if (accum->changes && pieceType(accum->deltas[0].piece) == KING)
            return 0;

        accum = accum - 1;
    }

    return 0;
}

void nnue_refresh_accumulators(NNUEAccumulator *accum, Board *board) {
    nnue_refresh_accumulator(accum, board, WHITE);
    nnue_refresh_accumulator(accum, board, BLACK);
    accum->accurate = 1;
}

void nnue_refresh_accumulator(NNUEAccumulator *accum, Board *board, int colour) {

    const uint64_t white = board->colours[WHITE];
    const uint64_t black = board->colours[BLACK];
    const uint64_t kings = board->pieces[KING];

    uint64_t pieces = (white | black) & ~kings;
    int relksq = relativeSquare(colour, board->ksquares[colour]);

    {
        const int index = nnue_index(board, relksq, colour, poplsb(&pieces));

        __m256i* inputs  = (__m256i*) &in_weights[index * KPSIZE];
        __m256i* outputs = (__m256i*) &accum->values[colour][0];
        __m256i* biases  = (__m256i*) &in_biases[0];

        for (int i = 0; i < KPSIZE / 16; i += 4) {
            outputs[i+0] = _mm256_add_epi16(biases[i+0], inputs[i+0]);
            outputs[i+1] = _mm256_add_epi16(biases[i+1], inputs[i+1]);
            outputs[i+2] = _mm256_add_epi16(biases[i+2], inputs[i+2]);
            outputs[i+3] = _mm256_add_epi16(biases[i+3], inputs[i+3]);
        }
    }

    while (pieces) {

        const int index = nnue_index(board, relksq, colour, poplsb(&pieces));

        __m256i* inputs  = (__m256i*) &in_weights[index * KPSIZE];
        __m256i* outputs = (__m256i*) &accum->values[colour][0];

        for (int i = 0; i < KPSIZE / 16; i += 4) {
            outputs[i+0] = _mm256_add_epi16(outputs[i+0], inputs[i+0]);
            outputs[i+1] = _mm256_add_epi16(outputs[i+1], inputs[i+1]);
            outputs[i+2] = _mm256_add_epi16(outputs[i+2], inputs[i+2]);
            outputs[i+3] = _mm256_add_epi16(outputs[i+3], inputs[i+3]);
        }
    }
}

void nnue_update_accumulator(NNUEAccumulator *accum, Board *board) {

    // Recurse and update all children
    if (!(accum-1)->accurate)
        nnue_update_accumulator(accum-1, board);

    // Null move from a (now) accurate Node
    if (!accum->changes) {
        memcpy(accum->values[WHITE], (accum-1)->values[WHITE], sizeof(int16_t) * KPSIZE);
        memcpy(accum->values[BLACK], (accum-1)->values[BLACK], sizeof(int16_t) * KPSIZE);
        goto FINISHED;
    }

    // ------------------------------------------------------------------------------------------

    NNUEDelta *deltas = accum->deltas;
    int add_list[2][3], remove_list[2][3];
    int add = 0, remove = 0, refreshed[2] = { 0, 0 };

    int wkrelsq = relativeSquare(WHITE, board->ksquares[WHITE]);
    int bkrelsq = relativeSquare(BLACK, board->ksquares[BLACK]);

    for (int i = 0; i < accum->changes; i++) {

        const NNUEDelta *delta = &accum->deltas[i];

        // Hard recompute a colour if their King has moved
        if (pieceType(delta->piece) == KING) {
            nnue_refresh_accumulator(accum, board, pieceColour(delta->piece));
            refreshed[pieceColour(delta->piece)] = 1;
            continue;
        }

        // Moving (or Placing) a Piece to a Square
        if (delta->to != SQUARE_NB) {
            add_list[WHITE][add  ] = nnue_index_delta(deltas[i].piece, wkrelsq, WHITE, delta->to);
            add_list[BLACK][add++] = nnue_index_delta(deltas[i].piece, bkrelsq, BLACK, delta->to);
        }

        // Moving (or Deleting) a Piece from a Square
        if (delta->from != SQUARE_NB) {
            remove_list[WHITE][remove  ] = nnue_index_delta(delta->piece, wkrelsq, WHITE, delta->from);
            remove_list[BLACK][remove++] = nnue_index_delta(delta->piece, bkrelsq, BLACK, delta->from);
        }
    }

    // ------------------------------------------------------------------------------------------

    for (int colour = WHITE; colour <= BLACK; colour++) {

        if (refreshed[colour])
            continue;

        memcpy(accum->values[colour], (accum-1)->values[colour], sizeof(int16_t) * KPSIZE);

        for (int i = 0; i < add; i++) {

            const int index = add_list[colour][i];
            __m256i* inputs  = (__m256i*) &in_weights[index * KPSIZE];
            __m256i* outputs = (__m256i*) &accum->values[colour][0];

            for (int j = 0; j < KPSIZE / 16; j += 4) {
                outputs[j+0] = _mm256_add_epi16(outputs[j+0], inputs[j+0]);
                outputs[j+1] = _mm256_add_epi16(outputs[j+1], inputs[j+1]);
                outputs[j+2] = _mm256_add_epi16(outputs[j+2], inputs[j+2]);
                outputs[j+3] = _mm256_add_epi16(outputs[j+3], inputs[j+3]);
            }
        }

        for (int i = 0; i < remove; i++) {

            const int index = remove_list[colour][i];
            __m256i* inputs  = (__m256i*) &in_weights[index * KPSIZE];
            __m256i* outputs = (__m256i*) &accum->values[colour][0];

            for (int j = 0; j < KPSIZE / 16; j += 4) {
                outputs[j+0] = _mm256_sub_epi16(outputs[j+0], inputs[j+0]);
                outputs[j+1] = _mm256_sub_epi16(outputs[j+1], inputs[j+1]);
                outputs[j+2] = _mm256_sub_epi16(outputs[j+2], inputs[j+2]);
                outputs[j+3] = _mm256_sub_epi16(outputs[j+3], inputs[j+3]);
            }
        }
    }

    FINISHED:
        accum->accurate = 1;
        return;
}

