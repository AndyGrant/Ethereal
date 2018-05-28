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
#include "piece.h"
#include "psqt.h"
#include "zorbist.h"

static void applyNormalMove(Board* board, uint16_t move, Undo* undo) {

    const int from = MoveFrom(move), to = MoveTo(move);
    const int toPiece = board->squares[to];
    const int fromType = PieceType(board->squares[from]);

    // Pawn moves and captures reset the 50 move counter
    if (fromType == PAWN || toPiece != EMPTY)
        board->fiftyMoveRule = 0;

    // Lift piece on 'from' square
    clearSquare(board, from);

    // Remove captured piece on 'to' square (if any)
    if (toPiece != EMPTY)
        clearSquare(board, to);

    // Drop piece on 'to' square
    setSquare(board, board->turn, fromType, to);

    // Save the captured piece for undo
    undo->capturePiece = toPiece;

    // Bitwise-and out the castle changes for the hash
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
    board->castleRights &= CastleMask[from];
    board->castleRights &= CastleMask[to];
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];

    // Swap the turn
    board->turn ^= BLACK;

    // If there was a possible enpass move, we must
    // xor the main zorbist key for it before moving on
    if (board->epSquare != -1)
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];

    // Zero out the enpass square for now
    board->epSquare = -1;

    // Check to see if this move creates an enpass square
    if (fromType == PAWN && (to - from == 16 || from - to == 16)) {
        // Determine if there is an enemy pawn that may perform an enpassant.
        // If so, we must factor in the hash for the enpassant square.
        uint64_t enemyPawns = board->pieces[PAWN] & board->colours[board->turn];
        enemyPawns &= isolatedPawnMasks(from);
        enemyPawns &= (board->turn == BLACK) ? RANK_4 : RANK_5;

        if (enemyPawns) {
            board->epSquare = from + ((to-from) >> 1);
            board->hash ^= ZorbistKeys[ENPASS][fileOf(from)];
        }
    }
}

static void applyCastleMove(Board* board, uint16_t move) {

    const int from = MoveFrom(move);
    const int to = MoveTo(move);

    // Lift King and Rook
    clearSquare(board, from);
    clearSquare(board, castleGetRookFrom(from, to));

    // Drop King and Rook into castled position
    setSquare(board, board->turn, KING, to);
    setSquare(board, board->turn, ROOK, castleGetRookTo(from, to));

    // Bitwise-and out the castle changes
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
    board->castleRights &= CastleMask[from];
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];

    // Swap the turn
    board->turn ^= BLACK;

    // If there was a possible enpass move, we must
    // xor the main zorbist key for it before moving on
    if (board->epSquare != -1) {
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];
        board->epSquare = -1;
    }
}

static void applyEnpassMove(Board* board, uint16_t move) {

    const int ep = board->epSquare - 8 + (board->turn << 4);

    // Reset fifty move rule on a pawn move
    board->fiftyMoveRule = 0;

    // Lift pawn on 'from' square, and ep-captured pawn on 'ep' square
    clearSquare(board, MoveFrom(move));
    clearSquare(board, ep);

    // Drop pawn on 'to' square
    setSquare(board, board->turn, PAWN, MoveTo(move));

    // Swap the turn
    board->turn ^= BLACK;

    // Update the main zorbist key
    board->hash ^= ZorbistKeys[ENPASS][fileOf(ep)];

    // Reset the enpass square
    board->epSquare = -1;
}

static void applyPromotionMove(Board* board, uint16_t move, Undo* undo) {

    const int from = MoveFrom(move), to = MoveTo(move);
    const int toPiece = board->squares[to];

    // Reset fifty move rule on a pawn move
    board->fiftyMoveRule = 0;

    // Lift piece on 'from' square
    clearSquare(board, from);

    // Remove captured piece on 'to' square (if any)
    if (toPiece != EMPTY)
        clearSquare(board, to);

    // Drop piece on 'to' square
    setSquare(board, board->turn, MovePromoPiece(move), to);

    // Save the captured piece for undo
    undo->capturePiece = toPiece;

    // Bitwise-and out the castle changes
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
    board->castleRights &= CastleMask[to];
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];

    // Swap the turn
    board->turn ^= BLACK;

    // If there was a possible enpass move, we must
    // xor the main zorbist key for it before moving on
    if (board->epSquare != -1) {
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];
        board->epSquare = -1;
    }
}

