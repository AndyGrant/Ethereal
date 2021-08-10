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

INLINE NNUEAccumulator* nnue_create_accumulators() {
    return align_malloc(sizeof(NNUEAccumulator) * (MAX_PLY + 4));
}

INLINE void nnue_delete_accumulators(NNUEAccumulator* ptr) {
    align_free(ptr);
}

INLINE void nnue_push(Board *board) {
    if (USE_NNUE && board->thread != NULL) {
        const int height = board->thread->height;
        NNUEAccumulator *accum = &board->thread->nnueStack[height+1];
        accum->accurate = 0; accum->changes = 0;
    }
}

INLINE void nnue_move_piece(Board *board, int piece, int from, int to) {
    if (USE_NNUE && board->thread != NULL) {
        const int height = board->thread->height;
        NNUEAccumulator *accum = &board->thread->nnueStack[height+1];
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

int nnue_can_update(NNUEAccumulator *accum, Board *board);
void nnue_refresh_accumulators(NNUEAccumulator *accum, Board *board, int wrelksq, int brelksq);
void nnue_refresh_accumulator(NNUEAccumulator *accum, Board *board, int colour, int relksq);
void nnue_update_accumulator(NNUEAccumulator *accum, Board *board, int wrelksq, int brelksq);
