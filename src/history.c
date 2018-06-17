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
#include "thread.h"
#include "types.h"

void updateHistory(HistoryTable history, uint16_t move, int colour, int delta) {

    int entry;
    int from  = MoveFrom(move);
    int to    = MoveTo(move);

    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= from && from < SQUARE_NB);
    assert(0 <= to && to < SQUARE_NB);

    // Bound the update. Weight the change so that the
    // new entry value is within the range of int16_t
    delta = MAX(-400, MIN(400, delta));
    entry = history[colour][from][to];
    entry += 32 * delta - entry * abs(delta) / 512;
    history[colour][from][to] = entry;
}

int getHistoryScore(HistoryTable history, uint16_t move, int colour) {

    int from = MoveFrom(move);
    int to   = MoveTo(move);

    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= from && from < SQUARE_NB);
    assert(0 <= to && to < SQUARE_NB);

    return history[colour][from][to];
}

void updateCounterMove(Thread *thread, int height, uint16_t move) {

    uint16_t previous = thread->moveStack[height-1];

    // Check for root position or null moves
    if (previous == NULL_MOVE || previous == NONE_MOVE)
        return;

    int colour = !thread->board.turn;
    int to     = MoveTo(previous);
    int piece  = pieceType(thread->board.squares[to]);

    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= piece && piece < PIECE_NB);
    assert(0 <= to && to < SQUARE_NB);

    thread->cmtable[colour][piece][to] = move;
}

uint16_t getCounterMove(Thread *thread, int height) {

    uint16_t previous = thread->moveStack[height-1];

    // Check for root position or null moves
    if (previous == NULL_MOVE || previous == NONE_MOVE)
        return NONE_MOVE;

    int colour = !thread->board.turn;
    int to     = MoveTo(previous);
    int piece  = pieceType(thread->board.squares[to]);

    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= piece && piece < PIECE_NB);
    assert(0 <= to && to < SQUARE_NB);

    return thread->cmtable[colour][piece][to];
}
