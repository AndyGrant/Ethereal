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

#include "types.h"

enum { NORMAL_PICKER, NOISY_PICKER };

enum {
    STAGE_TABLE,
    STAGE_GENERATE_NOISY, STAGE_GOOD_NOISY,
    STAGE_KILLER_1, STAGE_KILLER_2, STAGE_COUNTER_MOVE,
    STAGE_GENERATE_QUIET, STAGE_QUIET,
    STAGE_BAD_NOISY,
    STAGE_DONE,
};

struct MovePicker {
    int split, noisy_size, quiet_size;
    int stage, type, threshold;
    int values[MAX_MOVES];
    uint16_t moves[MAX_MOVES];
    uint16_t tt_move, killer1, killer2, counter;
};

void     init_picker       (MovePicker *mp, Thread *thread, uint16_t tt_move);
void     init_noisy_picker (MovePicker *mp, Thread *thread, uint16_t tt_move, int threshold);
uint16_t select_next       (MovePicker *mp, Thread *thread, int skip_quiets);
