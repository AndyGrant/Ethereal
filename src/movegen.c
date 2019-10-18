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

#include "attacks.h"
#include "board.h"
#include "bitboards.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "types.h"

static void buildEnpassMoves(uint16_t *moves, int *size, uint64_t attacks, int epsq) {
    while (attacks) {
        int sq = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq, epsq, ENPASS_MOVE);
    }
}

static void buildPawnMoves(uint16_t *moves, int *size, uint64_t attacks, int delta) {
    while (attacks) {
        int sq = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq + delta, sq, NORMAL_MOVE);
    }
}

static void buildPawnPromotions(uint16_t *moves, int *size, uint64_t attacks, int delta) {
    while (attacks) {
        int sq = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq + delta, sq,  QUEEN_PROMO_MOVE);
        moves[(*size)++] = MoveMake(sq + delta, sq,   ROOK_PROMO_MOVE);
        moves[(*size)++] = MoveMake(sq + delta, sq, BISHOP_PROMO_MOVE);
        moves[(*size)++] = MoveMake(sq + delta, sq, KNIGHT_PROMO_MOVE);
    }
}

static void buildNonPawnMoves(uint16_t *moves, int *size, uint64_t attacks, int sq) {
    while (attacks) {
        int to = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq, to, NORMAL_MOVE);
    }
}

static void buildKnightMoves(uint16_t *moves, int *size, uint64_t pieces, uint64_t targets) {
    while (pieces) {
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, knightAttacks(sq) & targets, sq);
    }
}

static void buildBishopMoves(uint16_t *moves, int *size, uint64_t pieces, uint64_t occupied, uint64_t targets) {
    while (pieces) {
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, bishopAttacks(sq, occupied) & targets, sq);
    }
}

static void buildRookMoves(uint16_t *moves, int *size, uint64_t pieces, uint64_t occupied, uint64_t targets) {
    while (pieces) {
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, rookAttacks(sq, occupied) & targets, sq);
    }
}

static void buildKingMoves(uint16_t *moves, int *size, uint64_t pieces, uint64_t targets) {
    int sq = getlsb(pieces);
    buildNonPawnMoves(moves, size, kingAttacks(sq) & targets, sq);
}

void genAllLegalMoves(Board *board, uint16_t *moves, int *size) {

    Undo undo[1];
    int pseudoSize = 0;
    uint16_t pseudoMoves[MAX_MOVES];

    // Call genAllNoisyMoves() & genAllNoisyMoves()
    genAllNoisyMoves(board, pseudoMoves, &pseudoSize);
    genAllQuietMoves(board, pseudoMoves, &pseudoSize);

    // Check each move for legality before copying
    for (int i = 0; i < pseudoSize; i++) {
        applyMove(board, pseudoMoves[i], undo);
        if (moveWasLegal(board)) moves[(*size)++] = pseudoMoves[i];
        revertMove(board, pseudoMoves[i], undo);
    }
}

void genAllNoisyMoves(Board *board, uint16_t *moves, int *size) {

    const int Forward = board->turn == WHITE ? -8 : 8;
    const int Left    = board->turn == WHITE ? -7 : 7;
    const int Right   = board->turn == WHITE ? -9 : 9;

    uint64_t destinations, pawnEnpass, pawnLeft, pawnRight;
    uint64_t pawnPromoForward, pawnPromoLeft, pawnPromoRight;

    uint64_t friendly = board->colours[ board->turn];
    uint64_t enemy    = board->colours[!board->turn];
    uint64_t occupied = friendly | enemy;

    uint64_t myPawns   = friendly &  board->pieces[PAWN  ];
    uint64_t myKnights = friendly &  board->pieces[KNIGHT];
    uint64_t myBishops = friendly & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t myRooks   = friendly & (board->pieces[ROOK  ] | board->pieces[QUEEN]);
    uint64_t myKings   = friendly &  board->pieces[KING  ];

    // Double checks can only be evaded by moving the King. We only
    // look at captures by the king since we are generating noisy moves
    if (several(board->kingAttackers)) {
        buildKingMoves(moves, size, myKings, enemy);
        return;
    }

    // If we are attacked we will only consider the normal moves which would
    // capture the checking piece, since we are only generating noisy moves
    // in this function. Otherwise, we consider all enemy pieces as targets
    destinations = board->kingAttackers ? board->kingAttackers : enemy;

    // Compute bitboards for each type of Pawn movement
    pawnEnpass       = pawnEnpassCaptures(myPawns, board->epSquare, board->turn);
    pawnLeft         = pawnLeftAttacks(myPawns, enemy, board->turn);
    pawnRight        = pawnRightAttacks(myPawns, enemy, board->turn);
    pawnPromoForward = pawnAdvance(myPawns, occupied, board->turn) & PROMOTION_RANKS;
    pawnPromoLeft    = pawnLeft & PROMOTION_RANKS; pawnLeft &= ~PROMOTION_RANKS;
    pawnPromoRight   = pawnRight & PROMOTION_RANKS; pawnRight &= ~PROMOTION_RANKS;

    // Generate all the noisy Pawn moves
    buildEnpassMoves(moves, size, pawnEnpass, board->epSquare);
    buildPawnMoves(moves, size, pawnLeft & destinations, Left);
    buildPawnMoves(moves, size, pawnRight & destinations, Right);
    buildPawnPromotions(moves, size, pawnPromoForward, Forward);
    buildPawnPromotions(moves, size, pawnPromoLeft, Left);
    buildPawnPromotions(moves, size, pawnPromoRight, Right);

    // Generate all the noisy non-Pawn moves
    buildKnightMoves(moves, size, myKnights, destinations);
    buildBishopMoves(moves, size, myBishops, occupied, destinations);
    buildRookMoves(moves, size, myRooks, occupied, destinations);
    buildKingMoves(moves, size, myKings, enemy);
}