void applyMove(Board* board, uint16_t move, Undo* undo) {

    // Save information that is either hard to reverse,
    // or is not worth the time in order to do so
    undo->hash = board->hash;
    undo->kingAttackers = board->kingAttackers;
    undo->turn = board->turn;
    undo->castleRights = board->castleRights;
    undo->epSquare = board->epSquare;
    undo->fiftyMoveRule = board->fiftyMoveRule;
    undo->capturePiece = EMPTY;

    // Update the hash history and the move count
    board->history[board->numMoves++] = board->hash;

    // Update the key to include the turn change
    board->hash ^= ZorbistKeys[TURN][0];

    // Always increment the fifty counter
    // We will reset later if needed
    board->fiftyMoveRule += 1;

    // Call the proper move application function
    if (MoveType(move) == NORMAL_MOVE)
        applyNormalMove(board, move, undo);
    else if (MoveType(move) == CASTLE_MOVE)
        applyCastleMove(board, move);
    else if (MoveType(move) == PROMOTION_MOVE)
        applyPromotionMove(board, move, undo);
    else {
        assert(MoveType(move) == ENPASS_MOVE);
        applyEnpassMove(board, move);
    }

    // Get attackers to the new side to move's King
    board->kingAttackers = attackersToKingSquare(board);
}

void applyNullMove(Board* board, Undo* undo) {
    
    // Store turn, hash and epSquare
    undo->hash = board->hash;
    undo->kingAttackers = board->kingAttackers;
    undo->turn = board->turn;
    undo->epSquare = board->epSquare;
    
    // Swap the turn and update the history
    board->turn ^= BLACK;
    board->history[board->numMoves++] = NULL_MOVE;
    
    // Update the key to include the turn change
    board->hash ^= ZorbistKeys[TURN][0];
    
    // Must empty the enpass square and update hash
    if (board->epSquare != -1){
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];
        board->epSquare = -1;
    }
}

void revertMove(Board* board, uint16_t move, Undo* undo) {

    board->numMoves--;
    board->kingAttackers = undo->kingAttackers;
    board->castleRights = undo->castleRights;
    board->epSquare = undo->epSquare;
    board->fiftyMoveRule = undo->fiftyMoveRule;
    
    const int from = MoveFrom(move), to = MoveTo(move);
    
    if (MoveType(move) == NORMAL_MOVE) {
        setSquare(board, undo->turn, PieceType(board->squares[to]), from);
        clearSquare(board, to);

        if (undo->capturePiece != EMPTY)
            setSquare(board, board->turn, PieceType(undo->capturePiece), to);
    } else if (MoveType(move) == CASTLE_MOVE) {
        clearSquare(board, to);
        setSquare(board, undo->turn, KING, from);
        clearSquare(board, castleGetRookTo(from, to));
        setSquare(board, undo->turn, ROOK, castleGetRookFrom(from, to));
    } else if (MoveType(move) == PROMOTION_MOVE) {
        clearSquare(board, to);
        setSquare(board, undo->turn, PAWN, from);

        if (undo->capturePiece != EMPTY)
            setSquare(board, board->turn, PieceType(undo->capturePiece), to);
    } else {
        assert(MoveType(move) == ENPASS_MOVE);
        const int ep = undo->epSquare - 8 + (undo->turn << 4);
        clearSquare(board, to);
        setSquare(board, undo->turn, PAWN, from);
        setSquare(board, board->turn, PAWN, ep);
    }

    board->hash = undo->hash;
    board->turn = undo->turn;
}

void revertNullMove(Board* board, Undo* undo){
    board->hash = undo->hash;
    board->kingAttackers = undo->kingAttackers;
    board->turn ^= BLACK;
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
