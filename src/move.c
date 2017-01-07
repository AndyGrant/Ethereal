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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "bitboards.h"
#include "board.h"
#include "castle.h"
#include "types.h"
#include "masks.h"
#include "move.h"
#include "piece.h"
#include "psqt.h"
#include "zorbist.h"

/**
 * Apply a given move to a given board and update all of the
 * necessary information including, opening and endgame values
 * hash signature, piece counts, castling rights, enpassant
 * potentials, and the history of moves made.
 *
 * @param   board   Board pointer to current position
 * @param   move    Move to be applied to the board
 * @param   undo    Undo object to be filled with information
 *                  which is either hard or impossible to reverse
 *                  with only the board and the move
 */
void applyMove(Board * board, uint16_t move, Undo * undo){
    
    int to, from;
    int rTo, rFrom;
    int fromType, toType;
    int promotype, ep;
    
    int fromPiece, toPiece;
    int rFromPiece, enpassPiece;
    int promoPiece;
    
    uint64_t shiftFrom, shiftTo;
    uint64_t rShiftFrom, rShiftTo;
    uint64_t shiftEnpass;
    
    uint64_t enemyPawns;
    
    // Save information that is either hard to reverse,
    // or is not worth the time in order to do so
    undo->epSquare = board->epSquare;
    undo->turn = board->turn;
    undo->castleRights = board->castleRights;
    undo->opening = board->opening;
    undo->endgame = board->endgame;
    undo->phash = board->phash;
    undo->hash = board->hash;
    
    board->history[board->numMoves++] = board->hash;
    
    // Update the key to include the turn change
    board->hash ^= ZorbistKeys[TURN][0];
    
    if (MoveType(move) == NORMAL_MOVE){
        to = MoveTo(move);
        from = MoveFrom(move);
        
        shiftFrom = 1ull << from;
        shiftTo = 1ull << to;
        
        fromPiece = board->squares[from];
        toPiece = board->squares[to];
        
        fromType = PieceType(fromPiece);
        toType = PieceType(toPiece);
        
        // Update the colour bitboards
        board->colours[board->turn] ^= shiftFrom | shiftTo;
        board->colours[PieceColour(toPiece)] ^= shiftTo;
        
        // Update the piece bitboards
        board->pieces[toType] ^= shiftTo;
        board->pieces[fromType] ^= shiftFrom | shiftTo;
        
        // Save the captured piece and update the board array
        undo->capturePiece = toPiece;
        board->squares[to] = fromPiece;
        board->squares[from] = EMPTY;
        
        // Bitwise-and out the castle changes
        board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
        board->castleRights &= CastleMask[from];
        board->castleRights &= CastleMask[to];
        board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
        
        // Swap the turn
        board->turn = !board->turn;
        
        // Update the PSQT value for the opening
        board->opening += PSQTopening[fromPiece][to]
                        - PSQTopening[fromPiece][from]
                        - PSQTopening[toPiece][to];
        
        // Update the PSQT value for the endgame
        board->endgame += PSQTendgame[fromPiece][to]
                        - PSQTendgame[fromPiece][from]
                        - PSQTendgame[toPiece][to];
                        
        // Update the main zorbist hash
        board->hash ^= ZorbistKeys[fromPiece][from]
                    ^  ZorbistKeys[fromPiece][to]
                    ^  ZorbistKeys[toPiece][to];
                    
        // Update the pawn zorbist hash
        board->phash^= PawnKeys[fromPiece][from]
                    ^  PawnKeys[fromPiece][to]
                    ^  PawnKeys[toPiece][to];
                    
        // If there was a possible enpass move, we must
        // xor the main zorbist key for it before moving on
        if (board->epSquare != -1)
            board->hash ^= ZorbistKeys[ENPASS][File(board->epSquare)];
        
        // Zero out the enpass square for now
        board->epSquare = -1;
        
        // Check to see if this move creates an enpass square
        if (fromType == PAWN && abs(to-from) == 16){
            
            enemyPawns = board->colours[board->turn] & board->pieces[PAWN];
            enemyPawns &= IsolatedPawnMasks[from];;
            enemyPawns &= (board->turn == BLACK) ? RANK_4 : RANK_5;
            
            // Only set the enpass square if there is a     
            // pawn that could actually perform the enpass
            if (enemyPawns){
                
                // Update the main zorbist key to include the enpass file
                board->epSquare = from + ((to-from) >> 1);
                board->hash ^= ZorbistKeys[ENPASS][File(from)];
            } 
        }
        
        return;
    }
    
    if (MoveType(move) == CASTLE_MOVE){
        
        board->hasCastled[board->turn] = 1;
        
        to = MoveTo(move);
        from = MoveFrom(move);
        
        rTo = CastleGetRookTo(from,to);
        rFrom = CastleGetRookFrom(from,to);
        
        fromPiece = WHITE_KING + board->turn;
        rFromPiece = fromPiece - 8;
        
        shiftFrom = 1ull << from;
        shiftTo = 1ull << to;
        rShiftFrom = 1ull << rFrom;
        rShiftTo = 1ull << rTo;
        
        // Update the colour bitboard
        board->colours[board->turn] ^= shiftTo | shiftFrom | rShiftTo | rShiftFrom;
        
        // Update the piece bitboards
        board->pieces[KING] ^= shiftFrom | shiftTo;
        board->pieces[ROOK] ^= rShiftFrom | rShiftTo;
        
        // Update the board array
        board->squares[to] = fromPiece;
        board->squares[from] = EMPTY;
        board->squares[rTo] = rFromPiece;
        board->squares[rFrom] = EMPTY;
        
        // Bitwise-and out the castle changes
        board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
        board->castleRights &= CastleMask[from];
        board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
        
        // Swap the turn
        board->turn = !board->turn;
        
        // Update the PSQT value for the opening
        board->opening += PSQTopening[fromPiece][to]
                        - PSQTopening[fromPiece][from]
                        + PSQTopening[rFromPiece][to]
                        - PSQTopening[rFromPiece][from];
                        
        // Update the PSQT value for the endgame
        board->endgame += PSQTendgame[fromPiece][to]
                        - PSQTendgame[fromPiece][from]
                        + PSQTendgame[rFromPiece][to]
                        - PSQTendgame[rFromPiece][from];
        
        // Update the main zorbist hash
        board->hash ^= ZorbistKeys[fromPiece][from]
                    ^  ZorbistKeys[fromPiece][to]
                    ^  ZorbistKeys[rFromPiece][rFrom]
                    ^  ZorbistKeys[rFromPiece][rTo];
        
        // If there was a possible enpass move, we must
        // xor the main zorbist key for it before moving on
        if (board->epSquare != -1){
            board->hash ^= ZorbistKeys[ENPASS][File(board->epSquare)];
            board->epSquare = -1;
        }
        
        return;
    }
    
    if (MoveType(move) == PROMOTION_MOVE){
        to = MoveTo(move);
        from = MoveFrom(move);
        
        fromType = PieceType(board->squares[from]);
        toType = PieceType(board->squares[to]);
        promotype = 1 + (move >> 14);
        
        fromPiece = board->squares[from];
        toPiece = board->squares[to];
        promoPiece = (promotype << 2) + board->turn;
        
        shiftFrom = 1ull << from;
        shiftTo = 1ull << to;
    
        // Update the colour bitboards
        board->colours[board->turn] ^= shiftFrom | shiftTo;
        board->colours[PieceColour(toPiece)] ^= shiftTo;
        
        // Update the piece bitboards
        board->pieces[PAWN] ^= shiftFrom;
        board->pieces[promotype] ^= shiftTo;
        board->pieces[toType] ^= shiftTo;
        
        // Save the captured piece and update the board array
        undo->capturePiece = toPiece;
        board->squares[to] = promoPiece;
        board->squares[from] = EMPTY;
        
        // Bitwise-and out the castle changes
        board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
        board->castleRights &= CastleMask[to];
        board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
        
        // Swap the turn
        board->turn = !board->turn;
        
        // Update the PSQT value for the opening
        board->opening += PSQTopening[promoPiece][to]
                        - PSQTopening[fromPiece][from] 
                        - PSQTopening[toPiece][to];
                        
        // Update the PSQT value for the endgame
        board->endgame += PSQTendgame[promoPiece][to]
                        - PSQTendgame[fromPiece][from]
                        - PSQTendgame[toPiece][to];
        
        // Update the main zorbist hash
        board->hash ^= ZorbistKeys[fromPiece][from]
                    ^  ZorbistKeys[promoPiece][to]
                    ^  ZorbistKeys[toPiece][to];
                    
        // Update the pawn zorbist hash
        board->phash^= PawnKeys[fromPiece][from];
        
        if (board->epSquare != -1){
            board->hash ^= ZorbistKeys[ENPASS][File(board->epSquare)];
            board->epSquare = -1;
        }
        
        return;
    }
    
    if (MoveType(move) == ENPASS_MOVE){
        to = MoveTo(move);
        from = MoveFrom(move);
        ep = board->epSquare - 8 + (board->turn << 4);
        
        fromPiece = WHITE_PAWN + board->turn;
        enpassPiece = WHITE_PAWN + !board->turn;
        
        shiftFrom = 1ull << from;
        shiftTo = 1ull << to;
        shiftEnpass = 1ull << ep;
        
        // Update the colour bitboards
        board->colours[!board->turn] ^= shiftEnpass;
        board->colours[board->turn] ^= shiftFrom | shiftTo;
        
        // Update the piece bitboard
        board->pieces[PAWN] ^= shiftEnpass | shiftFrom | shiftTo;
        
        // Save the captured piece and update the board array
        undo->capturePiece = enpassPiece;
        board->squares[to] = fromPiece;
        board->squares[from] = EMPTY;
        board->squares[ep] = EMPTY;
        
        // Swap the turn
        board->turn = !board->turn;
        
        // Update the PSQT value for the opening
        board->opening += PSQTopening[fromPiece][to]
                        - PSQTopening[fromPiece][from]
                        - PSQTopening[enpassPiece][ep];
                        
        // Update the PSQT value for the endgame
        board->endgame += PSQTendgame[fromPiece][to]
                        - PSQTendgame[fromPiece][from]
                        - PSQTendgame[enpassPiece][ep];
        
        // Update the main zorbist key
        board->hash ^= ZorbistKeys[fromPiece][from]
                    ^  ZorbistKeys[fromPiece][to]
                    ^  ZorbistKeys[enpassPiece][ep]
                    ^  ZorbistKeys[ENPASS][File(ep)];
                    
        // Update the pawn zorbist key
        board->phash^= PawnKeys[fromPiece][from]
                    ^  PawnKeys[fromPiece][to]
                    ^  PawnKeys[enpassPiece][ep];
        
        // Reset the enpass square
        board->epSquare = -1;
        
        return;
    }
}

