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

#define STAGE_TABLE             (0)
#define STAGE_GENERATE_NOISY    (1)
#define STAGE_NOISY             (2)
#define STAGE_KILLER_1          (3)
#define STAGE_KILLER_2          (4)
#define STAGE_GENERATE_QUIET    (5)
#define STAGE_QUIET             (6)
#define STAGE_DONE              (7)

void initalizeMovePicker(MovePicker * mp, int isQuiescencePick,
                          uint16_t tableMove, uint16_t killer1,
                                             uint16_t killer2);

uint16_t selectNextMove(MovePicker * mp, Board * board);

void evaluateNoisyMoves(MovePicker * mp, Board * board);

void evaluateQuietMoves(MovePicker * mp, Board * board);

int moveIsGoodCapture(Board * board, uint16_t move);

int moveIsPsuedoLegal(Board * board, uint16_t move);

#endif