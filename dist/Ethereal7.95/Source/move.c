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

#include "board.h"
#include "castle.h"
#include "types.h"
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
                    which is either hard or impossible to reverse
                    with only the board and the move
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
    
    undo->epSquare = board->epSquare;
    undo->turn = board->turn;
    undo->castleRights = board->castleRights;
    undo->opening = board->opening;
    undo->endgame = board->endgame;
    undo->phash = board->phash;
    undo->hash = board->hash;
    
    board->history[board->numMoves++] = board->hash;
    
    if (MoveType(move) == NORMAL_MOVE){
        to = MoveTo(move);
        from = MoveFrom(move);
        
        shiftFrom = 1ull << from;
        shiftTo = 1ull << to;
        
        fromPiece = board->squares[from];
        toPiece = board->squares[to];
        
        fromType = PieceType(fromPiece);
        toType = PieceType(toPiece);
        
        board->colours[board->turn] ^= shiftFrom | shiftTo;
        board->colours[PieceColour(toPiece)] ^= shiftTo;
        
        board->pieces[toType] ^= shiftTo;
        board->pieces[fromType] ^= shiftFrom | shiftTo;
        
        undo->capturePiece = toPiece;
        board->squares[to] = fromPiece;
        board->squares[from] = EMPTY;
        
        board->castleRights &= CastleMask[from];
        board->turn = !board->turn;
        
        board->opening += PSQTopening[fromPiece][to]
                        - PSQTopening[fromPiece][from]
                        - PSQTopening[toPiece][to];
                        
        board->endgame += PSQTendgame[fromPiece][to]
                        - PSQTendgame[fromPiece][from]
                        - PSQTendgame[toPiece][to];
        
        board->hash ^= ZorbistKeys[fromPiece][from]
                    ^  ZorbistKeys[fromPiece][to]
                    ^  ZorbistKeys[toPiece][to];
                    
        board->phash^= PawnKeys[fromPiece][from]
                    ^  PawnKeys[fromPiece][to]
                    ^  PawnKeys[toPiece][to];
                    
        board->epSquare = ((!fromType) & (abs(to-from) == 16)) * (from + ((to-from)>>1));
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
        
        board->colours[board->turn] ^= shiftTo | shiftFrom | rShiftTo | rShiftFrom;
        
        board->pieces[KING] ^= shiftFrom | shiftTo;
        board->pieces[ROOK] ^= rShiftFrom | rShiftTo;
        
        board->squares[to] = fromPiece;
        board->squares[from] = EMPTY;
        
        board->squares[rTo] = rFromPiece;
        board->squares[rFrom] = EMPTY;
        
        board->castleRights &= CastleMask[from];
        board->turn = !board->turn;
        
        board->opening += PSQTopening[fromPiece][to]
                        - PSQTopening[fromPiece][from]
                        + PSQTopening[rFromPiece][to]
                        - PSQTopening[rFromPiece][from];
                        
        board->endgame += PSQTendgame[fromPiece][to]
                        - PSQTendgame[fromPiece][from]
                        + PSQTendgame[rFromPiece][to]
                        - PSQTendgame[rFromPiece][from];
        
        board->hash ^= ZorbistKeys[fromPiece][from]
                    ^  ZorbistKeys[fromPiece][to]
                    ^  ZorbistKeys[rFromPiece][rFrom]
                    ^  ZorbistKeys[rFromPiece][rTo];
                    
        board->phash^= PawnKeys[fromPiece][from]
                    ^  PawnKeys[fromPiece][to]
                    ^  PawnKeys[rFromPiece][rFrom]
                    ^  PawnKeys[rFromPiece][rTo];
        
        board->epSquare = -1;
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
    
        board->colours[board->turn] ^= shiftFrom | shiftTo;
        board->colours[PieceColour(toPiece)] ^= shiftTo;
        
        board->pieces[PAWN] ^= shiftFrom;
        board->pieces[promotype] ^= shiftTo;
        board->pieces[toType] ^= shiftTo;
        
        undo->capturePiece = toPiece;
        board->squares[to] = promoPiece;
        board->squares[from] = EMPTY;
        
        board->turn = !board->turn;
        
        board->opening += PSQTopening[promoPiece][to]
                        - PSQTopening[fromPiece][from] 
                        - PSQTopening[toPiece][to];
                        
        board->endgame += PSQTendgame[promoPiece][to]
                        - PSQTendgame[fromPiece][from]
                        - PSQTendgame[toPiece][to];
        
        board->hash ^= ZorbistKeys[fromPiece][from]
                    ^  ZorbistKeys[promoPiece][to]
                    ^  ZorbistKeys[toPiece][to];
                    
        board->phash^= PawnKeys[fromPiece][from]
                    ^  PawnKeys[promoPiece][to]
                    ^  PawnKeys[toPiece][to];
        
        board->epSquare = -1;
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
        
        board->colours[!board->turn] ^= shiftEnpass;
        board->pieces[PAWN] ^= shiftEnpass;
        
        board->colours[board->turn] ^= shiftFrom | shiftTo;
        board->pieces[PAWN] ^= shiftFrom | shiftTo;
        
        board->squares[to] = fromPiece;
        board->squares[from] = EMPTY;
        
        undo->capturePiece = enpassPiece;
        board->squares[ep] = EMPTY;
        
        board->turn = !board->turn;
        
        board->opening += PSQTopening[fromPiece][to]
                        - PSQTopening[fromPiece][from]
                        - PSQTopening[enpassPiece][ep];
                        
        board->endgame += PSQTendgame[fromPiece][to]
                        - PSQTendgame[fromPiece][from]
                        - PSQTendgame[enpassPiece][ep];
        
        board->hash ^= ZorbistKeys[fromPiece][from]
                    ^  ZorbistKeys[fromPiece][to]
                    ^  ZorbistKeys[enpassPiece][ep];
                    
        board->phash^= PawnKeys[fromPiece][from]
                    ^  PawnKeys[fromPiece][to]
                    ^  PawnKeys[enpassPiece][ep];
        
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