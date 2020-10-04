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


typedef uint64_t (*JumperFunc)(int);
typedef uint64_t (*SliderFunc)(int, uint64_t);

uint16_t * buildEnpassMoves(uint16_t *moves, uint64_t attacks, int epsq) {

    while (attacks)
        *(moves++) = MoveMake(poplsb(&attacks), epsq, ENPASS_MOVE);

    return moves;
}

uint16_t * buildPawnMoves(uint16_t *moves, uint64_t attacks, int delta) {

    while (attacks) {
        int sq = poplsb(&attacks);
        *(moves++) = MoveMake(sq + delta, sq, NORMAL_MOVE);
    }

    return moves;
}

uint16_t * buildPawnPromotions(uint16_t *moves, uint64_t attacks, int delta) {

    while (attacks) {
        int sq = poplsb(&attacks);
        *(moves++) = MoveMake(sq + delta, sq,  QUEEN_PROMO_MOVE);
        *(moves++) = MoveMake(sq + delta, sq,   ROOK_PROMO_MOVE);
        *(moves++) = MoveMake(sq + delta, sq, BISHOP_PROMO_MOVE);
        *(moves++) = MoveMake(sq + delta, sq, KNIGHT_PROMO_MOVE);
    }

    return moves;
}

uint16_t * buildNormalMoves(uint16_t *moves, uint64_t attacks, int sq) {

    while (attacks)
        *(moves++) = MoveMake(sq, poplsb(&attacks), NORMAL_MOVE);

    return moves;
}

uint16_t * buildJumperMoves(JumperFunc F, uint16_t *moves, uint64_t pieces, uint64_t targets) {

    while (pieces) {
        int sq = poplsb(&pieces);
        moves = buildNormalMoves(moves, F(sq) & targets, sq);
    }

    return moves;
}

uint16_t * buildSliderMoves(SliderFunc F, uint16_t *moves, uint64_t pieces, uint64_t targets, uint64_t occupied) {

    while (pieces) {
        int sq = poplsb(&pieces);
        moves = buildNormalMoves(moves, F(sq, occupied) & targets, sq);
    }

    return moves;
}


int genAllLegalMoves(Board *board, uint16_t *moves) {

    Undo undo[1];
    int size = 0, pseudo = 0;
    uint16_t pseudoMoves[MAX_MOVES];

    // Call genAllNoisyMoves() & genAllNoisyMoves()
    pseudo  = genAllNoisyMoves(board, pseudoMoves);
    pseudo += genAllQuietMoves(board, pseudoMoves + pseudo);

    // Check each move for legality before copying
    for (int i = 0; i < pseudo; i++) {
        applyMove(board, pseudoMoves[i], undo);
        if (moveWasLegal(board)) moves[size++] = pseudoMoves[i];
        revertMove(board, pseudoMoves[i], undo);
    }

    return size;
}

