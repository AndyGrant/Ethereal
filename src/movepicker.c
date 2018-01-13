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

#include "board.h"
#include "bitboards.h"
#include "castle.h"
#include "evaluate.h"
#include "history.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "piece.h"
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
        fromType = PieceType(board->squares[ MoveFrom(move)]);
        toType   = PieceType(board->squares[MoveTo(move)]);
        
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
    
    uint16_t move;
    int i, value;
    
    for (i = mp->split; i < mp->split + mp->quietSize; i++){
        
        move = mp->moves[i];
        
        static const int SortingTypes[KING+1] = {10, 8, 8, 4, 3, 1};
        
        static const int SortingTable[SQUARE_NB] = {
            0, 0, 0, 0, 0, 0, 0, 0,
            1, 2, 2, 2, 2, 2, 2, 1,
            1, 2, 4, 4, 4, 4, 2, 1,
            1, 2, 4, 6, 6, 4, 2, 1,
            1, 2, 4, 6, 6, 4, 2, 1,
            1, 2, 4, 4, 4, 4, 2, 1,
            1, 2, 2, 2, 2, 2, 2, 1,
            0, 0, 0, 0, 0, 0, 0, 0,
        };
        
        // Use the history score and PSQT to evaluate the move
        value =  getHistoryScore(*mp->history, move, board->turn, 256);
        value += SortingTypes[PieceType(board->squares[MoveFrom(move)])] * SortingTable[MoveTo(move)  ];
        value -= SortingTypes[PieceType(board->squares[MoveFrom(move)])] * SortingTable[MoveFrom(move)];
        
        mp->values[i] = value;
    }
}

