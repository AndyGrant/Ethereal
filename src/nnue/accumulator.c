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


static int sq64_to_sq32(int sq) {
    static const int Mirror[] = { 3, 2, 1, 0, 0, 1, 2, 3 };
    return ((sq >> 1) & ~0x3) + Mirror[sq & 0x7];
}

static int nnue_index_delta(int piece, int relksq, int colour, int sq) {

    const int ptype   = pieceType(piece);
    const int pcolour = pieceColour(piece);
    const int relpsq  = relativeSquare(colour, sq);

    const int mksq = testBit(LEFT_FLANK, relksq) ? (relksq ^ 0x7) : relksq;
    const int mpsq = testBit(LEFT_FLANK, relksq) ? (relpsq ^ 0x7) : relpsq;

    return 640 * sq64_to_sq32(mksq) + (64 * (5 * (colour == pcolour) + ptype)) + mpsq;
}

static int nnue_index(Board *board, int relksq, int colour, int sq) {
    return nnue_index_delta(board->squares[sq], relksq, colour, sq);
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

void nnue_refresh_accumulators(NNUEAccumulator *accum, Board *board, int wrelksq, int brelksq) {
    nnue_refresh_accumulator(accum, board, WHITE, wrelksq);
    nnue_refresh_accumulator(accum, board, BLACK, brelksq);
    accum->accurate = 1;
}

void nnue_refresh_accumulator(NNUEAccumulator *accum, Board *board, int colour, int relsq) {

    const uint64_t white = board->colours[WHITE];
    const uint64_t black = board->colours[BLACK];
    const uint64_t kings = board->pieces[KING];

    uint64_t pieces = (white | black) & ~kings;

    vepi16* biases  = (vepi16*) &in_biases[0];
    vepi16* outputs = (vepi16*) &accum->values[colour][0];

    // We can assert that this position is not KvK, and therefore to
    // slightly optimize the AVX code, we can seperate out the very
    // first piece and use it to initialize the output with the biases

    {
        int index = nnue_index(board, relsq, colour, poplsb(&pieces));
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

        int index = nnue_index(board, relsq, colour, poplsb(&pieces));
        vepi16* inputs  = (vepi16*) &in_weights[index * KPSIZE];

        for (int i = 0; i < KPSIZE / vepi16_cnt; i += 4) {
            outputs[i+0] = vepi16_add(outputs[i+0], inputs[i+0]);
            outputs[i+1] = vepi16_add(outputs[i+1], inputs[i+1]);
            outputs[i+2] = vepi16_add(outputs[i+2], inputs[i+2]);
            outputs[i+3] = vepi16_add(outputs[i+3], inputs[i+3]);
        }
    }
}

void nnue_update_accumulator(NNUEAccumulator *accum, Board *board, int wrelksq, int brelksq) {

    // Recurse and update all out of date children
    if (!(accum-1)->accurate)
        nnue_update_accumulator((accum-1), board, wrelksq, brelksq);

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
            int relksq = colour == WHITE ? wrelksq : brelksq;
            nnue_refresh_accumulator(accum, board, colour, relksq);
            refreshed[colour] = 1;
            continue;
        }

        // Moving or placing a Piece to a Square
        if (delta->to != SQUARE_NB) {
            add_list[WHITE][add  ] = nnue_index_delta(delta->piece, wrelksq, WHITE, delta->to);
            add_list[BLACK][add++] = nnue_index_delta(delta->piece, brelksq, BLACK, delta->to);
        }

        // Moving or deleting a Piece from a Square
        if (delta->from != SQUARE_NB) {
            remove_list[WHITE][remove  ] = nnue_index_delta(delta->piece, wrelksq, WHITE, delta->from);
            remove_list[BLACK][remove++] = nnue_index_delta(delta->piece, brelksq, BLACK, delta->from);
        }
    }

    // ------------------------------------------------------------------------------------------


    if (!add && !remove) {
        int old = refreshed[WHITE] ? BLACK : WHITE;
        memcpy(accum->values[old], (accum-1)->values[old], sizeof(int16_t) * KPSIZE);
        accum->accurate = 1;
        return;
    }

    vepi16 registers[16];

    for (int colour = WHITE; colour <= BLACK; colour++) {

        if (refreshed[colour])
            continue;

        int16_t* dest = &accum->values[colour][0];
        int16_t* src  = &(accum-1)->values[colour][0];

        for (int o = 0; o < KPSIZE; o += 16 * vepi16_cnt) {
            const vepi16* inputs  = (vepi16*) &src[o];
                  vepi16* outputs = (vepi16*) &dest[o];
            
            for (int i = 0; i < 16; i++)
                registers[i] = vepi16_load(&inputs[i]);
            
            for (int r = 0; r < remove; r++) {
                const int index = remove_list[colour][r];
                const vepi16* weights = (vepi16*) &in_weights[index * KPSIZE + o];

                for (int i = 0; i < 16; i++)
                    registers[i] = vepi16_sub(registers[i], weights[i]);
            }

            for (int a = 0; a < add; a++) {
                const int index = add_list[colour][a];
                const vepi16* weights = (vepi16*) &in_weights[index * KPSIZE + o];

                for (int i = 0; i < 16; i++)
                    registers[i] = vepi16_add(registers[i], weights[i]);
            }

            for (int i = 0; i < 16; i++)
                vepi16_store(&outputs[i], registers[i]);
        }
    }

    accum->accurate = 1;
    return;
}
