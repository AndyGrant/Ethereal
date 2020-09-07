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

#pragma once

#include <stdint.h>

#include "board.h"
#include "types.h"

enum {
    EVAL_CACHE_KEY_SIZE = 16,
    EVAL_CACHE_MASK     = 0xFFFF,
    EVAL_CACHE_SIZE     = 1 << EVAL_CACHE_KEY_SIZE,
};

enum {
    PK_CACHE_KEY_SIZE   = 16,
    PK_CACHE_MASK       = 0xFFFF,
    PK_CACHE_SIZE       = 1 << PK_CACHE_KEY_SIZE,
};

typedef uint64_t EvalEntry;
typedef EvalEntry EvalTable[EVAL_CACHE_SIZE];

struct PKEntry { uint64_t pkhash, passed; int eval; };
typedef PKEntry PKTable[PK_CACHE_SIZE];

int getCachedEvaluation(Thread *thread, Board *board, int *eval);
void storeCachedEvaluation(Thread *thread, Board *board, int eval);

PKEntry* getCachedPawnKingEval(Thread *thread, Board *board);
void storeCachedPawnKingEval(Thread *thread, Board *board, uint64_t passed, int eval);