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

#ifndef _MOVEPICKER_H
#define _MOVEPICKER_H

#include "types.h"

enum {
    STAGE_TABLE,
    STAGE_GENERATE_NOISY, STAGE_NOISY,
    STAGE_KILLER_1, STAGE_KILLER_2, STAGE_COUNTER_MOVE,
    STAGE_GENERATE_QUIET, STAGE_QUIET,
    STAGE_DONE,
};

struct MovePicker {
    int height;
    int skipQuiets, stage, split;
    int noisySize, quietSize;
    uint16_t tableMove, killer1, killer2, counter;
    uint16_t moves[MAX_MOVES];
    int values[MAX_MOVES];
    Thread *thread;
};

void initializeMovePicker(MovePicker* mp, Thread* thread, uint16_t ttMove, int height, int skipQuiets);
uint16_t selectNextMove(MovePicker* mp, Board* board);
void evaluateNoisyMoves(MovePicker* mp);
void evaluateQuietMoves(MovePicker* mp);
int moveIsPsuedoLegal(Board* board, uint16_t move);

#endif
