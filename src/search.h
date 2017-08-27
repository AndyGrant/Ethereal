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

int aspirationWindow(PVariation * PV, Board * board, MoveList * moveList, 
                                               int depth, int lastScore);

int rootSearch(PVariation * PV, Board * board, MoveList * moveList, int alpha,
                                                         int beta, int depth);

int alphaBetaSearch(PVariation * PV, Board * board, int alpha, int beta, 
                                   int depth, int height, int nodeType);

int quiescenceSearch(Board * board, int alpha, int beta, int height);

void sortMoveList(MoveList * moveList);

int canDoNull(Board * board);

int moveIsTactical(Board * board, uint16_t move);

int hasNonPawnMaterial(Board * board, int turn);

int valueFromTT(int value, int height);

int valueToTT(int value, int height);

static const int LateMovePruningCounts[] = {0, 6, 9, 14, 21, 30, 41, 54, 69};

static const int RazorMargins[] = {0, 450, 480, 520, 580};

#define USE_STATIC_NULL_PRUNING             (1)
#define USE_FUTILITY_PRUNING                (1)
#define USE_NULL_MOVE_PRUNING               (1)
#define USE_LATE_MOVE_REDUCTIONS            (1)
#define USE_INTERNAL_ITERATIVE_DEEPENING    (1)
#define USE_TRANSPOSITION_TABLE             (1)
#define USE_LATE_MOVE_PRUNING               (1)
#define USE_RAZORING                        (1)

#endif