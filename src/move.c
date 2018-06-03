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
#include "movegen.h"
#include "psqt.h"
#include "types.h"
#include "zorbist.h"

void applyMove(Board* board, uint16_t move, Undo* undo){
    
    static void (*table[4])(Board*, uint16_t, Undo*) = {
        applyNormalMove, applyCastleMove,
        applyEnpassMove, applyPromotionMove
    };
    
    // Save information that is either hard to reverse,
    // or is not worth the time in order to do so
    undo->hash = board->hash;
    undo->pkhash = board->pkhash;
    undo->kingAttackers = board->kingAttackers;
    undo->turn = board->turn;
    undo->castleRights = board->castleRights;
    undo->epSquare = board->epSquare;
    undo->fiftyMoveRule = board->fiftyMoveRule;
    undo->psqtmat = board->psqtmat;
    
    // Update the hash history and the move count
    board->history[board->numMoves++] = board->hash;
    
    // Update the key to include the turn change
    board->hash ^= ZorbistKeys[TURN][0];
    
    // Always increment the fifty counter
    // We will reset later if needed
    board->fiftyMoveRule += 1;
    
    // Call the proper move application function
    table[MoveType(move) >> 12](board, move, undo);
    
    // Get attackers to the new side to move's King
    board->kingAttackers = attackersToKingSquare(board);
}

void applyNormalMove(Board* board, uint16_t move, Undo* undo){
    
    int to, from, fromType, toType, fromPiece, toPiece;
    uint64_t shiftFrom, shiftTo, enemyPawns;
    
    to = MoveTo(move);
    from = MoveFrom(move);
    
    shiftFrom = 1ull << from;
    shiftTo = 1ull << to;
    
    fromPiece = board->squares[from];
    toPiece = board->squares[to];
    
    fromType = pieceType(fromPiece);
    toType = pieceType(toPiece);
    
    // Reset fifty move rule on a pawn move
    if (fromType == PAWN) board->fiftyMoveRule = 0;
    
    // Reset fifty move rule on a capture
    else if (toPiece != EMPTY) board->fiftyMoveRule = 0;
    
    // Update the colour bitboards
    board->colours[board->turn] ^= shiftFrom | shiftTo;
    board->colours[pieceColour(toPiece)] ^= shiftTo;
    
    // Update the piece bitboards
    board->pieces[toType] ^= shiftTo;
    board->pieces[fromType] ^= shiftFrom | shiftTo;
    
    // Save the captured piece and update the board array
    undo->capturePiece = toPiece;
    board->squares[to] = fromPiece;
    board->squares[from] = EMPTY;
    
    // Bitwise-and out the castle changes for the hash
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
    board->castleRights &= CastleMask[from];
    board->castleRights &= CastleMask[to];
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
    
    // Swap the turn
    board->turn = !board->turn;
    
    // Update the PSQT and Material values
    board->psqtmat += PSQT[fromPiece][to]
                   -  PSQT[fromPiece][from]
                   -  PSQT[toPiece][to];
                    
    // Update the main zorbist hash
    board->hash   ^= ZorbistKeys[fromPiece][from]
                  ^  ZorbistKeys[fromPiece][to]
                  ^  ZorbistKeys[toPiece][to];
                
    // Update the pawn zorbist hash
    board->pkhash ^= PawnKingKeys[fromPiece][from]
                  ^  PawnKingKeys[fromPiece][to]
                  ^  PawnKingKeys[toPiece][to];
                
    // If there was a possible enpass move, we must
    // xor the main zorbist key for it before moving on
    if (board->epSquare != -1)
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];
    
    // Zero out the enpass square for now
    board->epSquare = -1;
    
    // Check to see if this move creates an enpass square
    if (fromType == PAWN && (to - from == 16 || from - to == 16)){
        
        // Determine if there is an enemy pawn that may perform an enpassant.
        // If so, we must factor in the hash for the enpassant square.
        enemyPawns  = board->pieces[PAWN] & board->colours[board->turn];
        enemyPawns &= isolatedPawnMasks(from);
        enemyPawns &= (board->turn == BLACK) ? RANK_4 : RANK_5;
        
        if (enemyPawns){
            board->epSquare = from + ((to-from) >> 1);
            board->hash ^= ZorbistKeys[ENPASS][fileOf(from)];
        } 
    }
}

