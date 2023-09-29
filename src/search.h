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

#include <stdbool.h>
#include <stdint.h>

#include "types.h"

struct PVariation {
    int length, score;
    uint16_t line[MAX_PLY];
};

void initSearch();
void *start_search_threads(void *arguments);
void getBestMove(Thread *threads, Board *board, Limits *limits, uint16_t *best, uint16_t *ponder, int *score);
void* iterativeDeepening(void *vthread);
void aspirationWindow(Thread *thread);
int search(Thread *thread, PVariation *pv, int alpha, int beta, int depth, bool cutnode);
int qsearch(Thread *thread, PVariation *pv, int alpha, int beta);
int staticExchangeEvaluation(Board *board, uint16_t move, int threshold);
int singularity(Thread *thread, uint16_t ttMove, int ttValue, int depth, int PvNode, int alpha, int beta, bool cutnode);