/**
 * Revert a given move to a given board and update all of the
 * necessary information including, opening and endgame values
 * hash signature, piece counts, castling rights, enpassant
 * potentials, and the history of moves made.
 *
 * @param   board   Board pointer to current position
 * @param   move    Move to be applied to the board
 * @param   undo    Undo object with the missing information needed
 */
void revertMove(Board * board, uint16_t move, Undo * undo){
    
    int to, from;
    int rTo, rFrom;
    int fromType, toType;
    int promotype, ep;
    uint64_t shiftFrom, shiftTo;
    uint64_t rShiftFrom, rShiftTo;
    uint64_t shiftEnpass;
    
    board->numMoves--;
    
    if (MoveType(move) == NORMAL_MOVE){
        to = MoveTo(move);
        from = MoveFrom(move);
        
        fromType = PieceType(board->squares[to]);
        toType = PieceType(undo->capturePiece);
        
        shiftFrom = 1ull << from;
        shiftTo = 1ull << to;
        
        board->colours[undo->turn] ^= shiftFrom | shiftTo;
        board->colours[PieceColour(undo->capturePiece)] |= shiftTo;
        
        board->pieces[fromType] ^= shiftTo | shiftFrom;
        board->pieces[toType] |= shiftTo;
        
        board->squares[from] = board->squares[to];
        board->squares[to] = undo->capturePiece;
        
        board->castleRights = undo->castleRights;
        board->turn = undo->turn;
        board->epSquare = undo->epSquare;
        board->opening = undo->opening;
        board->endgame = undo->endgame;
        board->phash = undo->phash;
        board->hash = undo->hash;
        return;
    }
    
    if (MoveType(move) == CASTLE_MOVE){
        
        board->hasCastled[undo->turn] = 0;
        
        to = MoveTo(move);
        from = MoveFrom(move);

        rTo = CastleGetRookTo(from,to);
        rFrom = CastleGetRookFrom(from,to);
        
        shiftFrom = 1ull << from;
        shiftTo = 1ull << to;
        rShiftFrom = 1ull << rFrom;
        rShiftTo = 1ull << rTo;
    
        board->colours[undo->turn] ^= shiftFrom | shiftTo | rShiftTo | rShiftFrom;
        
        board->pieces[KING] ^= shiftFrom | shiftTo;
        board->pieces[ROOK] ^= rShiftFrom | rShiftTo;
        
        board->squares[from] = board->squares[to];
        board->squares[to] = EMPTY;
        
        board->squares[rFrom] = board->squares[rTo];
        board->squares[rTo] = EMPTY;
        
        board->castleRights = undo->castleRights;
        board->turn = undo->turn;
        board->epSquare = undo->epSquare;
        board->opening = undo->opening;
        board->endgame = undo->endgame;
        board->phash = undo->phash;
        board->hash = undo->hash;
        return;
    }
    
    if (MoveType(move) == PROMOTION_MOVE){
        to = MoveTo(move);
        from = MoveFrom(move);
        
        fromType = WHITE_PAWN + undo->turn;
        toType = PieceType(undo->capturePiece);
        promotype = 1 + (move >> 14);
        
        shiftFrom = 1ull << from;
        shiftTo = 1ull << to;
    
        board->colours[undo->turn] ^= shiftFrom | shiftTo;
        board->colours[PieceColour(undo->capturePiece)] ^= shiftTo;
        
        board->pieces[PAWN] ^= shiftFrom;
        board->pieces[promotype] ^= shiftTo;
        board->pieces[toType] ^= shiftTo;
        
        board->squares[to] = undo->capturePiece;
        board->squares[from] = WHITE_PAWN + undo->turn;
        
        board->turn = undo->turn;
        board->epSquare = undo->epSquare;
        board->opening = undo->opening;
        board->endgame = undo->endgame;
        board->phash = undo->phash;
        board->hash = undo->hash;
        return;
    }
    
    if (MoveType(move) == ENPASS_MOVE){
        to = MoveTo(move);
        from = MoveFrom(move);
        ep = undo->epSquare - 8 + (undo->turn << 4);
        
        shiftFrom = 1ull << from;
        shiftTo = 1ull << to;
        shiftEnpass = 1ull << ep;
        
        board->colours[!undo->turn] ^= shiftEnpass;
        board->pieces[PAWN] ^= shiftEnpass;
        
        board->colours[undo->turn] ^= shiftFrom | shiftTo;
        board->pieces[PAWN] ^= shiftFrom | shiftTo;
        
        board->squares[from] = board->squares[to];
        board->squares[to] = EMPTY;
        board->squares[ep] = undo->capturePiece;
        
        board->turn = undo->turn;
        board->epSquare = undo->epSquare;
        board->opening = undo->opening;
        board->endgame = undo->endgame;
        board->phash = undo->phash;
        board->hash = undo->hash;
        return;
    }
}


