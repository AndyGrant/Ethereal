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

extern HistoryTable History;

void initalizeMovePicker(MovePicker * mp, int isQuiescencePick,
                          uint16_t tableMove, uint16_t killer1,
                                             uint16_t killer2){
                                 
    mp->isQuiescencePick = isQuiescencePick;
    mp->stage = STAGE_TABLE;
    mp->split = 0;
    mp->noisySize = 0;
    mp->badSize = 0;
    mp->quietSize = 0;
    mp->tableMove = tableMove;
    mp->killer1 = (killer1 != tableMove) ? killer1 : NONE_MOVE;
    mp->killer2 = (killer2 != tableMove) ? killer2 : NONE_MOVE;
}

uint16_t selectNextMove(MovePicker * mp, Board * board){
    
    int i, best;
    uint16_t bestMove;
    
    
    switch (mp->stage){
        
        case STAGE_TABLE:
        
            // Advance to the next stage no matter what
            mp->stage = STAGE_GENERATE_NOISY;
            
            // See if the table move is an available move
            if (moveIsPsuedoLegal(board, mp->tableMove))
                return mp->tableMove;            
        
        
        case STAGE_GENERATE_NOISY:
        
            // Generate all noisy moves and evaluate them
            genAllNoisyMoves(board, mp->moves, &mp->noisySize);
            evaluateNoisyMoves(mp, board);
            
            // Save the location of the split in the moves array.
            // We will use just one array for noisy and quiet moves.
            mp->split = mp->noisySize;
            
            // This stage is only a helper, advance to move selection
            mp->stage = STAGE_NOISY ;
            
        
        case STAGE_NOISY:
        
            // Check to see if there are still noisy moves left
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
            
            // If no noisy moves, or no good ones left, advance stages
            mp->stage = STAGE_KILLER_1;
            
            // If we are using this move picker for the quiescence
            // search, we have exhausted all moves already
            if (mp->isQuiescencePick){
                mp->stage = STAGE_DONE;
                return NONE_MOVE;
            }
            
            
        case STAGE_KILLER_1:
            
            // Advance to the next stage no matter what
            mp->stage = STAGE_KILLER_2;
            
            if (moveIsPsuedoLegal(board, mp->killer1))
                return mp->killer1;
            
            
        case STAGE_KILLER_2:
            
            // Advance to the next stage no matter what
            mp->stage = STAGE_GENERATE_QUIET;
            
            if (moveIsPsuedoLegal(board, mp->killer2))
                return mp->killer2;
        
        
        case STAGE_GENERATE_QUIET:
            
            // Generate all quiet moves and evaluate them
            genAllQuietMoves(board, mp->moves + mp->split, &mp->quietSize);
            evaluateQuietMoves(mp, board);
            
            // This stage is only a helper, advance to move selection
            mp->stage = STAGE_QUIET;
            
            
        case STAGE_QUIET:
        
            // Check to see if there are still quiet moves left
            if (mp->quietSize != 0){
        
                // Find highest scoring move
                best = mp->split;
                for (i = 1 + mp->split; i < mp->split + mp->quietSize; i++)
                    if (mp->values[i] > mp->values[best])
                        best = i;
                   
                // Save the best move before overwriting it
                bestMove = mp->moves[best];
                
                // Reduce effective move list size
                mp->quietSize--;
                mp->moves[best] = mp->moves[mp->split + mp->quietSize];
                mp->values[best] = mp->values[mp->split + mp->quietSize];
                
                // Don't play a move more than once
                if (bestMove == mp->tableMove
                    || bestMove == mp->killer1
                    || bestMove == mp->killer2)
                    return selectNextMove(mp, board);
                
                return bestMove;
            }
            
            // If no quiet moves left, advance stages
            mp->stage = STAGE_DONE;
            
            
        case STAGE_DONE:
        
            // return NONE_MOVE to indicate all moves picked
            return NONE_MOVE;
            
        default:
        
            // This statement should never be reached
            assert(0);
            return NONE_MOVE;
    }
}
   
