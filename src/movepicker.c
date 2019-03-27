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
#include <stdlib.h>

#include "attacks.h"
#include "board.h"
#include "bitboards.h"
#include "evaluate.h"
#include "history.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "psqt.h"
#include "types.h"
#include "thread.h"

void initMovePicker(MovePicker* mp, Thread* thread, uint16_t ttMove, int height){

    // Start with the table move
    mp->stage = STAGE_TABLE;
    mp->tableMove = ttMove;

    // Lookup our refutations (killers and counter moves)
    getRefutationMoves(thread, height, &mp->killer1, &mp->killer2, &mp->counter);

    // Threshold for good noisy
    mp->threshold = 0;

    mp->thread = thread;
    mp->height = height;
    mp->type = NORMAL_PICKER;
}

void initNoisyMovePicker(MovePicker* mp, Thread* thread, int threshold){

    // Start with just the noisy moves
    mp->stage = STAGE_GENERATE_NOISY;

    // Skip all special moves
    mp->tableMove = NONE_MOVE;
    mp->killer1   = NONE_MOVE;
    mp->killer2   = NONE_MOVE;
    mp->counter   = NONE_MOVE;

    // Threshold for good noisy
    mp->threshold = threshold;

    mp->thread = thread;
    mp->height = 0;
    mp->type = NOISY_PICKER;
}

