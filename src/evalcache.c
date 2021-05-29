/*
  Ethereal is a UCI chess playing engine authored by Andrew Grant.
  <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>
  Ethereal is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  Ethereal is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdlib.h>

#include "board.h"
#include "evaluate.h"
#include "thread.h"
#include "types.h"
#include "zobrist.h"

int getCachedEvaluation(Thread *thread, Board *board, int *eval) {

    EvalEntry eve;
    uint64_t key;

    eve =  thread->evtable[board->hash & EVAL_CACHE_MASK];
    key = (eve & ~0xFFFF) | (board->hash & 0xFFFF);

    *eval = (int16_t)((uint16_t)(eve & 0xFFFF));
    *eval = board->turn == WHITE ? *eval : -*eval;
    return board->hash == key;
}

void storeCachedEvaluation(Thread *thread, Board *board, int eval) {
    thread->evtable[board->hash & EVAL_CACHE_MASK]
        = (board->hash & ~0xFFFF) | (uint16_t)((int16_t)eval);
}


PKEntry* getCachedPawnKingEval(Thread *thread, Board *board) {
    PKEntry *pke = &thread->pktable[board->pkhash & PK_CACHE_MASK];
    return pke->pkhash == board->pkhash ? pke : NULL;
}

void storeCachedPawnKingEval(Thread *thread, Board *board, uint64_t passed, int eval, int safetyw, int safetyb) {
    PKEntry *pke = &thread->pktable[board->pkhash & PK_CACHE_MASK];
    *pke = (PKEntry) {board->pkhash, passed, eval, safetyw, safetyb};
}

