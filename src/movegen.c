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

#include <stdint.h>
#include <assert.h>

#include "attacks.h"
#include "board.h"
#include "bitboards.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "types.h"

/* For Building Move Lists For Each Piece Type */

static void buildEnpassMoves(uint16_t* moves, int* size, uint64_t attacks, int epsq){
    while (attacks){
        int sq = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq, epsq, ENPASS_MOVE);
    }
}

static void buildPawnMoves(uint16_t* moves, int* size, uint64_t attacks, int delta){
    while (attacks){
        int sq = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq + delta, sq, NORMAL_MOVE);
    }
}

static void buildPawnPromotions(uint16_t* moves, int* size, uint64_t attacks, int delta){
    while (attacks){
        int sq = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq + delta, sq,  QUEEN_PROMO_MOVE);
        moves[(*size)++] = MoveMake(sq + delta, sq,   ROOK_PROMO_MOVE);
        moves[(*size)++] = MoveMake(sq + delta, sq, BISHOP_PROMO_MOVE);
        moves[(*size)++] = MoveMake(sq + delta, sq, KNIGHT_PROMO_MOVE);
    }
}

static void buildNonPawnMoves(uint16_t* moves, int* size, uint64_t attacks, int sq){
    while (attacks){
        int tg = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq, tg, NORMAL_MOVE);
    }
}

static void buildKnightMoves(uint16_t* moves, int* size, uint64_t pieces, uint64_t targets){
    while (pieces){
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, knightAttacks(sq) & targets, sq);
    }
}

static void buildBishopAndQueenMoves(uint16_t* moves, int* size, uint64_t pieces, uint64_t occupied, uint64_t targets){
    while (pieces){
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, bishopAttacks(sq, occupied) & targets, sq);
    }
}

static void buildRookAndQueenMoves(uint16_t* moves, int* size, uint64_t pieces, uint64_t occupied, uint64_t targets){
    while (pieces){
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, rookAttacks(sq, occupied) & targets, sq);
    }
}

static void buildKingMoves(uint16_t* moves, int* size, uint64_t pieces, uint64_t targets){
    int sq = getlsb(pieces);
    buildNonPawnMoves(moves, size, kingAttacks(sq) & targets, sq);
}

/* For Generating Attack BitBoards */

uint64_t pawnLeftAttacks(uint64_t pawns, uint64_t targets, int colour){
    return targets & (colour == WHITE ? (pawns << 7) & ~FILE_H
                                      : (pawns >> 7) & ~FILE_A);
}

uint64_t pawnRightAttacks(uint64_t pawns, uint64_t targets, int colour){
    return targets & (colour == WHITE ? (pawns << 9) & ~FILE_A
                                      : (pawns >> 9) & ~FILE_H);
}

uint64_t pawnAttackSpan(uint64_t pawns, uint64_t targets, int colour){
    return pawnLeftAttacks(pawns, targets, colour)
        | pawnRightAttacks(pawns, targets, colour);
}

uint64_t pawnAdvance(uint64_t pawns, uint64_t occupied, int colour){
    return ~occupied & (colour == WHITE ? (pawns << 8) : (pawns >> 8));
}

uint64_t pawnEnpassCaptures(uint64_t pawns, int epsq, int colour){
    return epsq == -1 ? 0ull : pawnAttacks(!colour, epsq) & pawns;
}

/* For Building Full Move Lists */

void genAllLegalMoves(Board* board, uint16_t* moves, int* size){

    Undo undo[1];
    int i, psuedoSize = 0;
    uint16_t psuedoMoves[MAX_MOVES];

    genAllMoves(board, psuedoMoves, &psuedoSize);

    // Check each move for legality before copying
    for (i = 0; i < psuedoSize; i++){
        applyMove(board, psuedoMoves[i], undo);
        if (isNotInCheck(board, !board->turn))
            moves[(*size)++] = psuedoMoves[i];
        revertMove(board, psuedoMoves[i], undo);
    }
}

void genAllMoves(Board* board, uint16_t* moves, int* size){

    int noisy = 0, quiet = 0;

    genAllNoisyMoves(board, moves, &noisy);

    genAllQuietMoves(board, moves + noisy, &quiet);

    *size = noisy + quiet;
}

