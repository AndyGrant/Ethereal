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

#pragma once

#include "types.h"
#include "utils.h"

#include "../board.h"
#include "../thread.h"
#include "../types.h"

extern ALIGN64 int16_t in_weights[INSIZE * KPSIZE];
extern ALIGN64 int16_t in_biases[KPSIZE];

INLINE NNUEEvaluator* nnue_create_evaluator() {
    return align_malloc(sizeof(NNUEEvaluator));
}

INLINE void nnue_reset_evaluator(NNUEEvaluator* ptr) {

    #if USE_NNUE

        // Reset the Finny table Accumulators
        for (size_t i = 0; i < SQUARE_NB; i++) {
            memset(ptr->table[i].occupancy, 0, sizeof(ptr->table[i].occupancy));
            memcpy(ptr->table[i].accumulator.values[WHITE], in_biases, sizeof(int16_t) * KPSIZE);
            memcpy(ptr->table[i].accumulator.values[BLACK], in_biases, sizeof(int16_t) * KPSIZE);
        }

        // Reset the base of the Accumulator stack
        ptr->current = &ptr->stack[0];
        ptr->current->accurate[WHITE] = 0;
        ptr->current->accurate[BLACK] = 0;

    #endif
}

INLINE void nnue_delete_evaluator(NNUEEvaluator* ptr) {
    align_free(ptr);
}


INLINE void nnue_pop(Board *board) {
    if (USE_NNUE && board->thread != NULL)
        --board->thread->nnue->current;
}

INLINE void nnue_push(Board *board) {
    if (USE_NNUE && board->thread != NULL) {
        NNUEAccumulator *accum = ++board->thread->nnue->current;
        accum->accurate[WHITE] = accum->accurate[BLACK] = FALSE;
        accum->changes = 0;
    }
}

INLINE void nnue_move_piece(Board *board, int piece, int from, int to) {
    if (USE_NNUE && board->thread != NULL) {
        NNUEAccumulator *accum = board->thread->nnue->current;
        accum->deltas[accum->changes++] = (NNUEDelta) { piece, from, to };
    }
}

INLINE void nnue_add_piece(Board *board, int piece, int sq) {
    nnue_move_piece(board, piece, SQUARE_NB, sq);
}

INLINE void nnue_remove_piece(Board *board, int piece, int sq) {
    if (piece != EMPTY)
        nnue_move_piece(board, piece, sq, SQUARE_NB);
}

int nnue_can_update(NNUEAccumulator *accum, Board *board, int colour);
void nnue_update_accumulator(NNUEAccumulator *accum, Board *board, int colour, int relksq);
void nnue_refresh_accumulator(NNUEEvaluator *nnue, NNUEAccumulator *accum, Board *board, int colour, int relksq);
