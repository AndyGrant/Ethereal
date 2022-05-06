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

static const int HistoryDivisor = 16384;

void update_history_heuristics(Thread *thread, uint16_t *moves, int length, int depth);
void update_killer_moves(Thread *thread, uint16_t move);
void get_refutation_moves(Thread *thread, uint16_t *killer1, uint16_t *killer2, uint16_t *counter);

int  get_capture_history(Thread *thread, uint16_t move);
void get_capture_histories(Thread *thread, uint16_t *moves, int *scores, int start, int length);
void update_capture_histories(Thread *thread, uint16_t best, uint16_t *moves, int length, int depth);

int  get_quiet_history(Thread *thread, uint16_t move, int *cmhist, int *fmhist);
void get_quiet_histories(Thread *thread, uint16_t *moves, int *scores, int start, int length);
void update_quiet_histories(Thread *thread, uint16_t *moves, int length, int depth);
