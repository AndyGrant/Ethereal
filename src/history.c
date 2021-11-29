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

    NodeState *const ns = &thread->states[thread->height];

    int bonus, colour = thread->board.turn;
    uint16_t bestMove = moves[length-1];

    // Update Killer Moves (Avoid duplicates)
    if (thread->killers[thread->height][0] != bestMove) {
        thread->killers[thread->height][1] = thread->killers[thread->height][0];
        thread->killers[thread->height][0] = bestMove;
    }

    // Update Counter Moves (BestMove refutes the previous move)
    if ((ns-1)->move != NONE_MOVE && (ns-1)->move != NULL_MOVE)
        thread->cmtable[!colour][(ns-1)->movedPiece][MoveTo((ns-1)->move)] = bestMove;

    // If the 1st quiet move failed-high below depth 4, we don't update history tables
    // Depth 0 gives no bonus in any case
    if (length == 1 && depth <= 3) return;

    // Cap update size to avoid saturation
    bonus = MIN(depth*depth, HistoryMax);

    for (int i = 0; i < length; i++) {

        // Apply a malus until the final move
        const int delta = (moves[i] == bestMove) ? bonus : -bonus;

        // Extract information from this move
        const int to    = MoveTo(moves[i]);
        const int from  = MoveFrom(moves[i]);
        const int piece = pieceType(thread->board.squares[from]);

        // Update Butterfly History
        updateHistoryWithDecay(&thread->history[colour][from][to], delta);

        // Update Counter Move History if it exists
        if ((ns-1)->continuations != NULL)
            updateHistoryWithDecay(&(*(ns-1)->continuations)[0][piece][to], delta);

        // Update Move History if it exists
        if ((ns-2)->continuations != NULL)
            updateHistoryWithDecay(&(*(ns-2)->continuations)[1][piece][to], delta);
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

    NodeState *const ns = &thread->states[thread->height];

    // Extract information from this move
    const int to    = MoveTo(move);
    const int from  = MoveFrom(move);
    const int piece = pieceType(thread->board.squares[from]);

    // Set Counter Move History if it exists
    *cmhist = (ns-1)->continuations == NULL ? 0
            : (*(ns-1)->continuations)[0][piece][to];

    // Set Followup Move History if it exists
    *fmhist = (ns-2)->continuations == NULL ? 0
            : (*(ns-2)->continuations)[1][piece][to];

    // Return CMHist + FMHist + ButterflyHist
    return *cmhist + *fmhist + thread->history[thread->board.turn][from][to];
}

void getHistoryScores(Thread *thread, uint16_t *moves, int *scores, int start, int length) {

    NodeState *const ns = &thread->states[thread->height];

    for (int i = start; i < start + length; i++) {

        // Extract information from this move
        const int to    = MoveTo(moves[i]);
        const int from  = MoveFrom(moves[i]);
        const int piece = pieceType(thread->board.squares[from]);

        // Start with the basic Butterfly history
        scores[i] = thread->history[thread->board.turn][from][to];

        // Add Counter Move History if it exists
        if ((ns-1)->continuations != NULL)
            scores[i] += (*(ns-1)->continuations)[0][piece][to];

        // Add Followup Move History if it exists
        if ((ns-2)->continuations != NULL)
            scores[i] += (*(ns-2)->continuations)[1][piece][to];
    }
}

void getRefutationMoves(Thread *thread, uint16_t *killer1, uint16_t *killer2, uint16_t *counter) {

    NodeState *const ns = &thread->states[thread->height];

    // Set Counter Move if one exists
    *counter = ((ns-1)->move == NONE_MOVE || (ns-1)->move == NULL_MOVE) ? NONE_MOVE
             :  thread->cmtable[!thread->board.turn][(ns-1)->movedPiece][MoveTo((ns-1)->move)];

    // Set Killer Moves by height
    *killer1 = thread->killers[thread->height][0];
    *killer2 = thread->killers[thread->height][1];
}
