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

void applyMove(Board *board, uint16_t move, Undo *undo) {

    static void (*table[4])(Board*, uint16_t, Undo*) = {
        applyNormalMove, applyCastleMove,
        applyEnpassMove, applyPromotionMove
    };

    undo->hash = board->hash;
    undo->pkhash = board->pkhash;
    undo->kingAttackers = board->kingAttackers;
    undo->castleRights = board->castleRights;
    undo->epSquare = board->epSquare;
    undo->fiftyMoveRule = board->fiftyMoveRule;
    undo->psqtmat = board->psqtmat;

    // Store hash history for three-fold checking
    board->history[board->numMoves++] = board->hash;

    // Always update fifty move, functions will reset
    board->fiftyMoveRule += 1;

    // Always update for turn and chances to enpass square
    board->hash ^= ZorbistKeys[TURN][0];
    if (board->epSquare != -1)
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];

    // Run the correct move function
    table[MoveType(move) >> 12](board, move, undo);

    // No function updated epsquare, so we reset
    if (board->epSquare == undo->epSquare) board->epSquare = -1;

    // No function updates this, so we do it here
    board->turn = !board->turn;

    // Need king attackers to verify move legality
    board->kingAttackers = attackersToKingSquare(board);
}

void applyNormalMove(Board *board, uint16_t move, Undo *undo) {

    const int from = MoveFrom(move);
    const int to = MoveTo(move);

    const int fromPiece = board->squares[from];
    const int toPiece = board->squares[to];

    const int fromType = pieceType(fromPiece);
    const int toType = pieceType(toPiece);
    const int toColour = pieceColour(toPiece);

    if (fromType == PAWN || toPiece != EMPTY)
        board->fiftyMoveRule = 0;

    board->pieces[fromType]     ^= (1ull << from) ^ (1ull << to);
    board->colours[board->turn] ^= (1ull << from) ^ (1ull << to);

    board->pieces[toType]    ^= (1ull << to);
    board->colours[toColour] ^= (1ull << to);

    board->squares[from] = EMPTY;
    board->squares[to]   = fromPiece;
    undo->capturePiece   = toPiece;

    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
    board->castleRights &= CastleMask[from] & CastleMask[to];
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];

    board->psqtmat += PSQT[fromPiece][to]
                   -  PSQT[fromPiece][from]
                   -  PSQT[toPiece][to];

    board->hash    ^= ZorbistKeys[fromPiece][from]
                   ^  ZorbistKeys[fromPiece][to]
                   ^  ZorbistKeys[toPiece][to];

    board->pkhash  ^= PawnKingKeys[fromPiece][from]
                   ^  PawnKingKeys[fromPiece][to]
                   ^  PawnKingKeys[toPiece][to];

    if (fromType == PAWN && (to ^ from) == 16) {

        const uint64_t enemyPawns =  board->pieces[PAWN]
                                  &  board->colours[!board->turn]
                                  &  isolatedPawnMasks(from)
                                  & (board->turn == WHITE ? RANK_4 : RANK_5);
        if (enemyPawns) {
            board->epSquare = board->turn == WHITE ? from + 8 : from - 8;
            board->hash ^= ZorbistKeys[ENPASS][fileOf(from)];
        }
    }
}

void applyCastleMove(Board *board, uint16_t move, Undo *undo) {

    const int from = MoveFrom(move);
    const int to = MoveTo(move);

    const int rFrom = castleGetRookFrom(from, to);
    const int rTo = castleGetRookTo(from, to);

    const int fromPiece = makePiece(KING, board->turn);
    const int rFromPiece = makePiece(ROOK, board->turn);

    board->pieces[KING]         ^= (1ull << from) ^ (1ull << to);
    board->colours[board->turn] ^= (1ull << from) ^ (1ull << to);

    board->pieces[ROOK]         ^= (1ull << rFrom) ^ (1ull << rTo);
    board->colours[board->turn] ^= (1ull << rFrom) ^ (1ull << rTo);

    board->squares[from]  = EMPTY;
    board->squares[to]    = fromPiece;

    board->squares[rFrom] = EMPTY;
    board->squares[rTo]   = rFromPiece;

    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
    board->castleRights &= CastleMask[from];
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];

    board->psqtmat += PSQT[fromPiece][to]
                    - PSQT[fromPiece][from]
                    + PSQT[rFromPiece][rTo]
                    - PSQT[rFromPiece][rFrom];

    board->hash    ^= ZorbistKeys[fromPiece][from]
                   ^  ZorbistKeys[fromPiece][to]
                   ^  ZorbistKeys[rFromPiece][rFrom]
                   ^  ZorbistKeys[rFromPiece][rTo];

    board->pkhash  ^= PawnKingKeys[fromPiece][from]
                   ^  PawnKingKeys[fromPiece][to];

    undo->capturePiece = EMPTY;
}

void applyEnpassMove(Board *board, uint16_t move, Undo *undo) {

    const int from = MoveFrom(move);
    const int to = MoveTo(move);
    const int ep = board->epSquare - 8 + (board->turn << 4);

    const int fromPiece = makePiece(PAWN, board->turn);
    const int enpassPiece = makePiece(PAWN, !board->turn);

    board->fiftyMoveRule = 0;

    board->pieces[PAWN]         ^= (1ull << from) ^ (1ull << to);
    board->colours[board->turn] ^= (1ull << from) ^ (1ull << to);

    board->pieces[PAWN]          ^= (1ull << ep);
    board->colours[!board->turn] ^= (1ull << ep);

    board->squares[from] = EMPTY;
    board->squares[to]   = fromPiece;
    board->squares[ep]   = EMPTY;
    undo->capturePiece   = enpassPiece;

    board->psqtmat += PSQT[fromPiece][to]
                    - PSQT[fromPiece][from]
                    - PSQT[enpassPiece][ep];

    board->hash    ^= ZorbistKeys[fromPiece][from]
                   ^  ZorbistKeys[fromPiece][to]
                   ^  ZorbistKeys[enpassPiece][ep];

    board->pkhash  ^= PawnKingKeys[fromPiece][from]
                   ^  PawnKingKeys[fromPiece][to]
                   ^  PawnKingKeys[enpassPiece][ep];
}