int genAllNoisyMoves(Board *board, uint16_t *moves) {

    const uint16_t *start = moves;

    const int Left    = board->turn == WHITE ? -7 : 7;
    const int Right   = board->turn == WHITE ? -9 : 9;
    const int Forward = board->turn == WHITE ? -8 : 8;

    uint64_t destinations, pawnEnpass, pawnLeft, pawnRight;
    uint64_t pawnPromoForward, pawnPromoLeft, pawnPromoRight;

    uint64_t us       = board->colours[board->turn];
    uint64_t them     = board->colours[!board->turn];
    uint64_t occupied = us | them;

    uint64_t pawns   = us & (board->pieces[PAWN  ]);
    uint64_t knights = us & (board->pieces[KNIGHT]);
    uint64_t bishops = us & (board->pieces[BISHOP]);
    uint64_t rooks   = us & (board->pieces[ROOK  ]);
    uint64_t kings   = us & (board->pieces[KING  ]);

    // Merge together duplicate piece ideas
    bishops |= us & board->pieces[QUEEN];
    rooks   |= us & board->pieces[QUEEN];

    // Double checks can only be evaded by moving the King
    if (several(board->kingAttackers))
        return buildJumperMoves(&kingAttacks, moves, kings, them) - start;

    // When checked, we may only uncheck by capturing the checker
    destinations = board->kingAttackers ? board->kingAttackers : them;

    // Compute bitboards for each type of Pawn movement
    pawnEnpass       = pawnEnpassCaptures(pawns, board->epSquare, board->turn);
    pawnLeft         = pawnLeftAttacks(pawns, them, board->turn);
    pawnRight        = pawnRightAttacks(pawns, them, board->turn);
    pawnPromoForward = pawnAdvance(pawns, occupied, board->turn) & PROMOTION_RANKS;
    pawnPromoLeft    = pawnLeft & PROMOTION_RANKS; pawnLeft &= ~PROMOTION_RANKS;
    pawnPromoRight   = pawnRight & PROMOTION_RANKS; pawnRight &= ~PROMOTION_RANKS;

    // Generate moves for all the Pawns, so long as they are noisy
    moves = buildEnpassMoves(moves, pawnEnpass, board->epSquare);
    moves = buildPawnMoves(moves, pawnLeft & destinations, Left);
    moves = buildPawnMoves(moves, pawnRight & destinations, Right);
    moves = buildPawnPromotions(moves, pawnPromoForward, Forward);
    moves = buildPawnPromotions(moves, pawnPromoLeft, Left);
    moves = buildPawnPromotions(moves, pawnPromoRight, Right);

    // Generate moves for the remainder of the pieces, so long as they are noisy
    moves = buildJumperMoves(&knightAttacks, moves, knights, destinations);
    moves = buildSliderMoves(&bishopAttacks, moves, bishops, destinations, occupied);
    moves = buildSliderMoves(&rookAttacks, moves, rooks, destinations, occupied);
    moves = buildJumperMoves(&kingAttacks, moves, kings, them);

    return moves - start;
}

int genAllQuietMoves(Board *board, uint16_t *moves) {

    const uint16_t *start = moves;

    const int Forward = board->turn == WHITE ? -8 : 8;
    const uint64_t Rank3Relative = board->turn == WHITE ? RANK_3 : RANK_6;

    int rook, king, rookTo, kingTo, attacked;
    uint64_t destinations, pawnForwardOne, pawnForwardTwo, mask;

    uint64_t us       = board->colours[board->turn];
    uint64_t occupied = us | board->colours[!board->turn];
    uint64_t castles  = us & board->castleRooks;

    uint64_t pawns   = us & (board->pieces[PAWN  ]);
    uint64_t knights = us & (board->pieces[KNIGHT]);
    uint64_t bishops = us & (board->pieces[BISHOP]);
    uint64_t rooks   = us & (board->pieces[ROOK  ]);
    uint64_t kings   = us & (board->pieces[KING  ]);

    // Merge together duplicate piece ideas
    bishops |= us & board->pieces[QUEEN];
    rooks   |= us & board->pieces[QUEEN];

    // Double checks can only be evaded by moving the King
    if (several(board->kingAttackers))
        return buildJumperMoves(&kingAttacks, moves, kings, ~occupied) - start;

    // When checked, we must block the checker with non-King pieces
    destinations = !board->kingAttackers ? ~occupied
                 : bitsBetweenMasks(getlsb(kings), getlsb(board->kingAttackers));

    // Compute bitboards for each type of Pawn movement
    pawnForwardOne = pawnAdvance(pawns, occupied, board->turn) & ~PROMOTION_RANKS;
    pawnForwardTwo = pawnAdvance(pawnForwardOne & Rank3Relative, occupied, board->turn);

    // Generate moves for all the pawns, so long as they are quiet
    moves = buildPawnMoves(moves, pawnForwardOne & destinations, Forward);
    moves = buildPawnMoves(moves, pawnForwardTwo & destinations, Forward * 2);

    // Generate moves for the remainder of the pieces, so long as they are quiet
    moves = buildJumperMoves(&knightAttacks, moves, knights, destinations);
    moves = buildSliderMoves(&bishopAttacks, moves, bishops, destinations, occupied);
    moves = buildSliderMoves(&rookAttacks, moves, rooks, destinations, occupied);
    moves = buildJumperMoves(&kingAttacks, moves, kings, ~occupied);

    // Attempt to generate a castle move for each rook
    while (castles && !board->kingAttackers) {

        // Figure out which pieces are moving to which squares
        rook = poplsb(&castles), king = getlsb(kings);
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
        *(moves++) = MoveMake(king, rook, CASTLE_MOVE);
    }

    return moves - start;
}
