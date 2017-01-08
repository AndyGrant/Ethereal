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

#ifndef _SEARCH_H
#define _SEARCH_H

#include <stdint.h>

#include "types.h"

uint16_t getBestMove(SearchInfo * info);
int aspirationWindow(Board * board, MoveList * moveList, int depth, int previousScore);
int rootSearch(Board * board, MoveList * moveList, int alpha, int beta, int depth);
int alphaBetaSearch(Board * board, int alpha, int beta, int depth, int height, int nodeType);
int quiescenceSearch(Board * board, int alpha, int beta, int height);
void evaluateMoves(Board * board, int * values, uint16_t * moves, int size, int height, uint16_t tableMove);
uint16_t getNextMove(uint16_t * moves, int * values, int index, int size);
void sortMoveList(MoveList * moveList);
int canDoNull(Board * board);

#define USE_STATIC_NULL_PRUNING             (1)
#define USE_FUTILITY_PRUNING                (1)
#define USE_NULL_MOVE_PRUNING               (1)
#define USE_LATE_MOVE_REDUCTIONS            (1)
#define USE_INTERNAL_ITERATIVE_DEEPENING    (1)
#define USE_TRANSPOSITION_TABLE             (1)

#endif