void genAllQuietMoves(Board *board, uint16_t *moves, int *size) {

    const int Forward = board->turn == WHITE ? -8 : 8;
    const uint64_t Rank3Relative = board->turn == WHITE ? RANK_3 : RANK_6;

    int rook, king, rookTo, kingTo, attacked;
    uint64_t destinations, pawnForwardOne, pawnForwardTwo, mask;

    uint64_t friendly = board->colours[ board->turn];
    uint64_t enemy    = board->colours[!board->turn];
    uint64_t occupied = friendly | enemy;

    uint64_t myPawns   = friendly &  board->pieces[PAWN  ];
    uint64_t myKnights = friendly &  board->pieces[KNIGHT];
    uint64_t myBishops = friendly & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t myRooks   = friendly & (board->pieces[ROOK  ] | board->pieces[QUEEN]);
    uint64_t myKings   = friendly &  board->pieces[KING  ];
    uint64_t castles   = friendly &  board->castleRooks;

    // Double checks can only be evaded by moving the King. We do not
    // look at captures by the king since we are generating quiet moves
    if (several(board->kingAttackers)) {
        buildKingMoves(moves, size, myKings, ~occupied);
        return;
    }

    // Like above, since checks by Pawns and Knights cannot be blocked,
    // the only quiet evasions are the ones in which we move our King
    if (board->kingAttackers & (board->pieces[PAWN] | board->pieces[KNIGHT])) {
        buildKingMoves(moves, size, myKings, ~occupied);
        return;
    }

    // When there is no checking threat we will look at all moves to empty squares.
    // squares. When there is a checking threat, we may only look at moves which
    // block the sliding path between the piece giving check and our King
    destinations = !board->kingAttackers ? ~occupied
                 : ~occupied & bitsBetweenMasks(getlsb(myKings), getlsb(board->kingAttackers));

    // Compute bitboards for the quiet Pawn moves
    pawnForwardOne = pawnAdvance(myPawns, occupied, board->turn) & ~PROMOTION_RANKS;
    pawnForwardTwo = pawnAdvance(pawnForwardOne & Rank3Relative, occupied, board->turn);

    // Generate all of the quiet Pawn moves
    buildPawnMoves(moves, size, pawnForwardOne & destinations, Forward);
    buildPawnMoves(moves, size, pawnForwardTwo & destinations, Forward * 2);

    // Generate all of the quiet non-Pawn moves, except castles
    buildKnightMoves(moves, size, myKnights, destinations);
    buildBishopMoves(moves, size, myBishops, occupied, destinations);
    buildRookMoves(moves, size, myRooks, occupied, destinations);
    buildKingMoves(moves, size, myKings, ~occupied);

    // Attempt to generate a castle move for each rook
    while (castles && !board->kingAttackers) {

        // Figure out which pieces are moving to which squares
        rook = poplsb(&castles), king = getlsb(myKings);
        rookTo = castleRookTo(king, rook);
        kingTo = castleKingTo(king, rook);
        attacked = 0;

        // Castle is illegal if we would go over a piece
        mask  = bitsBetweenMasks(king, kingTo) | (1ull << kingTo);
        mask |= bitsBetweenMasks(rook, rookTo) | (1ull << rookTo);
        mask &= ~((1ull << king) | (1ull << rook));
        if (occupied & mask) continue;

        // Castle is illegal if we move through a checking threat
        mask = bitsBetweenMasks(king, kingTo);
        while (mask)
            if (squareIsAttacked(board, board->turn, poplsb(&mask)))
                { attacked = 1; break; }
        if (attacked) continue;

        // All conditions have been met. Identify which side we are castling to
        moves[(*size)++] = MoveMake(king, rook, CASTLE_MOVE);
    }
}
