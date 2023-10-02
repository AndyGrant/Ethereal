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

static const int WindowDepth   = 5;
static const int WindowSize    = 10;
static const int WindowTimerMS = 2500;

static const int CurrmoveTimerMS = 2500;

static const int TTResearchMargin = 131;

static const int BetaPruningDepth = 8;
static const int BetaMargin = 63;

static const int AlphaPruningDepth = 5;
static const int AlphaMargin = 3193;

static const int NullMovePruningDepth = 2;

static const int ProbCutDepth = 5;
static const int ProbCutMargin = 98;

static const int FutilityPruningDepth = 8;
static const int FutilityMarginBase = 88;
static const int FutilityMarginPerDepth = 52;
static const int FutilityMarginNoHistory = 153;
static const int FutilityPruningHistoryLimit[] = { 13261, 6004 };

static const int ContinuationPruningDepth[] = { 3, 2 };
static const int ContinuationPruningHistoryLimit[] = { -1000, -2500 };

static const int LateMovePruningDepth = 8;

static const int SEEPruningDepth = 9;
static const int SEEQuietMargin = -64;
static const int SEENoisyMargin = -19;
static const int SEEPieceValues[] = {
     105,  416,  449,  668,
    1328,    0,    0,    0,
};

static const int QSSeeMargin = 115;
static const int QSDeltaMargin = 149;
