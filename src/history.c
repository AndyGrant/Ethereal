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

static void updateHistoryWithDecay(int16_t *current, int delta) {
    *current += HistoryMultiplier * delta - *current * abs(delta) / HistoryDivisor;
}

void updateHistoryHeuristics(Thread *thread, uint16_t *moves, int length, int depth) {

    int bonus, colour = thread->board.turn;
    uint16_t bestMove = moves[length-1];

    // Extract information from last move
    uint16_t counter = thread->moveStack[thread->height-1];
    int cmPiece = thread->pieceStack[thread->height-1];
    int cmTo = MoveTo(counter);

    // Extract information from two moves ago
    uint16_t follow = thread->moveStack[thread->height-2];
    int fmPiece = thread->pieceStack[thread->height-2];
    int fmTo = MoveTo(follow);

    // Update Killer Moves (Avoid duplicates)
    if (thread->killers[thread->height][0] != bestMove) {
        thread->killers[thread->height][1] = thread->killers[thread->height][0];
        thread->killers[thread->height][0] = bestMove;
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
        updateHistoryWithDecay(&thread->history[colour][from][to], delta);

        // Update Counter Move History
        if (counter != NONE_MOVE && counter != NULL_MOVE)
            updateHistoryWithDecay(&thread->continuation[0][cmPiece][cmTo][piece][to], delta);

        // Update Followup Move History
        if (follow != NONE_MOVE && follow != NULL_MOVE)
            updateHistoryWithDecay(&thread->continuation[1][fmPiece][fmTo][piece][to], delta);
    }
}

void updateKillerMoves(Thread *thread, uint16_t move) {

    // Avoid saving the same Killer Move twice
    if (thread->killers[thread->height][0] == move) return;

    thread->killers[thread->height][1] = thread->killers[thread->height][0];
    thread->killers[thread->height][0] = move;
}


void updateCaptureHistories(Thread *thread, uint16_t best, uint16_t *moves, int length, int depth) {

    const int bonus = MIN(depth * depth, HistoryMax);

    for (int i = 0; i < length; i++) {

        const int to = MoveTo(moves[i]);
        const int from = MoveFrom(moves[i]);
        const int delta = moves[i] == best ? bonus : -bonus;

        int piece = pieceType(thread->board.squares[from]);
        int captured = pieceType(thread->board.squares[to]);

        if (MoveType(moves[i]) == ENPASS_MOVE   ) captured = PAWN;
        if (MoveType(moves[i]) == PROMOTION_MOVE) captured = PAWN;

        assert(PAWN <= piece && piece <= KING);
        assert(PAWN <= captured && captured <= QUEEN);

        updateHistoryWithDecay(&thread->chistory[piece][to][captured], delta);
    }
}

void getCaptureHistories(Thread *thread, uint16_t *moves, int *scores, int start, int length) {

    static const int MVVAugment[] = {0, 2400, 2400, 4800, 9600};

    for (int i = start; i < start + length; i++) {

        const int to = MoveTo(moves[i]);
        const int from = MoveFrom(moves[i]);

        int piece = pieceType(thread->board.squares[from]);
        int captured = pieceType(thread->board.squares[to]);

        if (MoveType(moves[i]) == ENPASS_MOVE   ) captured = PAWN;
        if (MoveType(moves[i]) == PROMOTION_MOVE) captured = PAWN;

        assert(PAWN <= piece && piece <= KING);
        assert(PAWN <= captured && captured <= QUEEN);

        scores[i] = 64000 + thread->chistory[piece][to][captured];
        if (MovePromoPiece(moves[i]) == QUEEN) scores[i] += 64000;
        scores[i] += MVVAugment[captured];

        assert(scores[i] >= 0);
    }
}

int getCaptureHistory(Thread *thread, uint16_t move) {

    const int to   = MoveTo(move);
    const int from = MoveFrom(move);

    int piece = pieceType(thread->board.squares[from]);
    int captured = pieceType(thread->board.squares[to]);

    if (MoveType(move) == ENPASS_MOVE   ) captured = PAWN;
    if (MoveType(move) == PROMOTION_MOVE) captured = PAWN;

    assert(PAWN <= piece && piece <= KING);
    assert(PAWN <= captured && captured <= QUEEN);

    return thread->chistory[piece][to][captured]
         + 64000 * (MovePromoPiece(move) == QUEEN);
}


int getHistory(Thread *thread, uint16_t move, int *cmhist, int *fmhist) {

    // Extract information from this move
    int to = MoveTo(move);
    int from = MoveFrom(move);
    int piece = pieceType(thread->board.squares[from]);

    // Extract information from last move
    uint16_t counter = thread->moveStack[thread->height-1];
    int cmPiece = thread->pieceStack[thread->height-1];
    int cmTo = MoveTo(counter);

    // Extract information from two moves ago
    uint16_t follow = thread->moveStack[thread->height-2];
    int fmPiece = thread->pieceStack[thread->height-2];
    int fmTo = MoveTo(follow);

    // Set Counter Move History if it exists
    if (counter == NONE_MOVE || counter == NULL_MOVE) *cmhist = 0;
    else *cmhist = thread->continuation[0][cmPiece][cmTo][piece][to];

    // Set Followup Move History if it exists
    if (follow == NONE_MOVE || follow == NULL_MOVE) *fmhist = 0;
    else *fmhist = thread->continuation[1][fmPiece][fmTo][piece][to];

    // Return CMHist + FMHist + ButterflyHist
    return *cmhist + *fmhist + thread->history[thread->board.turn][from][to];
}

void getHistoryScores(Thread *thread, uint16_t *moves, int *scores, int start, int length) {

    // Extract information from last move
    uint16_t counter = thread->moveStack[thread->height-1];
    int cmPiece = thread->pieceStack[thread->height-1];
    int cmTo = MoveTo(counter);

    // Extract information from two moves ago
    uint16_t follow = thread->moveStack[thread->height-2];
    int fmPiece = thread->pieceStack[thread->height-2];
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

void getRefutationMoves(Thread *thread, uint16_t *killer1, uint16_t *killer2, uint16_t *counter) {

    // Extract information from last move
    uint16_t previous = thread->moveStack[thread->height-1];
    int cmPiece = thread->pieceStack[thread->height-1];
    int cmTo = MoveTo(previous);

    // Set Killer Moves by height
    *killer1 = thread->killers[thread->height][0];
    *killer2 = thread->killers[thread->height][1];

    // Set Counter Move if one exists
    if (previous == NONE_MOVE || previous == NULL_MOVE) *counter = NONE_MOVE;
    else *counter = thread->cmtable[!thread->board.turn][cmPiece][cmTo];
}