void applyCastleMove(Board* board, uint16_t move, Undo* undo){
    
    int to, from, rTo, rFrom, fromPiece, rFromPiece;
    uint64_t shiftFrom, shiftTo, rShiftFrom, rShiftTo;
    
    to = MoveTo(move);
    from = MoveFrom(move);
    
    rTo = castleGetRookTo(from, to);
    rFrom = castleGetRookFrom(from, to);
    
    fromPiece = makePiece(KING, board->turn);
    rFromPiece = makePiece(ROOK, board->turn);
    
    shiftFrom = 1ull << from;
    shiftTo = 1ull << to;
    rShiftFrom = 1ull << rFrom;
    rShiftTo = 1ull << rTo;
    
    // Update the colour bitboard
    board->colours[board->turn] ^= shiftTo | shiftFrom | rShiftTo | rShiftFrom;
    
    // Update the piece bitboards
    board->pieces[KING] ^=  shiftFrom |  shiftTo;
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
    
    // Update the PSQT and Material values
    board->psqtmat += PSQT[fromPiece][to]
                    - PSQT[fromPiece][from]
                    + PSQT[rFromPiece][rTo]
                    - PSQT[rFromPiece][rFrom];
    
    // Update the main zorbist hash
    board->hash   ^= ZorbistKeys[fromPiece][from]
                  ^  ZorbistKeys[fromPiece][to]
                  ^  ZorbistKeys[rFromPiece][rFrom]
                  ^  ZorbistKeys[rFromPiece][rTo];
                
    // Update PawnKing hash for the King movement
    board->pkhash ^= PawnKingKeys[fromPiece][from]
                  ^  PawnKingKeys[fromPiece][to];
    
    // If there was a possible enpass move, we must
    // xor the main zorbist key for it before moving on
    if (board->epSquare != -1){
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];
        board->epSquare = -1;
    }
    
    // We don't capture anything. We set this to be safe.
    undo->capturePiece = EMPTY;
}

void applyEnpassMove(Board* board, uint16_t move, Undo* undo){
    
    int to, from, ep, fromPiece, enpassPiece;
    uint64_t shiftFrom, shiftTo, shiftEnpass;
    
    to = MoveTo(move);
    from = MoveFrom(move);
    ep = board->epSquare - 8 + (board->turn << 4);
    
    fromPiece = makePiece(PAWN, board->turn);
    enpassPiece = makePiece(PAWN, !board->turn);
    
    shiftFrom = 1ull << from;
    shiftTo = 1ull << to;
    shiftEnpass = 1ull << ep;
    
    // Reset fifty move rule on a pawn move
    board->fiftyMoveRule = 0;
    
    // Update the colour bitboards
    board->colours[board->turn] ^= shiftFrom | shiftTo;
    board->colours[!board->turn] ^= shiftEnpass;
    
    // Update the piece bitboard
    board->pieces[PAWN] ^= shiftEnpass | shiftFrom | shiftTo;
    
    // Save the captured piece and update the board array
    undo->capturePiece = enpassPiece;
    board->squares[to] = fromPiece;
    board->squares[from] = EMPTY;
    board->squares[ep] = EMPTY;
    
    // Swap the turn
    board->turn = !board->turn;
    
    // Update the PSQT and Material values
    board->psqtmat += PSQT[fromPiece][to]
                    - PSQT[fromPiece][from]
                    - PSQT[enpassPiece][ep];
    
    // Update the main zorbist key
    board->hash   ^= ZorbistKeys[fromPiece][from]
                  ^  ZorbistKeys[fromPiece][to]
                  ^  ZorbistKeys[enpassPiece][ep]
                  ^  ZorbistKeys[ENPASS][fileOf(ep)];
                
    // Update the PawnKing zorbist key
    board->pkhash ^= PawnKingKeys[fromPiece][from]
                  ^  PawnKingKeys[fromPiece][to]
                  ^  PawnKingKeys[enpassPiece][ep];
    
    // Reset the enpass square
    board->epSquare = -1;
}

void applyPromotionMove(Board* board, uint16_t move, Undo* undo){
    
    int to, from, toType, promotype, fromPiece, toPiece, promoPiece;
    uint64_t shiftFrom, shiftTo;
    
    to = MoveTo(move);
    from = MoveFrom(move);
    
    toType = pieceType(board->squares[to]);
    promotype = MovePromoPiece(move);
    
    fromPiece = board->squares[from];
    toPiece = board->squares[to];
    promoPiece = makePiece(promotype, board->turn);
    
    shiftFrom = 1ull << from;
    shiftTo = 1ull << to;
    
    // Reset fifty move rule on a pawn move
    board->fiftyMoveRule = 0;

    // Update the colour bitboards
    board->colours[board->turn] ^= shiftFrom | shiftTo;
    board->colours[pieceColour(toPiece)] ^= shiftTo;
    
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
    
    // Update the PSQT and Material values
    board->psqtmat += PSQT[promoPiece][to]
                    - PSQT[fromPiece][from] 
                    - PSQT[toPiece][to];
    
    // Update the main zorbist hash
    board->hash   ^= ZorbistKeys[fromPiece][from]
                  ^  ZorbistKeys[promoPiece][to]
                  ^  ZorbistKeys[toPiece][to];
                
    // Update the PawnKing zorbist hash
    board->pkhash ^= PawnKingKeys[fromPiece][from];
    
    // If there was a possible enpass move, we must
    // xor the main zorbist key for it before moving on
    if (board->epSquare != -1){
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];
        board->epSquare = -1;
    }
}