/**
 * Apply a null move to the given board. Store crucial
 * information in the undo pointer passed to the function
 *
 * @param   board   Board pointer to current position
 * @param   undo    Undo object to be filled with information
 *                  which is either hard or impossible to reverse
 *                  with only the board and the move
 */                  
void applyNullMove(Board * board, Undo * undo){
    
    // Store turn, hash and epSquare
    undo->turn = board->turn;
    undo->hash = board->hash;
    undo->epSquare = board->epSquare;
    
    // Swap the turn and update the history
    board->turn = !board->turn;
    board->history[board->numMoves++] = NULL_MOVE;
    
    // Update the key to include the turn change
    board->hash ^= ZorbistKeys[TURN][0];
    
    // Must empty the enpass square and update hash
    if (board->epSquare != -1){
        board->hash ^= ZorbistKeys[ENPASS][File(board->epSquare)];
        board->epSquare = -1;
    }
}

/**
 * Revert a null move from the given board.
 *
 * @param   board   Board pointer to current position
 * @param   undo    Undo object with the missing information needed
 */
void revertNullMove(Board * board, Undo * undo){
    
    // Revert turn, hash, epSquare, and history
    board->turn = !board->turn;
    board->hash = undo->hash;
    board->epSquare = undo->epSquare;
    board->numMoves--;
    
}

/**
 * Print a move in long algebraic form.
 *
 * @param   move    Move to be printed
 */
void printMove(uint16_t move){
    
    int from = MoveFrom(move);
    int to = MoveTo(move);
    
    char fromFile = '1' + (from / 8);
    char toFile = '1' + (to / 8);
    
    char fromRank = 'a' + (from % 8);
    char toRank = 'a' + (to % 8);
    
    printf("%c%c%c%c", fromRank, fromFile, toRank, toFile);
}