void applyPromotionMove(Board *board, uint16_t move, Undo *undo) {

    const int from = MoveFrom(move);
    const int to = MoveTo(move);

    const int fromPiece = board->squares[from];
    const int toPiece = board->squares[to];
    const int promoPiece = makePiece(MovePromoPiece(move), board->turn);

    const int toType = pieceType(toPiece);
    const int toColour = pieceColour(toPiece);
    const int promotype = MovePromoPiece(move);

    board->fiftyMoveRule = 0;

    board->pieces[PAWN]         ^= (1ull << from);
    board->pieces[promotype]    ^= (1ull << to);
    board->colours[board->turn] ^= (1ull << from) ^ (1ull << to);

    board->pieces[toType]    ^= (1ull << to);
    board->colours[toColour] ^= (1ull << to);

    board->squares[from] = EMPTY;
    board->squares[to]   = promoPiece;
    undo->capturePiece   = toPiece;

    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
    board->castleRights &= CastleMask[to];
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];

    board->psqtmat += PSQT[promoPiece][to]
                    - PSQT[fromPiece][from]
                    - PSQT[toPiece][to];

    board->hash    ^= ZorbistKeys[fromPiece][from]
                   ^  ZorbistKeys[promoPiece][to]
                   ^  ZorbistKeys[toPiece][to];

    board->pkhash  ^= PawnKingKeys[fromPiece][from];
}

void applyNullMove(Board *board, Undo *undo) {

    undo->hash = board->hash;
    undo->epSquare = board->epSquare;

    board->turn = !board->turn;
    board->history[board->numMoves++] = NULL_MOVE;

    board->hash ^= ZorbistKeys[TURN][0];
    if (board->epSquare != -1)
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];

    board->epSquare = -1;
}

void revertMove(Board *board, uint16_t move, Undo *undo) {

    const int to = MoveTo(move);
    const int from = MoveFrom(move);

    board->turn = !board->turn;
    board->numMoves--;
    board->hash = undo->hash;
    board->pkhash = undo->pkhash;
    board->kingAttackers = undo->kingAttackers;
    board->castleRights = undo->castleRights;
    board->epSquare = undo->epSquare;
    board->fiftyMoveRule = undo->fiftyMoveRule;
    board->psqtmat = undo->psqtmat;

    if (MoveType(move) == NORMAL_MOVE){

        const int fromType = pieceType(board->squares[to]);
        const int toType = pieceType(undo->capturePiece);
        const int toColour = pieceColour(undo->capturePiece);

        board->pieces[fromType]     ^= (1ull << from) ^ (1ull << to);
        board->colours[board->turn] ^= (1ull << from) ^ (1ull << to);

        board->pieces[toType]    ^= (1ull << to);
        board->colours[toColour] ^= (1ull << to);

        board->squares[from] = board->squares[to];
        board->squares[to] = undo->capturePiece;
    }

    else if (MoveType(move) == CASTLE_MOVE){

        const int rFrom = castleGetRookFrom(from, to);
        const int rTo = castleGetRookTo(from, to);

        board->pieces[KING]         ^= (1ull << from) ^ (1ull << to);
        board->colours[board->turn] ^= (1ull << from) ^ (1ull << to);

        board->pieces[ROOK]         ^= (1ull << rFrom) ^ (1ull << rTo);
        board->colours[board->turn] ^= (1ull << rFrom) ^ (1ull << rTo);

        board->squares[from] = board->squares[to];
        board->squares[to] = EMPTY;

        board->squares[rFrom] = board->squares[rTo];
        board->squares[rTo] = EMPTY;
    }

    else if (MoveType(move) == PROMOTION_MOVE){

        const int toType = pieceType(undo->capturePiece);
        const int toColour = pieceColour(undo->capturePiece);
        const int promotype = MovePromoPiece(move);

        board->pieces[PAWN]         ^= (1ull << from);
        board->pieces[promotype]    ^= (1ull << to);
        board->colours[board->turn] ^= (1ull << from) ^ (1ull << to);

        board->pieces[toType]    ^= (1ull << to);
        board->colours[toColour] ^= (1ull << to);

        board->squares[from] = makePiece(PAWN, board->turn);
        board->squares[to] = undo->capturePiece;
    }

    else { // (MoveType(move) == ENPASS_MOVE)

        assert(MoveType(move) == ENPASS_MOVE);

        const int ep = undo->epSquare - 8 + (board->turn << 4);

        board->pieces[PAWN]         ^= (1ull << from) ^ (1ull << to);
        board->colours[board->turn] ^= (1ull << from) ^ (1ull << to);

        board->pieces[PAWN]          ^= (1ull << ep);
        board->colours[!board->turn] ^= (1ull << ep);

        board->squares[from] = board->squares[to];
        board->squares[to] = EMPTY;
        board->squares[ep] = undo->capturePiece;
    }
}

void revertNullMove(Board *board, Undo *undo) {
    board->hash = undo->hash;
    board->kingAttackers = 0ull;
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