void applyNullMove(Board* board, Undo* undo){
    
    // Store turn, hash and epSquare
    undo->hash = board->hash;
    undo->kingAttackers = board->kingAttackers;
    undo->turn = board->turn;
    undo->epSquare = board->epSquare;
    
    // Swap the turn and update the history
    board->turn = !board->turn;
    board->history[board->numMoves++] = NULL_MOVE;
    
    // Update the key to include the turn change
    board->hash ^= ZorbistKeys[TURN][0];
    
    // Must empty the enpass square and update hash
    if (board->epSquare != -1){
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];
        board->epSquare = -1;
    }
}

void revertMove(Board* board, uint16_t move, Undo* undo){
    
    int to, from, rTo, rFrom, fromType, toType, promotype, ep;
    uint64_t shiftFrom, shiftTo, rShiftFrom, rShiftTo, shiftEnpass;
    
    board->numMoves--;
    board->hash = undo->hash;
    board->pkhash = undo->pkhash;
    board->kingAttackers = undo->kingAttackers;
    board->turn = undo->turn;
    board->castleRights = undo->castleRights;
    board->epSquare = undo->epSquare;
    board->fiftyMoveRule = undo->fiftyMoveRule;
    board->psqtmat = undo->psqtmat;
    
    to = MoveTo(move);
    from = MoveFrom(move);
    
    shiftFrom = 1ull << from;
    shiftTo = 1ull << to;
    
    if (MoveType(move) == NORMAL_MOVE){
        
        fromType = pieceType(board->squares[to]);
        toType = pieceType(undo->capturePiece);
        
        board->colours[undo->turn] ^= shiftFrom | shiftTo;
        board->colours[pieceColour(undo->capturePiece)] |= shiftTo;
        
        board->pieces[fromType] ^= shiftTo | shiftFrom;
        board->pieces[toType] |= shiftTo;
        
        board->squares[from] = board->squares[to];
        board->squares[to] = undo->capturePiece;
    }
    
    else if (MoveType(move) == CASTLE_MOVE){
        
        rTo = castleGetRookTo(from, to);
        rFrom = castleGetRookFrom(from, to);
        
        rShiftFrom = 1ull << rFrom;
        rShiftTo = 1ull << rTo;
    
        board->colours[undo->turn] ^= shiftTo | shiftFrom | rShiftTo | rShiftFrom;
        
        board->pieces[KING] ^= shiftFrom | shiftTo;
        board->pieces[ROOK] ^= rShiftFrom | rShiftTo;
        
        board->squares[from] = board->squares[to];
        board->squares[to] = EMPTY;
        
        board->squares[rFrom] = board->squares[rTo];
        board->squares[rTo] = EMPTY;
    }
    
    else if (MoveType(move) == PROMOTION_MOVE){
        
        fromType = makePiece(PAWN, undo->turn);
        toType = pieceType(undo->capturePiece);
        promotype = MovePromoPiece(move);
        
        board->colours[undo->turn] ^= shiftFrom | shiftTo;
        board->colours[pieceColour(undo->capturePiece)] ^= shiftTo;
        
        board->pieces[PAWN] ^= shiftFrom;
        board->pieces[promotype] ^= shiftTo;
        board->pieces[toType] ^= shiftTo;
        
        board->squares[to] = undo->capturePiece;
        board->squares[from] = WHITE_PAWN + undo->turn;
    }
    
    else { // (MoveType(move) == ENPASS_MOVE)
        
        ep = undo->epSquare - 8 + (undo->turn << 4);
        
        shiftEnpass = 1ull << ep;
        
        board->colours[!undo->turn] ^= shiftEnpass;
        board->pieces[PAWN] ^= shiftEnpass;
        
        board->colours[undo->turn] ^= shiftFrom | shiftTo;
        board->pieces[PAWN] ^= shiftFrom | shiftTo;
        
        board->squares[from] = board->squares[to];
        board->squares[to] = EMPTY;
        board->squares[ep] = undo->capturePiece;
    }
}

void revertNullMove(Board* board, Undo* undo){
    board->hash = undo->hash;
    board->kingAttackers = undo->kingAttackers;
    board->turn = !board->turn;
    board->epSquare = undo->epSquare;
    board->numMoves--;
}

void moveToString(uint16_t move, char *str) {

    squareToString(MoveFrom(move), &str[0]);
    squareToString(MoveTo(move), &str[2]);

    if (MoveType(move) == PROMOTION_MOVE) {
        str[4] = PieceLabel[BLACK][MovePromoPiece(move)];
        str[5] = '\0';
    }
}
