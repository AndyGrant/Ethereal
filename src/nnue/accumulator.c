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


int nnue_can_update(NNUEAccumulator *accum, Board *board, int colour) {

    // Search back through the tree to find an accurate accum
    while (accum != board->thread->nnueStack) {

        // A King move prevents the entire tree from being updated
        if (   accum->changes
            && accum->deltas[0].piece == makePiece(KING, colour))
            return FALSE;

        // Step back, since the root can't be accurate
        accum = accum - 1;

        // We found it, so we can update the entire tree
        if (accum->accurate[colour])
            return TRUE;
    }

    return FALSE;
}

void nnue_refresh_accumulator(NNUEAccumulator *accum, Board *board, int colour, int relsq) {

    const uint64_t white = board->colours[WHITE];
    const uint64_t black = board->colours[BLACK];
    const uint64_t kings = board->pieces[KING];

    int indices[32], count = 0;
    uint64_t pieces = (white | black) & ~kings;
    vepi16 *biases, *outputs, *weights, registers[NUM_REGS];

    // Compute the list of indices just once, to then be used multiple
    // times while updating the accumulator using a tiling method

    while (pieces) {
        const int sq = poplsb(&pieces);
        indices[count++] = nnue_index(board, relsq, colour, sq);
    }

    // Refresh completely, using all pieces as inputs except the Kings
    // We do this by tiling over the accumulator, to get the compiler to
    // produce more optimal code that does not emit extra move instructions

    for (int offset = 0; offset < KPSIZE; offset += NUM_REGS * vepi16_cnt) {

        biases  = (vepi16*) &in_biases[offset];
        outputs = (vepi16*) &accum->values[colour][offset];

        for (int i = 0; i < NUM_REGS; i++)
            registers[i] = biases[i];

        for (int i = 0; i < count; i++) {

            weights = (vepi16*) &in_weights[indices[i] * KPSIZE + offset];

            for (int j = 0; j < NUM_REGS; j++)
                registers[j] = vepi16_add(registers[j], weights[j]);
        }

        for (int i = 0; i < NUM_REGS; i++)
            outputs[i] = registers[i];
    }

    accum->accurate[colour] = TRUE;
}

void nnue_update_accumulator(NNUEAccumulator *accum, Board *board, int colour, int relksq) {

    int add = 0, remove = 0;
    int add_list[3], remove_list[3];
    vepi16 *inputs, *outputs, *weights, registers[NUM_REGS];

    // Recurse and update all out of our date parents
    if (!(accum-1)->accurate[colour])
        nnue_update_accumulator((accum-1), board, colour, relksq);

    // The last move was a NULL move so we can cheat and copy
    if (!accum->changes) {
        memcpy(&accum->values[colour], &(accum-1)->values[colour], sizeof(int16_t) * KPSIZE);
        accum->accurate[colour] = TRUE;
        return;
    }

    // Determine the features that have changed, by looping through them
    for (NNUEDelta *x = &accum->deltas[0]; x < &accum->deltas[0] + accum->changes; x++) {

        // HalfKP does not concern with KxK relations
        if (pieceType(x->piece) == KING)
            continue;

        // Moving or placing a Piece to a Square
        if (x->to != SQUARE_NB)
            add_list[add++] = nnue_index_delta(x->piece, relksq, colour, x->to);

        // Moving or deleting a Piece from a Square
        if (x->from != SQUARE_NB)
            remove_list[remove++] = nnue_index_delta(x->piece, relksq, colour, x->from);
    }

    for (int offset = 0; offset < KPSIZE; offset += NUM_REGS * vepi16_cnt) {

        outputs = (vepi16*) &(accum-0)->values[colour][offset];
        inputs  = (vepi16*) &(accum-1)->values[colour][offset];

        for (int i = 0; i < NUM_REGS; i++)
            registers[i] = inputs[i];

        for (int i = 0; i < add; i++) {

            weights = (vepi16*) &in_weights[add_list[i] * KPSIZE + offset];

            for (int j = 0; j < NUM_REGS; j++)
                registers[j] = vepi16_add(registers[j], weights[j]);
        }

        for (int i = 0; i < remove; i++) {

            weights = (vepi16*) &in_weights[remove_list[i] * KPSIZE + offset];

            for (int j = 0; j < NUM_REGS; j++)
                registers[j] = vepi16_sub(registers[j], weights[j]);
        }

        for (int i = 0; i < NUM_REGS; i++)
            outputs[i] = registers[i];
    }

    accum->accurate[colour] = TRUE;
    return;
}
