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

int getHistoryScore(Thread *thread, uint16_t move) {

    int colour = thread->board.turn;
    int from   = MoveFrom(move);
    int to     = MoveTo(move);

    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= from && from < SQUARE_NB);
    assert(0 <= to && to < SQUARE_NB);

    return thread->history[colour][from][to];
}

void updateHistory(Thread *thread, uint16_t move, int delta) {

    int entry;
    int colour = thread->board.turn;
    int from  = MoveFrom(move);
    int to    = MoveTo(move);

    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= from && from < SQUARE_NB);
    assert(0 <= to && to < SQUARE_NB);

    delta = MAX(-400, MIN(400, delta));

    entry = thread->history[colour][from][to];
    entry += 32 * delta - entry * abs(delta) / 512;
    thread->history[colour][from][to] = entry;
}

int getCMHistoryScore(Thread *thread, int height, uint16_t move) {

    int to1, to2, piece1, piece2;
    uint16_t previous = thread->moveStack[height-1];

    // Check for root position or null moves
    if (previous == NULL_MOVE || previous == NONE_MOVE)
        return NONE_MOVE;

    to1    = MoveTo(previous);
    piece1 = pieceType(thread->board.squares[to1]);

    to2    = MoveTo(move);
    piece2 = pieceType(thread->board.squares[MoveFrom(move)]);

    assert(0 <= piece1 && piece1 < PIECE_NB);
    assert(0 <= to1 && to1 < SQUARE_NB);
    assert(0 <= piece2 && piece2 < PIECE_NB);
    assert(0 <= to2 && to2 < SQUARE_NB);

    return thread->cmhistory[piece1][to1][piece2][to2];
}

void updateCMHistory(Thread *thread, int height, uint16_t move, int delta) {

    int entry, to1, to2, piece1, piece2;
    uint16_t previous = thread->moveStack[height-1];

    // Check for root position or null moves
    if (previous == NULL_MOVE || previous == NONE_MOVE)
        return;

    to1    = MoveTo(previous);
    piece1 = pieceType(thread->board.squares[to1]);

    to2    = MoveTo(move);
    piece2 = pieceType(thread->board.squares[MoveFrom(move)]);

    assert(0 <= piece1 && piece1 < PIECE_NB);
    assert(0 <= to1 && to1 < SQUARE_NB);
    assert(0 <= piece2 && piece2 < PIECE_NB);
    assert(0 <= to2 && to2 < SQUARE_NB);

    delta = MAX(-400, MIN(400, delta));

    entry = thread->cmhistory[piece1][to1][piece2][to2];
    entry += 32 * delta - entry * abs(delta) / 512;
    thread->cmhistory[piece1][to1][piece2][to2] = entry;
}

uint16_t getCounterMove(Thread *thread, int height) {

    int colour, to, piece;
    const uint16_t previous = thread->moveStack[height-1];

    // Check for root position or null moves
    if (previous == NULL_MOVE || previous == NONE_MOVE)
        return NONE_MOVE;

    colour = !thread->board.turn;
    to     = MoveTo(previous);
    piece  = pieceType(thread->board.squares[to]);

    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= piece && piece < PIECE_NB);
    assert(0 <= to && to < SQUARE_NB);

    return thread->cmtable[colour][piece][to];
}

void updateCounterMove(Thread *thread, int height, uint16_t move) {

    int colour, to, piece;
    const uint16_t previous = thread->moveStack[height-1];

    // Check for root position or null moves
    if (previous == NULL_MOVE || previous == NONE_MOVE)
        return;

    colour = !thread->board.turn;
    to     = MoveTo(previous);
    piece  = pieceType(thread->board.squares[to]);

    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= piece && piece < PIECE_NB);
    assert(0 <= to && to < SQUARE_NB);

    thread->cmtable[colour][piece][to] = move;
}
