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

#define VERSION_ID "13.00"

#define LICENSE_OWNER "Unlicensed"

#if USE_NNUE
    #define ETHEREAL_VERSION VERSION_ID" (NNUE)"
#elif defined(USE_PEXT)
    #define ETHEREAL_VERSION VERSION_ID" (PEXT)"
#elif defined(USE_POPCNT)
    #define ETHEREAL_VERSION VERSION_ID" (POPCNT)"
#else
    #define ETHEREAL_VERSION VERSION_ID
#endif

struct Limits {
    double start, time, inc, mtg, timeLimit;
    int limitedByNone, limitedByTime, limitedBySelf;
    int limitedByDepth, limitedByMoves, limitedByNodes;
    int multiPV, depthLimit; uint64_t nodeLimit;
    uint16_t searchMoves[MAX_MOVES], excludedMoves[MAX_MOVES];
};

struct UCIGoStruct {
    int multiPV;
    char str[512];
    Board *board;
    Thread *threads;
};

void *uciGo(void *cargo);
void uciSetOption(char *str, Thread **threads, int *multiPV, int *chess960);
void uciPosition(char *str, Board *board, int chess960);

void uciReport(Thread *threads, int alpha, int beta, int value);
void uciReportCurrentMove(Board *board, uint16_t move, int currmove, int depth);

int strEquals(char *str1, char *str2);
int strStartsWith(char *str, char *key);
int strContains(char *str, char *key);
int getInput(char *str);