void evaluateNoisyMoves(MovePicker * mp, Board * board){
    
    uint16_t move;
    int i, value, from, to;
    int fromType, toType;
    
    for (i = 0; i < mp->noisySize; i++){
        
        move = mp->moves[i];
        from = MoveFrom(move);
        to = MoveTo(move);
        fromType = PieceType(board->squares[from]);
        toType = PieceType(board->squares[to]);
        
        // Use the standard MVV-LVA
        value = PieceValues[toType] - fromType;
        
        // A bonus is in order for queen promotions
        if ((move & QUEEN_PROMO_MOVE) == QUEEN_PROMO_MOVE)
            value += QueenValue;
        
        // Enpass is a special case of MVV-LVA
        else if (MoveType(move) == ENPASS_MOVE)
            value = PawnValue - PAWN;
        
        mp->values[i] = value;
    }
}

void evaluateQuietMoves(MovePicker * mp, Board * board){
    
    uint16_t move;
    int i, value, from, to;
    
    for (i = mp->split; i < mp->split + mp->quietSize; i++){
        
        move = mp->moves[i];
        from = MoveFrom(move);
        to = MoveTo(move);
        
        // Use the history score and PSQT to evaluate the move
        value =  getHistoryScore(History, move, board->turn, 512);
        value += abs(PSQTopening[board->squares[from]][to]);
        value -= abs(PSQTopening[board->squares[from]][from]);
        mp->values[i] = value;
    }
}

int moveIsGoodCapture(Board * board, uint16_t move){
    
    int from, to, fromType, toType;
    
    if (MoveType(move) == PROMOTION_MOVE)
        return MovePromoType(move) == PROMOTE_TO_QUEEN;
    
    if (MoveType(move) == ENPASS_MOVE)
        return 1;
    
    from = MoveFrom(move);
    to = MoveTo(move);
    
    fromType = PieceType(board->squares[from]);
    toType = PieceType(board->squares[to]);
    
    return PieceValues[toType] >= PieceValues[fromType];    
}

int moveIsPsuedoLegal(Board * board, uint16_t move){
    
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
                if ((leftward|rightward) & (1ull << board->epSquare)) return 1;
                
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
            options = KnightAttacks(from, ~friendly);
            return (options & (1ull << to)) >> to;
        
        
        case BISHOP:
            
            // First ensure the move type is correct
            if (moveType != NORMAL_MOVE) return 0;
        
            // Generate Bishop attacks and compare to destination
            options = BishopAttacks(from, ~empty, ~friendly);
            return (options & (1ull << to)) >> to;
        
        
        case ROOK:
        
            // First ensure the move type is correct
            if (moveType != NORMAL_MOVE) return 0;
            
            // Generate Rook attacks and compare to destination
            options = RookAttacks(from, ~empty, ~friendly);
            return (options & (1ull << to)) >> to;
        
        
        case QUEEN:
        
            // First ensure the move type is correct
            if (moveType != NORMAL_MOVE) return 0;
            
            // Generate Queen attacks and compare to destination
            options = BishopAttacks(from, ~empty, ~friendly)
                    | RookAttacks(from, ~empty, ~friendly);
            return (options & (1ull << to)) >> to;
        
        
        case KING:
        
            // If normal move, generate King attacks and compare to destination
            if (moveType == NORMAL_MOVE){
                options = KingAttacks(from, ~friendly);
                return (options & (1ull << to)) >> to;
            }
            
            else if (moveType == CASTLE_MOVE){
                
                // Determine the squares which must be unoccupied,
                // the needed castle rights, and the crossover square
                if (board->turn == WHITE){
                    
                    map = (to > from)       ? WHITE_CASTLE_KING_SIDE_MAP
                                            : WHITE_CASTLE_QUEEN_SIDE_MAP;
                                            
                    rights = (to > from)    ? WHITE_KING_RIGHTS
                                            : WHITE_QUEEN_RIGHTS;
                                            
                    crossover = (to > from) ? 5 : 3;
                    
                    castleStart = 4;
                    
                    castleEnd = (to > from) ? 6 : 2;
                    
                } else {
                    
                    map = (to > from)       ? BLACK_CASTLE_KING_SIDE_MAP
                                            : BLACK_CASTLE_QUEEN_SIDE_MAP;
                                            
                    rights = (to > from)    ? BLACK_KING_RIGHTS
                                            : BLACK_QUEEN_RIGHTS;
                                            
                    crossover = (to > from) ? 61 : 59;

                    castleStart = 60;
                    
                    castleEnd = (to > from) ? 62 : 58;
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
            
            else {
                
                // Move has a King either promoting or enpassing
                return 0;
            }
        
        default:
        
            // We should never get to this default case
            assert(0);
            return 0;
    }
}