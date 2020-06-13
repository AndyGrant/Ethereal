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

void updateHistoryHeuristics(Thread *thread, uint16_t *moves, int length, int height, int depth) {

    int entry, bonus, colour = thread->board.turn;
    uint16_t bestMove = moves[length-1];

    // Extract information from last move
    uint16_t counter = thread->moveStack[height-1];
    int cmPiece = thread->pieceStack[height-1];
    int cmTo = MoveTo(counter);

    // Extract information from two moves ago
    uint16_t follow = thread->moveStack[height-2];
    int fmPiece = thread->pieceStack[height-2];
    int fmTo = MoveTo(follow);

    // Update Killer Moves (Avoid duplicates)
    if (thread->killers[height][0] != bestMove) {
        thread->killers[height][1] = thread->killers[height][0];
        thread->killers[height][0] = bestMove;
    }

    // Update Counter Moves (BestMove refutes the previous move)
    if (counter != NONE_MOVE && counter != NULL_MOVE)
        thread->cmtable[!colour][cmPiece][cmTo] = bestMove;

    // If the 1st quiet move failed-high below depth 4, we don't update history tables
    // Depth 0 gives no bonus in any case
    if (length == 1 && depth <= 3) return;

    // Cap update size to avoid saturation
    bonus = MIN(depth*depth, HistoryMax);

    for (int i = 0; i < length; i++) {

        // Apply a malus until the final move
        int delta = (moves[i] == bestMove) ? bonus : -bonus;

        // Extract information from this move
        int to = MoveTo(moves[i]);
        int from = MoveFrom(moves[i]);
        int piece = pieceType(thread->board.squares[from]);

        // Update Butterfly History
        entry = thread->history[colour][from][to];
        entry += HistoryMultiplier * delta - entry * abs(delta) / HistoryDivisor;
        thread->history[colour][from][to] = entry;

        // Update Counter Move History
        if (counter != NONE_MOVE && counter != NULL_MOVE) {
            entry = thread->continuation[0][cmPiece][cmTo][piece][to];
            entry += HistoryMultiplier * delta - entry * abs(delta) / HistoryDivisor;
            thread->continuation[0][cmPiece][cmTo][piece][to] = entry;
        }

        // Update Followup Move History
        if (follow != NONE_MOVE && follow != NULL_MOVE) {
            entry = thread->continuation[1][fmPiece][fmTo][piece][to];
            entry += HistoryMultiplier * delta - entry * abs(delta) / HistoryDivisor;
            thread->continuation[1][fmPiece][fmTo][piece][to] = entry;
        }
    }
}

void updateKillerMoves(Thread *thread, int height, uint16_t move) {

    // Avoid saving the same Killer Move twice
    if (thread->killers[height][0] == move) return;

    thread->killers[height][1] = thread->killers[height][0];
    thread->killers[height][0] = move;
}


void getHistory(Thread *thread, uint16_t move, int height, int *hist, int *cmhist, int *fmhist) {

    // Extract information from this move
    int to = MoveTo(move);
    int from = MoveFrom(move);
    int piece = pieceType(thread->board.squares[from]);

    // Extract information from last move
    uint16_t counter = thread->moveStack[height-1];
    int cmPiece = thread->pieceStack[height-1];
    int cmTo = MoveTo(counter);

    // Extract information from two moves ago
    uint16_t follow = thread->moveStack[height-2];
    int fmPiece = thread->pieceStack[height-2];
    int fmTo = MoveTo(follow);

    // Set basic Butterfly history
    *hist = thread->history[thread->board.turn][from][to];

    // Set Counter Move History if it exists
    if (counter == NONE_MOVE || counter == NULL_MOVE) *cmhist = 0;
    else *cmhist = thread->continuation[0][cmPiece][cmTo][piece][to];

    // Set Followup Move History if it exists
    if (follow == NONE_MOVE || follow == NULL_MOVE) *fmhist = 0;
    else *fmhist = thread->continuation[1][fmPiece][fmTo][piece][to];
}

void getHistoryScores(Thread *thread, uint16_t *moves, int *scores, int start, int length, int height) {

    // Extract information from last move
    uint16_t counter = thread->moveStack[height-1];
    int cmPiece = thread->pieceStack[height-1];
    int cmTo = MoveTo(counter);

    // Extract information from two moves ago
    uint16_t follow = thread->moveStack[height-2];
    int fmPiece = thread->pieceStack[height-2];
    int fmTo = MoveTo(follow);

    for (int i = start; i < start + length; i++) {

        // Extract information from this move
        int to = MoveTo(moves[i]);
        int from = MoveFrom(moves[i]);
        int piece = pieceType(thread->board.squares[from]);

        // Start with the basic Butterfly history
        scores[i] = thread->history[thread->board.turn][from][to];

        // Add Counter Move History if it exists
        if (counter != NONE_MOVE && counter != NULL_MOVE)
            scores[i] += thread->continuation[0][cmPiece][cmTo][piece][to];

        // Add Followup Move History if it exists
        if (follow != NONE_MOVE && follow != NULL_MOVE)
            scores[i] += thread->continuation[1][fmPiece][fmTo][piece][to];
    }
}

void getRefutationMoves(Thread *thread, int height, uint16_t *killer1, uint16_t *killer2, uint16_t *counter) {

    // Extract information from last move
    uint16_t previous = thread->moveStack[height-1];
    int cmPiece = thread->pieceStack[height-1];
    int cmTo = MoveTo(previous);

    // Set Killer Moves by height
    *killer1 = thread->killers[height][0];
    *killer2 = thread->killers[height][1];

    // Set Counter Move if one exists
    if (previous == NONE_MOVE || previous == NULL_MOVE) *counter = NONE_MOVE;
    else *counter = thread->cmtable[!thread->board.turn][cmPiece][cmTo];
}
