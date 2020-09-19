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

#include <assert.h>

#include "board.h"
#include "history.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "types.h"
#include "thread.h"

static uint16_t popMove(int *size, uint16_t *moves, int *values, int index) {
    uint16_t popped = moves[index];
    moves[index] = moves[--*size];
    values[index] = values[*size];
    return popped;
}

static int getBestMoveIndex(MovePicker *mp, int start, int end) {

    int best = start;

    for (int i = start + 1; i < end; i++)
        if (mp->values[i] > mp->values[best])
            best = i;

    return best;
}


void initMovePicker(MovePicker *mp, Thread *thread, uint16_t ttMove) {

    // Start with the table move
    mp->stage = STAGE_TABLE;
    mp->tableMove = ttMove;

    // Lookup our refutations (killers and counter moves)
    getRefutationMoves(thread, &mp->killer1, &mp->killer2, &mp->counter);

    // General housekeeping
    mp->threshold = 0;
    mp->thread = thread;
    mp->type = NORMAL_PICKER;
}

void initSingularMovePicker(MovePicker *mp, Thread *thread, uint16_t ttMove) {

    // Simply skip over the TT move
    initMovePicker(mp, thread, ttMove);
    mp->stage = STAGE_GENERATE_NOISY;

}

void initNoisyMovePicker(MovePicker *mp, Thread *thread, int threshold) {

    // Start with just the noisy moves
    mp->stage = STAGE_GENERATE_NOISY;

    // Skip all of the special (refutation and table) moves
    mp->tableMove = mp->killer1 = mp->killer2 = mp->counter = NONE_MOVE;

    // General housekeeping
    mp->threshold = threshold;
    mp->thread = thread;
    mp->type = NOISY_PICKER;
}

uint16_t selectNextMove(MovePicker *mp, Board *board, int skipQuiets) {

    int best; uint16_t bestMove;

    switch (mp->stage) {

        case STAGE_TABLE:

            // Play table move if it is pseudo legal
            mp->stage = STAGE_GENERATE_NOISY;
            if (moveIsPseudoLegal(board, mp->tableMove))
                return mp->tableMove;

            /* fallthrough */

        case STAGE_GENERATE_NOISY:

            // Generate and evaluate noisy moves. mp->split sets a break point
            // to seperate the noisy from the quiet moves, so that we can skip
            // some of the noisy moves during STAGE_GOOD_NOISY and return later
            mp->noisySize = mp->split = genAllNoisyMoves(board, mp->moves);
            getCaptureHistories(mp->thread, mp->moves, mp->values, 0, mp->noisySize);
            mp->stage = STAGE_GOOD_NOISY;

            /* fallthrough */

        case STAGE_GOOD_NOISY:

            // Check to see if there are still more noisy moves
            if (mp->noisySize) {

                // Grab the next best move index
                best = getBestMoveIndex(mp, 0, mp->noisySize);

                // Values below zero are flagged as failing an SEE (bad noisy)
                if (mp->values[best] >= 0) {

                    // Skip moves which fail to beat our SEE margin. We flag those moves
                    // as failed with the value (-1), and then repeat the selection process
                    if (!staticExchangeEvaluation(board, mp->moves[best], mp->threshold)) {
                        mp->values[best] = -1;
                        return selectNextMove(mp, board, skipQuiets);
                    }

                    // Reduce effective move list size
                    bestMove = popMove(&mp->noisySize, mp->moves, mp->values, best);

                    // Don't play the table move twice
                    if (bestMove == mp->tableMove)
                        return selectNextMove(mp, board, skipQuiets);

                    // Don't play the refutation moves twice
                    if (bestMove == mp->killer1) mp->killer1 = NONE_MOVE;
                    if (bestMove == mp->killer2) mp->killer2 = NONE_MOVE;
                    if (bestMove == mp->counter) mp->counter = NONE_MOVE;

                    return bestMove;
                }
            }

            // Jump to bad noisy moves when skipping quiets
            if (skipQuiets) {
                mp->stage = STAGE_BAD_NOISY;
                return selectNextMove(mp, board, skipQuiets);
            }

            mp->stage = STAGE_KILLER_1;

            /* fallthrough */

        case STAGE_KILLER_1:

            // Play killer move if not yet played, and pseudo legal
            mp->stage = STAGE_KILLER_2;
            if (   !skipQuiets
                &&  mp->killer1 != mp->tableMove
                &&  moveIsPseudoLegal(board, mp->killer1))
                return mp->killer1;

            /* fallthrough */

        case STAGE_KILLER_2:

            // Play killer move if not yet played, and pseudo legal
            mp->stage = STAGE_COUNTER_MOVE;
            if (   !skipQuiets
                &&  mp->killer2 != mp->tableMove
                &&  moveIsPseudoLegal(board, mp->killer2))
                return mp->killer2;

            /* fallthrough */

        case STAGE_COUNTER_MOVE:

            // Play counter move if not yet played, and pseudo legal
            mp->stage = STAGE_GENERATE_QUIET;
            if (   !skipQuiets
                &&  mp->counter != mp->tableMove
                &&  mp->counter != mp->killer1
                &&  mp->counter != mp->killer2
                &&  moveIsPseudoLegal(board, mp->counter))
                return mp->counter;

            /* fallthrough */

        case STAGE_GENERATE_QUIET:

            // Generate and evaluate all quiet moves when not skipping them
            if (!skipQuiets) {
                mp->quietSize = genAllQuietMoves(board, mp->moves + mp->split);
                getHistoryScores(mp->thread, mp->moves, mp->values, mp->split, mp->quietSize);
            }

            mp->stage = STAGE_QUIET;

            /* fallthrough */

        case STAGE_QUIET:

            // Check to see if there are still more quiet moves
            if (!skipQuiets && mp->quietSize) {

                // Select next best quiet and reduce the effective move list size
                best = getBestMoveIndex(mp, mp->split, mp->split + mp->quietSize) - mp->split;
                bestMove = popMove(&mp->quietSize, mp->moves + mp->split, mp->values + mp->split, best);

                // Don't play a move more than once
                if (   bestMove == mp->tableMove
                    || bestMove == mp->killer1
                    || bestMove == mp->killer2
                    || bestMove == mp->counter)
                    return selectNextMove(mp, board, skipQuiets);

                return bestMove;
            }

            // Out of quiet moves, only bad quiets remain
            mp->stage = STAGE_BAD_NOISY;

            /* fallthrough */

        case STAGE_BAD_NOISY:

            // Check to see if there are still more noisy moves
            if (mp->noisySize && mp->type != NOISY_PICKER) {

                // Reduce effective move list size
                bestMove = popMove(&mp->noisySize, mp->moves, mp->values, 0);

                // Don't play a move more than once
                if (   bestMove == mp->tableMove
                    || bestMove == mp->killer1
                    || bestMove == mp->killer2
                    || bestMove == mp->counter)
                    return selectNextMove(mp, board, skipQuiets);

                return bestMove;
            }

            mp->stage = STAGE_DONE;

            /* fallthrough */

        case STAGE_DONE:
            return NONE_MOVE;

        default:
            assert(0);
            return NONE_MOVE;
    }
}