void genAllNoisyMoves(Board* board, uint16_t* moves, int* size){

    const int forwardShift = board->turn == WHITE ? -8 : 8;
    const int leftShift    = board->turn == WHITE ? -7 : 7;
    const int rightShift   = board->turn == WHITE ? -9 : 9;

    uint64_t destinations;
    uint64_t pawnEnpass;
    uint64_t pawnLeft;
    uint64_t pawnRight;
    uint64_t pawnPromoForward;
    uint64_t pawnPromoLeft;
    uint64_t pawnPromoRight;

    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy    = board->colours[!board->turn];

    uint64_t empty    = ~(friendly | enemy);
    uint64_t occupied = ~empty;

    uint64_t myPawns   = friendly &  board->pieces[PAWN];
    uint64_t myKnights = friendly &  board->pieces[KNIGHT];
    uint64_t myBishops = friendly & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t myRooks   = friendly & (board->pieces[ROOK]   | board->pieces[QUEEN]);
    uint64_t myKings   = friendly &  board->pieces[KING];

    // If there are two threats to the king, the only moves
    // which could be legal are captures made by the king
    if (several(board->kingAttackers)){
        buildKingMoves(moves, size, myKings, enemy);
        return;
    }

    // If there is one threat to the king and we are only looking for
    // noisy moves, then the only moves possible are captures of the
    // attacking piece, blocking the attacking piece via promotion,
    // and blocking the attacking piece via enpass. Therefore, we will
    // always generate all of the promotion and enpass moves
    else if (board->kingAttackers)
        destinations = board->kingAttackers;

    // No threats to the king, so any target square with an enemy piece,
    // or a move which is already noisy, is permitted in this position
    else
        destinations = enemy;

    // Compute bitboards for each type of pawn movement
    pawnEnpass       = pawnEnpassCaptures(myPawns, board->epSquare, board->turn);
    pawnLeft         = pawnLeftAttacks(myPawns, enemy, board->turn);
    pawnRight        = pawnRightAttacks(myPawns, enemy, board->turn);
    pawnPromoForward = pawnAdvance(myPawns, occupied, board->turn) & PROMOTION_RANKS;
    pawnPromoLeft    = pawnLeft & PROMOTION_RANKS; pawnLeft &= ~PROMOTION_RANKS;
    pawnPromoRight   = pawnRight & PROMOTION_RANKS; pawnRight &= ~PROMOTION_RANKS;

    // Generate all enpassant captures
    buildEnpassMoves(moves, size, pawnEnpass, board->epSquare);

    // Generate all pawn captures that are not promotions
    buildPawnMoves(moves, size, pawnLeft & destinations, leftShift);
    buildPawnMoves(moves, size, pawnRight & destinations, rightShift);

    // Generate all pawn promotions
    buildPawnPromotions(moves, size, pawnPromoForward, forwardShift);
    buildPawnPromotions(moves, size, pawnPromoLeft, leftShift);
    buildPawnPromotions(moves, size, pawnPromoRight, rightShift);

    // Generate attacks for all non pawn pieces
    buildKnightMoves(moves, size, myKnights, destinations);
    buildBishopAndQueenMoves(moves, size, myBishops, occupied, destinations);
    buildRookAndQueenMoves(moves, size, myRooks, occupied, destinations);
    buildKingMoves(moves, size, myKings, enemy);
}

void genAllQuietMoves(Board* board, uint16_t* moves, int* size){

    const uint64_t rank3Rel = board->turn == WHITE ? RANK_3 : RANK_6;
    const int forwardShift  = board->turn == WHITE ?     -8 :      8;

    uint64_t destinations;

    uint64_t pawnForwardOne;
    uint64_t pawnForwardTwo;

    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy = board->colours[!board->turn];

    uint64_t empty    = ~(friendly | enemy);
    uint64_t occupied = ~empty;

    uint64_t myPawns   = friendly &  board->pieces[PAWN];
    uint64_t myKnights = friendly &  board->pieces[KNIGHT];
    uint64_t myBishops = friendly & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t myRooks   = friendly & (board->pieces[ROOK]   | board->pieces[QUEEN]);
    uint64_t myKings   = friendly &  board->pieces[KING];

    // If there are two threats to the king, the only moves which
    // could be legal are moves made by the king, except castling
    if (several(board->kingAttackers)){
        buildKingMoves(moves, size, myKings, empty);
        return;
    }

    // If there is one threat to the king there are two possible cases
    else if (board->kingAttackers){

        // If the king is attacked by a pawn or a knight, only moving the king
        // or capturing the pawn / knight will be legal. However, here we are
        // only generating quiet moves, thus we must move the king
        if (board->kingAttackers & (board->pieces[PAWN] | board->pieces[KNIGHT])){
            buildKingMoves(moves, size, myKings, empty);
            return;
        }

        // The attacker is a sliding piece, therefore we can either block the attack
        // by moving a piece infront of the attacking path if the slider, or we can
        // again simple move our king (Castling excluded, of course)
        destinations = empty & bitsBetweenMasks(getlsb(myKings), getlsb(board->kingAttackers));
    }

    // We are not being attacked, and therefore will look at any quiet move,
    // which means we can move to any square without a piece, again of course
    // with promotions and enpass excluded, as they are noisy moves
    else
        destinations = empty;

    // Compute bitboards for the pawn advances
    pawnForwardOne = pawnAdvance(myPawns, occupied, board->turn) & ~PROMOTION_RANKS;
    pawnForwardTwo = pawnAdvance(pawnForwardOne & rank3Rel, occupied, board->turn);

    // Generate all of the pawn advances
    buildPawnMoves(moves, size, pawnForwardOne & destinations, forwardShift);
    buildPawnMoves(moves, size, pawnForwardTwo & destinations, forwardShift * 2);

    // Generate all moves for all non pawns aside from Castles
    buildKnightMoves(moves, size, myKnights, destinations);
    buildBishopAndQueenMoves(moves, size, myBishops, occupied, destinations);
    buildRookAndQueenMoves(moves, size, myRooks, occupied, destinations);
    buildKingMoves(moves, size, myKings, empty);

    // Generate all the castling moves
    if (board->turn == WHITE && !board->kingAttackers){

        if (  ((occupied & WHITE_OO_MAP) == 0)
            && (board->castleRights & WHITE_OO_RIGHTS)
            && !squareIsAttacked(board, WHITE, 5))
            moves[(*size)++] = MoveMake(4, 6, CASTLE_MOVE);

        if (  ((occupied & WHITE_OOO_MAP) == 0)
            && (board->castleRights & WHITE_OOO_RIGHTS)
            && !squareIsAttacked(board, WHITE, 3))
            moves[(*size)++] = MoveMake(4, 2, CASTLE_MOVE);
    }

    else if (board->turn == BLACK && !board->kingAttackers) {

        if (  ((occupied & BLACK_OO_MAP) == 0)
            && (board->castleRights & BLACK_OO_RIGHTS)
            && !squareIsAttacked(board, BLACK, 61))
            moves[(*size)++] = MoveMake(60, 62, CASTLE_MOVE);

        if (  ((occupied & BLACK_OOO_MAP) == 0)
            && (board->castleRights & BLACK_OOO_RIGHTS)
            && !squareIsAttacked(board, BLACK, 59))
            moves[(*size)++] = MoveMake(60, 58, CASTLE_MOVE);
    }
}

