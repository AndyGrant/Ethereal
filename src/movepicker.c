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

#include "board.h"
#include "history.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "thread.h"
#include "types.h"

static uint16_t pop_move(int *size, uint16_t *moves, int *values, int index) {
    uint16_t popped = moves[index];
    moves[index] = moves[--*size];
    values[index] = values[*size];
    return popped;
}

static int best_index(MovePicker *mp, int start, int end) {

    int best = start;

    for (int i = start + 1; i < end; i++)
        if (mp->values[i] > mp->values[best])
            best = i;

    return best;
}


void init_picker(MovePicker *mp, Thread *thread, uint16_t tt_move) {

    // Start with the tt-move
    mp->stage   = STAGE_TABLE;
    mp->tt_move = tt_move;

    // Lookup our refutations (killers and counter moves)
    get_refutation_moves(thread, &mp->killer1, &mp->killer2, &mp->counter);

    // General housekeeping
    mp->threshold = 0;
    mp->type      = NORMAL_PICKER;

    // Skip over the TT-move if it is illegal
    mp->stage += !moveIsPseudoLegal(&thread->board, tt_move);
}

void init_noisy_picker(MovePicker *mp, Thread *thread, uint16_t tt_move, int threshold) {

    // Start with the tt-move potentially
    mp->stage   = STAGE_TABLE;
    mp->tt_move = tt_move;

    // Skip all of the refutation moves
    mp->killer1 = mp->killer2 = mp->counter = NONE_MOVE;

    // General housekeeping
    mp->threshold = threshold;
    mp->type      = NOISY_PICKER;

    // Skip over the TT-move unless its a threshold-winning capture
    mp->stage += !moveIsTactical(&thread->board, tt_move)
              || !moveIsPseudoLegal(&thread->board, tt_move)
              || !staticExchangeEvaluation(&thread->board, tt_move, threshold);
}

uint16_t select_next(MovePicker *mp, Thread *thread, int skip_quiets) {

    int best;
    uint16_t best_move;
    Board *board = &thread->board;

    switch (mp->stage) {

        case STAGE_TABLE:

            // Play table move ( Already shown to be legal )
            mp->stage = STAGE_GENERATE_NOISY;
            return mp->tt_move;

        case STAGE_GENERATE_NOISY:

            // Generate and evaluate noisy moves. mp->split sets a break point
            // to seperate the noisy from the quiet moves, so that we can skip
            // some of the noisy moves during STAGE_GOOD_NOISY and return later
            mp->noisy_size = mp->split = genAllNoisyMoves(board, mp->moves);
            get_capture_histories(thread, mp->moves, mp->values, 0, mp->noisy_size);
            mp->stage = STAGE_GOOD_NOISY;

            /* fallthrough */

        case STAGE_GOOD_NOISY:

            // Check to see if there are still more noisy moves
            while (mp->noisy_size) {

                // Grab the next best move index
                best = best_index(mp, 0, mp->noisy_size);

                // Values below zero are flagged as failing an SEE (bad noisy)
                if (mp->values[best] < 0)
                    break;

                // Skip moves which fail to beat our SEE margin. We flag those moves
                // as failed with the value (-1), and then repeat the selection process
                if (!staticExchangeEvaluation(board, mp->moves[best], mp->threshold)) {
                    mp->values[best] = -1;
                    continue;
                }

                // Reduce effective move list size
                best_move = pop_move(&mp->noisy_size, mp->moves, mp->values, best);

                // Don't play the table move twice
                if (best_move == mp->tt_move)
                    continue;

                // Don't play the refutation moves twice
                if (best_move == mp->killer1) mp->killer1 = NONE_MOVE;
                if (best_move == mp->killer2) mp->killer2 = NONE_MOVE;
                if (best_move == mp->counter) mp->counter = NONE_MOVE;

                return best_move;
            }

            // Jump to bad noisy moves when skipping quiets
            if (skip_quiets) {
                mp->stage = STAGE_BAD_NOISY;
                return select_next(mp, thread, skip_quiets);
            }

            mp->stage = STAGE_KILLER_1;

            /* fallthrough */

        case STAGE_KILLER_1:

            // Play killer move if not yet played, and pseudo legal
            mp->stage = STAGE_KILLER_2;
            if (   !skip_quiets
                &&  mp->killer1 != mp->tt_move
                &&  moveIsPseudoLegal(board, mp->killer1))
                return mp->killer1;

            /* fallthrough */

        case STAGE_KILLER_2:

            // Play killer move if not yet played, and pseudo legal
            mp->stage = STAGE_COUNTER_MOVE;
            if (   !skip_quiets
                &&  mp->killer2 != mp->tt_move
                &&  moveIsPseudoLegal(board, mp->killer2))
                return mp->killer2;

            /* fallthrough */

        case STAGE_COUNTER_MOVE:

            // Play counter move if not yet played, and pseudo legal
            mp->stage = STAGE_GENERATE_QUIET;
            if (   !skip_quiets
                &&  mp->counter != mp->tt_move
                &&  mp->counter != mp->killer1
                &&  mp->counter != mp->killer2
                &&  moveIsPseudoLegal(board, mp->counter))
                return mp->counter;

            /* fallthrough */

        case STAGE_GENERATE_QUIET:

            // Generate and evaluate all quiet moves when not skipping them
            if (!skip_quiets) {
                mp->quiet_size = genAllQuietMoves(board, mp->moves + mp->split);
                get_quiet_histories(thread, mp->moves, mp->values, mp->split, mp->quiet_size);
            }

            mp->stage = STAGE_QUIET;

            /* fallthrough */

        case STAGE_QUIET:

            // Check to see if there are still more quiet moves
            while (!skip_quiets && mp->quiet_size) {

                // Select next best quiet and reduce the effective move list size
                best = best_index(mp, mp->split, mp->split + mp->quiet_size) - mp->split;
                best_move = pop_move(&mp->quiet_size, mp->moves + mp->split, mp->values + mp->split, best);

                // Don't play a move more than once
                if (   best_move == mp->tt_move || best_move == mp->killer1
                    || best_move == mp->killer2 || best_move == mp->counter)
                    continue;

                return best_move;
            }

            // Out of quiet moves, only bad quiets remain
            mp->stage = STAGE_BAD_NOISY;

            /* fallthrough */

        case STAGE_BAD_NOISY:

            // Check to see if there are still more noisy moves
            while (mp->noisy_size && mp->type != NOISY_PICKER) {

                // Reduce effective move list size
                best_move = pop_move(&mp->noisy_size, mp->moves, mp->values, 0);

                // Don't play a move more than once
                if (   best_move == mp->tt_move || best_move == mp->killer1
                    || best_move == mp->killer2 || best_move == mp->counter)
                    continue;

                return best_move;
            }

            mp->stage = STAGE_DONE;

            /* fallthrough */

        case STAGE_DONE:
            return NONE_MOVE;

        default:
            return NONE_MOVE;
    }
}
