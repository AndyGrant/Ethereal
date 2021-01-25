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

#include "types.h"

struct SearchInfo {
    int depth, values[MAX_PLY];
    uint16_t bestMoves[MAX_PLY], ponderMoves[MAX_PLY];
    double startTime, idealUsage, maxAlloc, maxUsage;
    int pvFactor;
};

struct PVariation {
    uint16_t line[MAX_PLY];
    int length;
};

void initSearch();
void getBestMove(Thread *threads, Board *board, Limits *limits, uint16_t *best, uint16_t *ponder);
void* iterativeDeepening(void *vthread);
void aspirationWindow(Thread *thread);
int search(Thread *thread, PVariation *pv, int alpha, int beta, int depth);
int qsearch(Thread *thread, PVariation *pv, int alpha, int beta);
int staticExchangeEvaluation(Board *board, uint16_t move, int threshold);
int singularity(Thread *thread, MovePicker *mp, int ttValue, int depth, int beta);

static const int WindowDepth   = 5;
static const int WindowSize    = 10;
static const int WindowTimerMS = 2500;

static const int CurrmoveTimerMS = 2500;

static const int BetaPruningDepth = 8;
static const int BetaMargin = 85;

static const int AlphaPruningDepth = 5;
static const int AlphaMargin = 3000;

static const int NullMovePruningDepth = 2;

static const int ProbCutDepth = 5;
static const int ProbCutMargin = 80;

static const int FutilityMargin = 65;
static const int FutilityMarginNoHistory = 210;
static const int FutilityPruningDepth = 8;
static const int FutilityPruningHistoryLimit[] = { 12000, 6000 };

static const int CounterMovePruningDepth[] = { 3, 2 };
static const int CounterMoveHistoryLimit[] = { 0, -1000 };

static const int FollowUpMovePruningDepth[] = { 3, 2 };
static const int FollowUpMoveHistoryLimit[] = { -2000, -4000 };

static const int LateMovePruningDepth = 8;

static const int SEEPruningDepth = 9;
static const int SEEQuietMargin = -64;
static const int SEENoisyMargin = -19;
static const int SEEPieceValues[] = {
     100,  450,  450,  675,
    1300,    0,    0,    0,
};

static const int HistexLimit = 10000;

static const int QSSeeMargin = 110;
static const int QSDeltaMargin = 150;

static const int SingularQuietLimit = 6;
static const int SingularTacticalLimit = 3;
