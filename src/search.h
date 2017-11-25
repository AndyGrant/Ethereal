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
int aspirationWindow(PVariation * pv, Board * board, int depth, int values[MAX_DEPTH]);
int search(PVariation * pv, Board * board, int alpha, int beta, int depth, int height);
int qsearch(PVariation * pv, Board * board, int alpha, int beta, int height);
int moveIsTactical(Board * board, uint16_t move);
int hasNonPawnMaterial(Board * board, int turn);
int valueFromTT(int value, int height);
int valueToTT(int value, int height);

static const int RazorDepth = 4;
static const int RazorMargins[] = {0, 450, 480, 520, 580};

static const int BetaPruningDepth = 8;

static const int InternalIterativeDeepeningDepth = 3;

static const int NullMovePruningDepth = 2;

static const int FutilityPruningDepth = 8;

static const int LateMovePruningDepth = 8;
static const int LateMovePruningCounts[] = {0, 5, 7, 11, 16, 24, 33, 43, 56};

#endif