uint16_t selectNextMove(MovePicker* mp, Board* board, int skipQuiets){

    int best;
    uint16_t bestMove;

    switch (mp->stage){

    case STAGE_TABLE:

        // Play table move if it is psuedo legal
        mp->stage = STAGE_GENERATE_NOISY;
        if (moveIsPsuedoLegal(board, mp->tableMove))
            return mp->tableMove;

        /* fallthrough */

    case STAGE_GENERATE_NOISY:

        // Generate and evaluate noisy moves. mp->split tracks the
        // break point between noisy and quiets, which allows us
        // to use a BAD_NOISY stage, where we skip noisy moves which
        // fail a simple SEE, and try them after all quiet moves

        mp->noisySize = 0;
        genAllNoisyMoves(board, mp->moves, &mp->noisySize);
        evaluateNoisyMoves(mp);
        mp->split = mp->noisySize;
        mp->stage = STAGE_GOOD_NOISY;

        /* fallthrough */

    case STAGE_GOOD_NOISY:

        // Check to see if there are still more noisy moves
        if (mp->noisySize != 0){

            // Select next best MVV-LVA from the noisy moves
            best = getBestMoveIndex(mp, 0, mp->noisySize);
            bestMove = mp->moves[best];

            // Values below zero are flagged as failing an SEE (bad noisy)
            if (mp->values[best] >= 0) {

                // Skip bad noisy moves during this stage
                if (!staticExchangeEvaluation(board, bestMove, mp->threshold)){

                    // Flag for failed use in STAGE_BAD_NOISY
                    mp->values[best] = -1;

                    // Try again to find a noisy move passing SEE
                    return selectNextMove(mp, board, skipQuiets);
                }

                // Reduce effective move list size
                mp->noisySize -= 1;
                mp->moves[best] = mp->moves[mp->noisySize];
                mp->values[best] = mp->values[mp->noisySize];

                // Don't play the table move twice
                if (bestMove == mp->tableMove)
                    return selectNextMove(mp, board, skipQuiets);

                // Don't play the special moves twice
                if (bestMove == mp->killer1) mp->killer1 = NONE_MOVE;
                if (bestMove == mp->killer2) mp->killer2 = NONE_MOVE;
                if (bestMove == mp->counter) mp->counter = NONE_MOVE;

                return bestMove;
            }
        }

        // Jump to bad noisy moves when skipping quiets
        if (skipQuiets){
            mp->stage = STAGE_BAD_NOISY;
            return selectNextMove(mp, board, skipQuiets);
        }

        mp->stage = STAGE_KILLER_1;

        /* fallthrough */

    case STAGE_KILLER_1:

        // Play killer move if not yet played, and psuedo legal
        mp->stage = STAGE_KILLER_2;
        if (   !skipQuiets
            &&  mp->killer1 != mp->tableMove
            &&  moveIsPsuedoLegal(board, mp->killer1))
            return mp->killer1;

        /* fallthrough */

    case STAGE_KILLER_2:

        // Play killer move if not yet played, and psuedo legal
        mp->stage = STAGE_COUNTER_MOVE;
        if (   !skipQuiets
            &&  mp->killer2 != mp->tableMove
            &&  moveIsPsuedoLegal(board, mp->killer2))
            return mp->killer2;

        /* fallthrough */

    case STAGE_COUNTER_MOVE:

        // Play counter move if not yet played, and psuedo legal
        mp->stage = STAGE_GENERATE_QUIET;
        if (   !skipQuiets
            &&  mp->counter != mp->tableMove
            &&  mp->counter != mp->killer1
            &&  mp->counter != mp->killer2
            &&  moveIsPsuedoLegal(board, mp->counter))
            return mp->counter;

        /* fallthrough */

    case STAGE_GENERATE_QUIET:

        // Generate and evaluate all quiet moves when not skipping quiet moves
        if (!skipQuiets){
            mp->quietSize = 0;
            genAllQuietMoves(board, mp->moves + mp->split, &mp->quietSize);
            getHistoryScores(mp->thread, mp->moves, mp->values, mp->split, mp->quietSize, mp->height);
        }

        mp->stage = STAGE_QUIET;

        /* fallthrough */

    case STAGE_QUIET:

        // Check to see if there are still more quiet moves
        if (!skipQuiets && mp->quietSize){

            // Select next best quiet by history scores
            best = getBestMoveIndex(mp, mp->split, mp->split + mp->quietSize);

            // Save the best move before overwriting it
            bestMove = mp->moves[best];

            // Reduce effective move list size
            mp->quietSize--;
            mp->moves[best] = mp->moves[mp->split + mp->quietSize];
            mp->values[best] = mp->values[mp->split + mp->quietSize];

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

        // Noisy picker skips all bad noisy moves
        if (mp->type == NOISY_PICKER) {
            mp->stage = STAGE_DONE;
            return NONE_MOVE;
        }

        // Check to see if there are still more noisy moves
        if (mp->noisySize != 0){

            // Save the best move before overwriting it
            bestMove = mp->moves[0];

            // Reduce effective move list size
            mp->noisySize -= 1;
            mp->moves[0] = mp->moves[mp->noisySize];
            mp->values[0] = mp->values[mp->noisySize];

            // Don't play a move more than once
            if (   bestMove == mp->tableMove
                || bestMove == mp->killer1
                || bestMove == mp->killer2
                || bestMove == mp->counter)
                return selectNextMove(mp, board, skipQuiets);

            return bestMove;
        }

        // Out of all captures and quiet moves, move picker complete
        mp->stage = STAGE_DONE;

        /* fallthrough */

    case STAGE_DONE:
        return NONE_MOVE;

    default:
        assert(0);
        return NONE_MOVE;
    }
}

int getBestMoveIndex(MovePicker *mp, int start, int end) {

    int best = start;

    for (int i = start + 1; i < end; i++)
        if (mp->values[i] > mp->values[best])
            best = i;

    return best;
}

void evaluateNoisyMoves(MovePicker* mp){

    int fromType, toType;

    // Use modified MVV-LVA to evaluate moves
    for (int i = 0; i < mp->noisySize; i++){

        fromType = pieceType(mp->thread->board.squares[MoveFrom(mp->moves[i])]);
        toType   = pieceType(mp->thread->board.squares[MoveTo(mp->moves[i])]);

        // Use the standard MVV-LVA
        mp->values[i] = PieceValues[toType][EG] - fromType;

        // A bonus is in order for queen promotions
        if ((mp->moves[i] & QUEEN_PROMO_MOVE) == QUEEN_PROMO_MOVE)
            mp->values[i] += PieceValues[QUEEN][EG];

        // Enpass is a special case of MVV-LVA
        else if (MoveType(mp->moves[i]) == ENPASS_MOVE)
            mp->values[i] = PieceValues[PAWN][EG] - PAWN;

        // Later we will flag moves which were passed over in the STAGE_GOOD_NOISY
        // phase due to failing an SEE(0), by setting the value to -1
        assert(mp->values[i] >= 0);
    }
}

int moveIsPsuedoLegal(Board* board, uint16_t move){

    int colour = board->turn;
    int to     = MoveTo(move);
    int from   = MoveFrom(move);
    int type   = MoveType(move);
    int ptype  = MovePromoType(move);
    int ftype  = pieceType(board->squares[from]);

    uint64_t friendly = board->colours[ colour];
    uint64_t enemy    = board->colours[!colour];
    uint64_t occupied = friendly | enemy;
    uint64_t left, right, forward;

    // Quick check against obvious illegal moves
    if (move == NULL_MOVE || move == NONE_MOVE)
        return 0;

    // Verify the moving piece is our own
    if (pieceColour(board->squares[from]) != colour)
        return 0;

    // Non promotions should be marked as PROMOTE_TO_KNIGHT
    if (ptype != PROMOTE_TO_KNIGHT && type != PROMOTION_MOVE)
        return 0;


    // Knight, Rook, Bishop, and Queen moves are legal so long as the
    // move type is NORMAL and the destination is an attacked square

    if (ftype == KNIGHT)
        return type == NORMAL_MOVE
            && testBit(knightAttacks(from) & ~friendly, to);

    if (ftype == BISHOP)
        return type == NORMAL_MOVE
            && testBit(bishopAttacks(from, occupied) & ~friendly, to);

    if (ftype == ROOK)
        return type == NORMAL_MOVE
            && testBit(rookAttacks(from, occupied) & ~friendly, to);

    if (ftype == QUEEN)
        return type == NORMAL_MOVE
            && testBit(queenAttacks(from, occupied) & ~friendly, to);

    if (ftype == PAWN){

        // Throw out castle moves with our pawn
        if (type == CASTLE_MOVE)
            return 0;

        // Look at the squares which our pawn threatens
        left  = pawnLeftAttacks(1ull << from, ~0ull, colour);
        right = pawnRightAttacks(1ull << from, ~0ull, colour);

        // Enpass moves are legal if our to square is the enpass
        // square and we could attack a piece on the enpass square
        if (type == ENPASS_MOVE)
            return   to == board->epSquare
                && ((left | right) & (1ull << board->epSquare));

        // Ensure that left and right are now captures, compute advances
        left = left & enemy;
        right = right & enemy;
        forward = pawnAdvance(1ull << from, occupied, colour);

        // Promotion moves are legal if we can move to one of the promotion
        // ranks, defined by PROMOTION_RANKS, independent of moving colour
        if (type == PROMOTION_MOVE)
            return !!(PROMOTION_RANKS & (left | right | forward) & (1ull << to));

        // Add the double advance to forward
        forward |= pawnAdvance(forward & (!colour ? RANK_3 : RANK_6), occupied, colour);

        // Normal moves are legal if we can move there
        return !!((left | right | forward) & (1ull << to) & ~PROMOTION_RANKS);
    }

    if (ftype == KING){

        // Normal moves are legal if to square is a valid target
        if (type == NORMAL_MOVE)
            return testBit(kingAttacks(from) & ~friendly, to);

        // Kings cannot castle or promote
        if (type == ENPASS_MOVE || type == PROMOTION_MOVE)
            return 0;

        // Kings cannot castle out of check
        if (board->kingAttackers)
            return 0;

        // Castling is hard to verify directly, so just generate
        // the possible castling options, and check equality

        if (colour == WHITE){

            if (  ((occupied & WHITE_OO_MAP) == 0ull)
                && (board->castleRights & WHITE_OO_RIGHTS)
                &&  MoveMake(4, 6, CASTLE_MOVE) == move
                && !squareIsAttacked(board, WHITE, 5))
                return 1;

            if (  ((occupied & WHITE_OOO_MAP) == 0ull)
                && (board->castleRights & WHITE_OOO_RIGHTS)
                &&  MoveMake(4, 2, CASTLE_MOVE) == move
                && !squareIsAttacked(board, WHITE, 3))
                return 1;
        }

        if (colour == BLACK){

            if (  ((occupied & BLACK_OO_MAP) == 0ull)
                && (board->castleRights & BLACK_OO_RIGHTS)
                &&  MoveMake(60, 62, CASTLE_MOVE) == move
                && !squareIsAttacked(board, BLACK, 61))
                return 1;

            if (  ((occupied & BLACK_OOO_MAP) == 0ull)
                && (board->castleRights & BLACK_OOO_RIGHTS)
                &&  MoveMake(60, 58, CASTLE_MOVE) == move
                && !squareIsAttacked(board, BLACK, 59))
                return 1;
        }

        // No such castle was found via generation
        return 0;
    }

    // The colour check should (assuming board->squares only contains pieces
    // and EMPTY flags...) should ensure that ftype is an actual piece
    assert(0); return 0;
}