int isNotInCheck(Board* board, int colour){
    int kingsq = getlsb(board->colours[colour] & board->pieces[KING]);
    assert(board->squares[kingsq] == WHITE_KING + colour);
    return !squareIsAttacked(board, colour, kingsq);
}

int squareIsAttacked(Board* board, int colour, int sq){

    uint64_t friendly = board->colours[ colour];
    uint64_t enemy    = board->colours[!colour];
    uint64_t occupied = friendly | enemy;

    uint64_t enemyPawns   = enemy &  board->pieces[PAWN  ];
    uint64_t enemyKnights = enemy &  board->pieces[KNIGHT];
    uint64_t enemyBishops = enemy & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t enemyRooks   = enemy & (board->pieces[ROOK  ] | board->pieces[QUEEN]);
    uint64_t enemyKings   = enemy &  board->pieces[KING  ];

    return (pawnAttacks(colour, sq) & enemyPawns)
        || (knightAttacks(sq) & enemyKnights)
        || (enemyBishops && (bishopAttacks(sq, occupied) & enemyBishops))
        || (enemyRooks && (rookAttacks(sq, occupied) & enemyRooks))
        || (kingAttacks(sq) & enemyKings);
}

uint64_t attackersToSquare(Board* board, int colour, int sq){

    uint64_t friendly = board->colours[ colour];
    uint64_t enemy    = board->colours[!colour];
    uint64_t occupied = friendly | enemy;

    return   (pawnAttacks(colour, sq) & enemy & board->pieces[PAWN])
           | (knightAttacks(sq) & enemy & board->pieces[KNIGHT])
           | (bishopAttacks(sq, occupied) & enemy & (board->pieces[BISHOP] | board->pieces[QUEEN]))
           | (rookAttacks(sq, occupied) & enemy & (board->pieces[ROOK] | board->pieces[QUEEN]))
           | (kingAttacks(sq) & enemy & board->pieces[KING]);
}

uint64_t allAttackersToSquare(Board* board, uint64_t occupied, int sq){

    return   (pawnAttacks(BLACK, sq) & board->colours[WHITE] & board->pieces[PAWN])
           | (pawnAttacks(WHITE, sq) & board->colours[BLACK] & board->pieces[PAWN])
           | (knightAttacks(sq) & board->pieces[KNIGHT])
           | (bishopAttacks(sq, occupied) & (board->pieces[BISHOP] | board->pieces[QUEEN]))
           | (rookAttacks(sq, occupied) & (board->pieces[ROOK] | board->pieces[QUEEN]))
           | (kingAttacks(sq) & board->pieces[KING]);
}

uint64_t attackersToKingSquare(Board* board){
    int kingsq = getlsb(board->colours[board->turn] & board->pieces[KING]);
    return attackersToSquare(board, board->turn, kingsq);
}
