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
#include "castle.h"
#include "evaluate.h"
#include "history.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "psqt.h"
#include "types.h"
#include "thread.h"

void initializeMovePicker(MovePicker* mp, Thread* thread, uint16_t ttMove, int height, int skipQuiets){
    mp->skipQuiets = skipQuiets;
    mp->stage      = STAGE_TABLE;
    mp->noisySize  = 0;
    mp->quietSize  = 0;
    mp->tableMove  = ttMove;
    mp->killer1    = thread->killers[height][0] != ttMove ? thread->killers[height][0] : NONE_MOVE;
    mp->killer2    = thread->killers[height][1] != ttMove ? thread->killers[height][1] : NONE_MOVE;
    mp->history    = &thread->history;
}

uint16_t selectNextMove(MovePicker* mp, Board* board){

    int i, best;
    uint16_t bestMove;

    switch (mp->stage){

        case STAGE_TABLE:

            // Play the table move if it is from this
            // position, also advance to the next stage
            mp->stage = STAGE_GENERATE_NOISY;
            if (moveIsPsuedoLegal(board, mp->tableMove))
                return mp->tableMove;

            /* fallthrough */

        case STAGE_GENERATE_NOISY:

            // Generate all noisy moves and evaluate them. Set up the
            // split in the array to store quiet and noisy moves. Also,
            // this stage is only a helper. Advance to the next one.
            genAllNoisyMoves(board, mp->moves, &mp->noisySize);
            evaluateNoisyMoves(mp, board);
            mp->split = mp->noisySize;
            mp->stage = STAGE_NOISY ;

            /* fallthrough */

        case STAGE_NOISY:

            // Check to see if there are still more noisy moves
            if (mp->noisySize != 0){

                // Find highest scoring move
                for (best = 0, i = 1; i < mp->noisySize; i++)
                    if (mp->values[i] > mp->values[best])
                        best = i;

                // Save the best move before overwriting it
                bestMove = mp->moves[best];

                // Reduce effective move list size
                mp->noisySize -= 1;
                mp->moves[best] = mp->moves[mp->noisySize];
                mp->values[best] = mp->values[mp->noisySize];

                // Don't play the table move twice
                if (bestMove == mp->tableMove)
                    return selectNextMove(mp, board);

                // Don't play the killer moves twice
                if (bestMove == mp->killer1) mp->killer1 = NONE_MOVE;
                if (bestMove == mp->killer2) mp->killer2 = NONE_MOVE;

                return bestMove;
            }

            // If we are using this move picker for the quiescence
            // search, we have exhausted all moves already. Otherwise,
            // we should move onto the quiet moves (+ killers)
            if (mp->skipQuiets)
                return mp->stage = STAGE_DONE, NONE_MOVE;
            else
                mp->stage = STAGE_KILLER_1;

            /* fallthrough */

        case STAGE_KILLER_1:

            // Play the killer move if it is from this position.
            // position, and also advance to the next stage
            mp->stage = STAGE_KILLER_2;
            if (moveIsPsuedoLegal(board, mp->killer1))
                return mp->killer1;

            /* fallthrough */

        case STAGE_KILLER_2:

            // Play the killer move if it is from this position.
            // position, and also advance to the next stage
            mp->stage = STAGE_GENERATE_QUIET;
            if (moveIsPsuedoLegal(board, mp->killer2))
                return mp->killer2;

            /* fallthrough */

        case STAGE_GENERATE_QUIET:

            // Generate all quiet moves and evaluate them
            // and also advance to the final fruitful stage
            genAllQuietMoves(board, mp->moves + mp->split, &mp->quietSize);
            evaluateQuietMoves(mp, board);
            mp->stage = STAGE_QUIET;

            /* fallthrough */

        case STAGE_QUIET:

            // Check to see if there are still more quiet moves
            if (mp->quietSize != 0){

                // Find highest scoring move
                for (i = 1 + mp->split, best = mp->split; i < mp->split + mp->quietSize; i++)
                    if (mp->values[i] > mp->values[best])
                        best = i;

                // Save the best move before overwriting it
                bestMove = mp->moves[best];

                // Reduce effective move list size
                mp->quietSize--;
                mp->moves[best] = mp->moves[mp->split + mp->quietSize];
                mp->values[best] = mp->values[mp->split + mp->quietSize];

                // Don't play a move more than once
                if (   bestMove == mp->tableMove
                    || bestMove == mp->killer1
                    || bestMove == mp->killer2)
                    return selectNextMove(mp, board);

                return bestMove;
            }

            // If no quiet moves left, advance stages
            mp->stage = STAGE_DONE;

            /* fallthrough */

        case STAGE_DONE:
            return NONE_MOVE;

        default:
            assert(0);
            return NONE_MOVE;
    }
}

void evaluateNoisyMoves(MovePicker* mp, Board* board){

    uint16_t move;
    int i, value;
    int fromType, toType;

    for (i = 0; i < mp->noisySize; i++){

        move     = mp->moves[i];
        fromType = pieceType(board->squares[ MoveFrom(move)]);
        toType   = pieceType(board->squares[MoveTo(move)]);

        // Use the standard MVV-LVA
        value = PieceValues[toType][EG] - fromType;

        // A bonus is in order for queen promotions
        if ((move & QUEEN_PROMO_MOVE) == QUEEN_PROMO_MOVE)
            value += PieceValues[QUEEN][EG];

        // Enpass is a special case of MVV-LVA
        else if (MoveType(move) == ENPASS_MOVE)
            value = PieceValues[PAWN][EG] - PAWN;

        mp->values[i] = value;
    }
}

void evaluateQuietMoves(MovePicker* mp, Board* board){

    int i;

    // Use the History score from the Butterfly Bitboards for sorting
    for (i = mp->split; i < mp->split + mp->quietSize; i++)
        mp->values[i] = getHistoryScore(*mp->history, mp->moves[i], board->turn);
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
        return    type == NORMAL_MOVE
            && testBit(knightAttacks(from) & ~friendly, to);

    if (ftype == BISHOP)
        return    type == NORMAL_MOVE
            && testBit(bishopAttacks(from, occupied) & ~friendly, to);

    if (ftype == ROOK)
        return    type == NORMAL_MOVE
            && testBit(rookAttacks(from, occupied) & ~friendly, to);

    if (ftype == QUEEN)
        return    type == NORMAL_MOVE
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

            if (  ((occupied & WHITE_CASTLE_KING_SIDE_MAP) == 0ull)
                && (board->castleRights & WHITE_KING_RIGHTS)
                &&  MoveMake(4, 6, CASTLE_MOVE) == move
                && !squareIsAttacked(board, WHITE, 5))
                return 1;

            if (  ((occupied & WHITE_CASTLE_QUEEN_SIDE_MAP) == 0ull)
                && (board->castleRights & WHITE_QUEEN_RIGHTS)
                &&  MoveMake(4, 2, CASTLE_MOVE) == move
                && !squareIsAttacked(board, WHITE, 3))
                return 1;
        }

        if (colour == BLACK){

            if (  ((occupied & BLACK_CASTLE_KING_SIDE_MAP) == 0ull)
                && (board->castleRights & BLACK_KING_RIGHTS)
                &&  MoveMake(60, 62, CASTLE_MOVE) == move
                && !squareIsAttacked(board, BLACK, 61))
                return 1;

            if (  ((occupied & BLACK_CASTLE_QUEEN_SIDE_MAP) == 0ull)
                && (board->castleRights & BLACK_QUEEN_RIGHTS)
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
