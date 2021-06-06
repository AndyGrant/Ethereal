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
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>    */
/*                                                                            */
/******************************************************************************/

#include <immintrin.h>
#include <stdio.h>
#include <string.h>

#include "accumulator.h"
#include "nnue.h"
#include "types.h"

#include "../bitboards.h"
#include "../thread.h"
#include "../types.h"

extern ALIGN64 int16_t in_weights[INSIZE * KPSIZE];
extern ALIGN64 int16_t in_biases[KPSIZE];

static int nnue_index_delta(int piece, int relkingidx, int colour, int sq) {

    const int ptype   = pieceType(piece);
    const int pcolour = pieceColour(piece);
    const int relsq   = relativeSquare(colour, sq);

    return relkingidx + (64 * (5 * (colour == pcolour) + ptype)) + relsq;
}

static int nnue_index(Board *board, int relkingidx, int colour, int sq) {
    return nnue_index_delta(board->squares[sq], relkingidx, colour, sq);
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

void nnue_refresh_accumulators(NNUEAccumulator *accum, Board *board, int wkingidx, int bkingidx) {
    nnue_refresh_accumulator(accum, board, WHITE, wkingidx);
    nnue_refresh_accumulator(accum, board, BLACK, bkingidx);
    accum->accurate = 1;
}

void nnue_refresh_accumulator(NNUEAccumulator *accum, Board *board, int colour, int kingidx) {

    const uint64_t white = board->colours[WHITE];
    const uint64_t black = board->colours[BLACK];
    const uint64_t kings = board->pieces[KING];

    uint64_t pieces = (white | black) & ~kings;

    vepi16* biases  = (vepi16*) &in_biases[0];
    vepi16* outputs = (vepi16*) &accum->values[colour][0];

    // We can assert that this position is not KvK, and therefore to
    // slightly optimize the AVX code, we can seperate out the very
    // first piece and use it to initialize the outpt with the biases

    {
        int index = nnue_index(board, kingidx, colour, poplsb(&pieces));
        vepi16* inputs  = (vepi16*) &in_weights[index * KPSIZE];

        for (int i = 0; i < KPSIZE / vepi16_cnt; i += 4) {
            outputs[i+0] = vepi16_add(biases[i+0], inputs[i+0]);
            outputs[i+1] = vepi16_add(biases[i+1], inputs[i+1]);
            outputs[i+2] = vepi16_add(biases[i+2], inputs[i+2]);
            outputs[i+3] = vepi16_add(biases[i+3], inputs[i+3]);
        }
    }

    // Add up the remainder of the pieces to the accumulator. We batch
    // updates into 4x16 at a timein order to get optimal assembly output

    while (pieces) {

        int index = nnue_index(board, kingidx, colour, poplsb(&pieces));
        vepi16* inputs  = (vepi16*) &in_weights[index * KPSIZE];

        for (int i = 0; i < KPSIZE / vepi16_cnt; i += 4) {
            outputs[i+0] = vepi16_add(outputs[i+0], inputs[i+0]);
            outputs[i+1] = vepi16_add(outputs[i+1], inputs[i+1]);
            outputs[i+2] = vepi16_add(outputs[i+2], inputs[i+2]);
            outputs[i+3] = vepi16_add(outputs[i+3], inputs[i+3]);
        }
    }
}

void nnue_update_accumulator(NNUEAccumulator *accum, Board *board, int wkingidx, int bkingidx) {

    // Recurse and update all out of date children
    if (!(accum-1)->accurate)
        nnue_update_accumulator((accum-1), board, wkingidx, bkingidx);

    // The last move was a NULL move so we can cheat and copy
    if (!accum->changes) {
        memcpy(accum->values, (accum-1)->values, sizeof(int16_t) * L1SIZE);
        accum->accurate = 1;
        return;
    }

    // ------------------------------------------------------------------------------------------

    int add_list[2][3], remove_list[2][3];
    int add = 0, remove = 0, refreshed[2] = { 0, 0 };

    for (int i = 0; i < accum->changes; i++) {

        const NNUEDelta *delta = &accum->deltas[i];

        // Fully recompute a colour if its King has moved
        if (pieceType(delta->piece) == KING) {
            int colour = pieceColour(delta->piece);
            int relkingidx = colour == WHITE ? wkingidx : bkingidx;
            nnue_refresh_accumulator(accum, board, colour, relkingidx);
            refreshed[colour] = 1;
            continue;
        }

        // Moving or placing a Piece to a Square
        if (delta->to != SQUARE_NB) {
            add_list[WHITE][add  ] = nnue_index_delta(delta->piece, wkingidx, WHITE, delta->to);
            add_list[BLACK][add++] = nnue_index_delta(delta->piece, bkingidx, BLACK, delta->to);
        }

        // Moving or deleting a Piece from a Square
        if (delta->from != SQUARE_NB) {
            remove_list[WHITE][remove  ] = nnue_index_delta(delta->piece, wkingidx, WHITE, delta->from);
            remove_list[BLACK][remove++] = nnue_index_delta(delta->piece, bkingidx, BLACK, delta->from);
        }
    }

    // ------------------------------------------------------------------------------------------


    if (!add && !remove) {
        int old = refreshed[WHITE] ? BLACK : WHITE;
        memcpy(accum->values[old], (accum-1)->values[old], sizeof(int16_t) * KPSIZE);
        accum->accurate = 1;
        return;
    }

    for (int colour = WHITE; colour <= BLACK; colour++) {

        if (refreshed[colour])
            continue;

        vepi16* outputs = (vepi16*) &accum->values[colour][0];
        vepi16* priors  = (vepi16*) &(accum-1)->values[colour][0];

        {
            const int index = remove_list[colour][0];
            vepi16* inputs  = (vepi16*) &in_weights[index * KPSIZE];

            for (int j = 0; j < KPSIZE / vepi16_cnt; j += 4) {
                outputs[j+0] = vepi16_sub(priors[j+0], inputs[j+0]);
                outputs[j+1] = vepi16_sub(priors[j+1], inputs[j+1]);
                outputs[j+2] = vepi16_sub(priors[j+2], inputs[j+2]);
                outputs[j+3] = vepi16_sub(priors[j+3], inputs[j+3]);
            }
        }

        for (int i = 1; i < remove; i++) {

            const int index = remove_list[colour][i];
            vepi16* inputs  = (vepi16*) &in_weights[index * KPSIZE];

            for (int j = 0; j < KPSIZE / vepi16_cnt; j += 4) {
                outputs[j+0] = vepi16_sub(outputs[j+0], inputs[j+0]);
                outputs[j+1] = vepi16_sub(outputs[j+1], inputs[j+1]);
                outputs[j+2] = vepi16_sub(outputs[j+2], inputs[j+2]);
                outputs[j+3] = vepi16_sub(outputs[j+3], inputs[j+3]);
            }
        }

        for (int i = 0; i < add; i++) {

            const int index = add_list[colour][i];
            vepi16* inputs  = (vepi16*) &in_weights[index * KPSIZE];

            for (int j = 0; j < KPSIZE / vepi16_cnt; j += 4) {
                outputs[j+0] = vepi16_add(outputs[j+0], inputs[j+0]);
                outputs[j+1] = vepi16_add(outputs[j+1], inputs[j+1]);
                outputs[j+2] = vepi16_add(outputs[j+2], inputs[j+2]);
                outputs[j+3] = vepi16_add(outputs[j+3], inputs[j+3]);
            }
        }
    }

    accum->accurate = 1;
    return;
}