int moveIsPsuedoLegal(Board* board, uint16_t move){
    
    int from, to, moveType, promoType, fromType; 
    int castleStart, castleEnd, rights, crossover;
    uint64_t friendly, enemy, empty, options, map;
    uint64_t forwardOne, forwardTwo, leftward, rightward;
    uint64_t promoForward, promoLeftward, promoRightward;
    
    if (move == NULL_MOVE || move == NONE_MOVE)
        return 0;
    
    from = MoveFrom(move);
    to = MoveTo(move);
    moveType = MoveType(move);
    promoType = MovePromoType(move);
    fromType = PieceType(board->squares[from]);
    friendly = board->colours[board->turn];
    enemy = board->colours[!board->turn];
    empty = ~(friendly | enemy);
    
    // Trying to move an empty square, or an enemy piece
    if (PieceColour(board->squares[from]) != board->turn)
        return 0;
    
    // Non promotion moves should be marked as Knight Promotions
    if (promoType != PROMOTE_TO_KNIGHT && moveType != PROMOTION_MOVE)
        return 0;
    
    switch (fromType){
        
        case PAWN:
        
            // Pawns cannot be involved in a Castle
            if (moveType == CASTLE_MOVE)
                return 0;
        
            // Compute bitboards for possible movement of a Pawn
            if (board->turn == WHITE){
                
                forwardOne     = ((1ull << from) << 8) & empty;
                forwardTwo     = ((forwardOne & RANK_3) << 8) & empty;
                leftward       = ((1ull << from) << 7) & ~FILE_H;
                rightward      = ((1ull << from) << 9) & ~FILE_A;
                promoForward   = forwardOne & RANK_8;
                promoLeftward  = leftward & RANK_8 & enemy;
                promoRightward = rightward & RANK_8 & enemy;
                forwardOne     = forwardOne & ~RANK_8;
                leftward       = leftward & ~RANK_8;
                rightward      = rightward & ~RANK_8;
                
            } else {
                
                forwardOne     = ((1ull << from) >> 8) & empty;
                forwardTwo     = ((forwardOne & RANK_6) >> 8) & empty;
                leftward       = ((1ull << from) >> 7) & ~FILE_A;
                rightward      = ((1ull << from) >> 9) & ~FILE_H;
                promoForward   = forwardOne & RANK_1;
                promoLeftward  = leftward & RANK_1 & enemy;
                promoRightward = rightward & RANK_1 & enemy;
                forwardOne     = forwardOne & ~RANK_1;
                leftward       = leftward & ~RANK_1;
                rightward      = rightward & ~RANK_1;
            }
            
            if (moveType == ENPASS_MOVE){
                
                // Make sure we can move to the enpass square
                if (!((leftward | rightward) & (1ull << board->epSquare))) return 0;
                
                // If the square matchs the to, then the move is valid
                return to == board->epSquare;
            }
            
            // Correct the movement bitboards
            leftward &= enemy; rightward &= enemy;
            
            // Determine the possible movements based on move type
            if (moveType == NORMAL_MOVE)
                options = forwardOne | forwardTwo | leftward | rightward;
            else
                options = promoForward | promoLeftward | promoRightward;
            
            // See if one of the possible moves includs to to square
            return (options & (1ull << to)) >> to;
                
        case KNIGHT:
            
            // First ensure the move type is correct
            if (moveType != NORMAL_MOVE) return 0;
            
            // Generate Knight attacks and compare to destination
            options = knightAttacks(from, ~friendly);
            return (options & (1ull << to)) >> to;
        
        
        case BISHOP:
            
            // First ensure the move type is correct
            if (moveType != NORMAL_MOVE) return 0;
        
            // Generate Bishop attacks and compare to destination
            options = bishopAttacks(from, ~empty, ~friendly);
            return (options & (1ull << to)) >> to;
        
        
        case ROOK:
        
            // First ensure the move type is correct
            if (moveType != NORMAL_MOVE) return 0;
            
            // Generate Rook attacks and compare to destination
            options = rookAttacks(from, ~empty, ~friendly);
            return (options & (1ull << to)) >> to;
        
        
        case QUEEN:
        
            // First ensure the move type is correct
            if (moveType != NORMAL_MOVE) return 0;
            
            // Generate Queen attacks and compare to destination
            options = bishopAttacks(from, ~empty, ~friendly)
                    | rookAttacks(from, ~empty, ~friendly);
            return (options & (1ull << to)) >> to;
        
        
        case KING:
        
            // If normal move, generate King attacks and compare to destination
            if (moveType == NORMAL_MOVE){
                options = kingAttacks(from, ~friendly);
                return (options & (1ull << to)) >> to;
            }
            
            else if (moveType == CASTLE_MOVE){
                
                // Determine the squares which must be unoccupied,
                // the needed castle rights, and the crossover square
                if (board->turn == WHITE){
                    map = (to > from)       ? WHITE_CASTLE_KING_SIDE_MAP : WHITE_CASTLE_QUEEN_SIDE_MAP;
                    rights = (to > from)    ? WHITE_KING_RIGHTS : WHITE_QUEEN_RIGHTS;
                    crossover = (to > from) ? 5 : 3;
                    castleEnd = (to > from) ? 6 : 2;
                    castleStart = 4;
                } 
                
                else {
                    map = (to > from)       ? BLACK_CASTLE_KING_SIDE_MAP : BLACK_CASTLE_QUEEN_SIDE_MAP;
                    rights = (to > from)    ? BLACK_KING_RIGHTS : BLACK_QUEEN_RIGHTS;
                    crossover = (to > from) ? 61 : 59;
                    castleEnd = (to > from) ? 62 : 58;
                    castleStart = 60;
                }
                
                // Inorder to be psuedo legal, the from square must
                // match the starting king square, the to square
                // must must the correct movement, the area between the king
                // and the rook must be empty, we must have the proper
                // castling rights, we must not current be in check,
                // and we must also not cross through a checked square.
                if (from != castleStart) return 0;
                if (to != castleEnd) return 0;
                if (~empty & map) return 0;
                if (!(board->castleRights & rights)) return 0;
                if (!isNotInCheck(board, board->turn)) return 0;
                return !squareIsAttacked(board, board->turn, crossover);
            }
            
            else
                return 0;
            
        default:
            assert(0);
            return 0;
    }
}
