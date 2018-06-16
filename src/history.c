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

#include <stdint.h>
#include <stdlib.h>

#include "board.h"
#include "history.h"
#include "move.h"
#include "types.h"

void updateHistory(HistoryTable history, uint16_t move, int colour, int delta){

    int from  = MoveFrom(move);
    int to    = MoveTo(move);
    int entry = history[colour][from][to];

    // Ensure the update value is within [-400, 400]
    delta = MAX(-400, MIN(400, delta));

    // Ensure the new value is within [-16384, 16384]
    entry += 32 * delta - entry * abs(delta) / 512;

    // Save back the adjusted history score
    history[colour][from][to] = entry;
}

int getHistoryScore(HistoryTable history, uint16_t move, int colour){
    return history[colour][MoveFrom(move)][MoveTo(move)];
}
