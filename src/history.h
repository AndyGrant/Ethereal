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

static const int HistoryMax = 400;
static const int HistoryMultiplier = 32;
static const int HistoryDivisor = 512;

void updateHistoryHeuristics(Thread *thread, uint16_t *moves, int length, int depth);
void updateKillerMoves(Thread *thread, uint16_t move);

void updateCaptureHistories(Thread *thread, uint16_t best, uint16_t *moves, int length, int depth);
void getCaptureHistories(Thread *thread, uint16_t *moves, int *scores, int start, int length);
int getCaptureHistory(Thread *thread, uint16_t move);

int getHistory(Thread *thread, uint16_t move, int *cmhist, int *fmhist);
void getHistoryScores(Thread *thread, uint16_t *moves, int *scores, int start, int length);
void getRefutationMoves(Thread *thread, uint16_t *killer1, uint16_t *killer2, uint16_t *counter);
