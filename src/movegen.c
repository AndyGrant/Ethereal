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

#include "board.h"
#include "bitboards.h"
#include "bitutils.h"
#include "castle.h"
#include "magics.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "piece.h"
#include "types.h"

/* For Generating Attack BitBoards */

uint64_t pawnAttacks(int sq, uint64_t targets, int colour){
    return targets & (colour == WHITE ? ((((1ull << sq) << 7) & ~FILE_H) | (((1ull << sq) << 9) & ~FILE_A))
                                      : ((((1ull << sq) >> 7) & ~FILE_A) | (((1ull << sq) >> 9) & ~FILE_H)));
}

uint64_t knightAttacks(int sq, uint64_t targets){
    return KnightMap[sq] & targets;
}

uint64_t bishopAttacks(int sq, uint64_t occupied, uint64_t targets){
    uint64_t mask = occupied & OccupancyMaskBishop[sq];
    int index     = (mask * MagicNumberBishop[sq]) >> MagicShiftsBishop[sq];
    return MoveDatabaseBishop[MagicBishopIndexes[sq] + index] & targets;
}

uint64_t rookAttacks(int sq, uint64_t occupied, uint64_t targets){
    uint64_t mask = occupied & OccupancyMaskRook[sq];
    int index     = (mask * MagicNumberRook[sq]) >> MagicShiftsRook[sq];
    return MoveDatabaseRook[MagicRookIndexes[sq] + index] & targets;
}

uint64_t queenAttacks(int sq, uint64_t occupiedDiagonol, uint64_t occupiedStraight, uint64_t targets){
    return  bishopAttacks(sq, occupiedDiagonol, targets)
            | rookAttacks(sq, occupiedStraight, targets);
}

uint64_t kingAttacks(int sq, uint64_t targets){
    return KingMap[sq] & targets;
}


/* For Building Actual Move Lists For Each Piece Type */

void buildPawnMoves(uint16_t* moves, int* size, uint64_t attacks, int delta){
    while (attacks){
        int sq = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq + delta, sq, NORMAL_MOVE);
    }
}

void buildPawnPromotions(uint16_t* moves, int* size, uint64_t attacks, int delta){
    while (attacks){
        int sq = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq + delta, sq,  QUEEN_PROMO_MOVE);
        moves[(*size)++] = MoveMake(sq + delta, sq,   ROOK_PROMO_MOVE);
        moves[(*size)++] = MoveMake(sq + delta, sq, BISHOP_PROMO_MOVE);
        moves[(*size)++] = MoveMake(sq + delta, sq, KNIGHT_PROMO_MOVE);
    }
}

void buildNonPawnMoves(uint16_t* moves, int* size, uint64_t attacks, int sq){
    while (attacks){
        int tg = poplsb(&attacks);
        moves[(*size)++] = MoveMake(sq, tg, NORMAL_MOVE);
    }
}

void buildKnightMoves(uint16_t* moves, int* size, uint64_t pieces, uint64_t targets){
    while (pieces){
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, knightAttacks(sq, targets), sq);
    }
}

void buildBishopAndQueenMoves(uint16_t* moves, int* size, uint64_t pieces, uint64_t occupied, uint64_t targets){
    while (pieces){
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, bishopAttacks(sq, occupied, targets), sq);
    }
}

void buildRookAndQueenMoves(uint16_t* moves, int* size, uint64_t pieces, uint64_t occupied, uint64_t targets){
    while (pieces){
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, rookAttacks(sq, occupied, targets), sq);
    }
}

void buildKingMoves(uint16_t* moves, int* size, uint64_t pieces, uint64_t targets){
    while (pieces){
        int sq = poplsb(&pieces);
        buildNonPawnMoves(moves, size, kingAttacks(sq, targets), sq);
    }
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
    
    uint64_t destinations;
    
    uint64_t pawnLeft;
    uint64_t pawnRight;
    
    uint64_t pawnPromoForward;
    uint64_t pawnPromoLeft;
    uint64_t pawnPromoRight;
    
    int forwardShift, leftShift, rightShift;
    int epSquare = board->epSquare;
    
    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy = board->colours[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    
    uint64_t myPawns   = friendly &  board->pieces[PAWN];
    uint64_t myKnights = friendly &  board->pieces[KNIGHT];
    uint64_t myBishops = friendly & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t myRooks   = friendly & (board->pieces[ROOK]   | board->pieces[QUEEN]);
    uint64_t myKings   = friendly &  board->pieces[KING];
    
    // If there are two threats to the king, the only moves
    // which could be legal are captures made by the king
    if (moreThanOne(board->kingAttackers)){
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
    
    // Generate Pawn BitBoards and Generate Enpass Moves
    if (board->turn == WHITE){
        forwardShift = -8;
        leftShift = -7;
        rightShift = -9;
        
        pawnLeft = ((myPawns << 7) & ~FILE_H) & enemy;
        pawnRight = ((myPawns << 9) & ~FILE_A) & enemy;
        
        pawnPromoForward = (myPawns << 8) & empty & RANK_8;
        pawnPromoLeft = pawnLeft & RANK_8;
        pawnPromoRight = pawnRight & RANK_8;
        
        pawnLeft &= ~RANK_8;
        pawnRight &= ~RANK_8;
        
        if(epSquare != -1){
            if (board->squares[epSquare-7] == WHITE_PAWN && epSquare != 47)
                moves[(*size)++] = MoveMake(epSquare-7, epSquare, ENPASS_MOVE);
            
            if (board->squares[epSquare-9] == WHITE_PAWN && epSquare != 40)
                moves[(*size)++] = MoveMake(epSquare-9, epSquare, ENPASS_MOVE);
        }
    } 
    
    else {
        forwardShift = 8;
        leftShift = 7;
        rightShift = 9;
        
        pawnLeft = ((myPawns >> 7) & ~FILE_A) & enemy;
        pawnRight = ((myPawns >> 9) & ~FILE_H) & enemy;
        
        pawnPromoForward = (myPawns >> 8) & empty & RANK_1;
        pawnPromoLeft = pawnLeft & RANK_1;
        pawnPromoRight = pawnRight & RANK_1;
        
        pawnLeft &= ~RANK_1;
        pawnRight &= ~RANK_1;
        
        if(epSquare != -1){
            if (board->squares[epSquare+7] == BLACK_PAWN && epSquare != 16)
                moves[(*size)++] = MoveMake(epSquare+7, epSquare, ENPASS_MOVE);
            
            if (board->squares[epSquare+9] == BLACK_PAWN && epSquare != 23)
                moves[(*size)++] = MoveMake(epSquare+9, epSquare, ENPASS_MOVE);
        }
    }
    
    // Generate all pawn captures that are not promotions
    buildPawnMoves(moves, size, pawnLeft & destinations, leftShift);
    buildPawnMoves(moves, size, pawnRight & destinations, rightShift);
    
    // Generate all pawn promotions
    buildPawnPromotions(moves, size, pawnPromoForward, forwardShift);
    buildPawnPromotions(moves, size, pawnPromoLeft, leftShift);
    buildPawnPromotions(moves, size, pawnPromoRight, rightShift);
    
    // Generate attacks for all non pawn pieces
    buildKnightMoves(moves, size, myKnights, destinations);
    buildBishopAndQueenMoves(moves, size, myBishops, notEmpty, destinations);
    buildRookAndQueenMoves(moves, size, myRooks, notEmpty, destinations);
    buildKingMoves(moves, size, myKings, enemy);
}

void genAllQuietMoves(Board* board, uint16_t* moves, int* size){
    
    uint64_t destinations;
    
    uint64_t pawnForwardOne;
    uint64_t pawnForwardTwo;
    
    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy = board->colours[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    
    uint64_t myPawns   = friendly &  board->pieces[PAWN];
    uint64_t myKnights = friendly &  board->pieces[KNIGHT];
    uint64_t myBishops = friendly & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t myRooks   = friendly & (board->pieces[ROOK]   | board->pieces[QUEEN]);
    uint64_t myKings   = friendly &  board->pieces[KING];
    
    // If there are two threats to the king, the only moves which
    // could be legal are moves made by the king, except castling
    if (moreThanOne(board->kingAttackers)){
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
        destinations = empty & BitsBetweenMasks[getlsb(myKings)][getlsb(board->kingAttackers)];
    }
    
    // We are not being attacked, and therefore will look at any quiet move,
    // which means we can move to any square without a piece, again of course
    // with promotions and enpass excluded, as they are noisy moves
    else
        destinations = empty;
    
    
    // Generate the pawn advances
    if (board->turn == WHITE){
        pawnForwardOne = (myPawns << 8) & empty & ~RANK_8;
        pawnForwardTwo = ((pawnForwardOne & RANK_3) << 8) & empty;
        buildPawnMoves(moves, size, pawnForwardOne & destinations, -8);
        buildPawnMoves(moves, size, pawnForwardTwo & destinations, -16);
    } 
    
    else {
        pawnForwardOne = (myPawns >> 8) & empty & ~RANK_1;
        pawnForwardTwo = ((pawnForwardOne & RANK_6) >> 8) & empty;
        buildPawnMoves(moves, size, pawnForwardOne & destinations, 8);
        buildPawnMoves(moves, size, pawnForwardTwo & destinations, 16);
    }
    
    // Generate all moves for all non pawns aside from Castles
    buildKnightMoves(moves, size, myKnights, destinations);
    buildBishopAndQueenMoves(moves, size, myBishops, notEmpty, destinations);
    buildRookAndQueenMoves(moves, size, myRooks, notEmpty, destinations);
    buildKingMoves(moves, size, myKings, empty);
    
    // Generate all the castling moves
    if (board->turn == WHITE && !board->kingAttackers){
        
        if (  ((notEmpty & WHITE_CASTLE_KING_SIDE_MAP) == 0)
            && (board->castleRights & WHITE_KING_RIGHTS)
            && !squareIsAttacked(board, WHITE, 5))
            moves[(*size)++] = MoveMake(4, 6, CASTLE_MOVE);
            
        if (  ((notEmpty & WHITE_CASTLE_QUEEN_SIDE_MAP) == 0)
            && (board->castleRights & WHITE_QUEEN_RIGHTS)
            && !squareIsAttacked(board, WHITE, 3))
            moves[(*size)++] = MoveMake(4, 2, CASTLE_MOVE);
    }
    
    else if (board->turn == BLACK && !board->kingAttackers) {
        
        if (  ((notEmpty & BLACK_CASTLE_KING_SIDE_MAP) == 0)
            && (board->castleRights & BLACK_KING_RIGHTS)
            && !squareIsAttacked(board, BLACK, 61))
            moves[(*size)++] = MoveMake(60, 62, CASTLE_MOVE);
                     
        if (  ((notEmpty & BLACK_CASTLE_QUEEN_SIDE_MAP) == 0)
            && (board->castleRights & BLACK_QUEEN_RIGHTS)
            && !squareIsAttacked(board, BLACK, 59))
            moves[(*size)++] = MoveMake(60, 58, CASTLE_MOVE);
    }
}

int isNotInCheck(Board* board, int turn){
    int kingsq = getlsb(board->colours[turn] & board->pieces[KING]);
    assert(board->squares[kingsq] == WHITE_KING + turn);
    return !squareIsAttacked(board, turn, kingsq);
}

int squareIsAttacked(Board* board, int turn, int sq){
    
    uint64_t square;
    
    uint64_t friendly = board->colours[turn];
    uint64_t enemy = board->colours[!turn];
    uint64_t notEmpty = friendly | enemy;
    
    uint64_t enemyPawns   = enemy & board->pieces[PAWN];
    uint64_t enemyKnights = enemy & board->pieces[KNIGHT];
    uint64_t enemyBishops = enemy & board->pieces[BISHOP];
    uint64_t enemyRooks   = enemy & board->pieces[ROOK];
    uint64_t enemyQueens  = enemy & board->pieces[QUEEN];
    uint64_t enemyKings   = enemy & board->pieces[KING];
    
    enemyBishops |= enemyQueens;
    enemyRooks |= enemyQueens;
    square = (1ull << sq);
    
    // Pawns
    if (turn == WHITE){
        if (((square << 7 & ~FILE_H) | (square << 9 & ~FILE_A)) & enemyPawns)
            return 1;
    } else {
        if (((square >> 7 & ~FILE_A) | (square >> 9 & ~FILE_H)) & enemyPawns)
            return 1;
    }
    
    // Knights
    if (enemyKnights && knightAttacks(sq, enemyKnights)) return 1;
    
    // Bishops and Queens
    if (enemyBishops && bishopAttacks(sq, notEmpty, enemyBishops)) return 1;
    
    // Rooks and Queens
    if (enemyRooks && rookAttacks(sq, notEmpty, enemyRooks)) return 1;
    
    // King
    if (kingAttacks(sq, enemyKings)) return 1;
    
    return 0;
}

uint64_t attackersToSquare(Board* board, int colour, int sq){
    
    uint64_t friendly = board->colours[ colour];
    uint64_t enemy    = board->colours[!colour];
    uint64_t occupied = friendly | enemy;
    
    return      pawnAttacks(sq, enemy & board->pieces[PAWN  ], colour)
           |  knightAttacks(sq, enemy & board->pieces[KNIGHT])
           |  bishopAttacks(sq, occupied, enemy & (board->pieces[BISHOP] | board->pieces[QUEEN]))
           |    rookAttacks(sq, occupied, enemy & (board->pieces[ROOK  ] | board->pieces[QUEEN]))
           |    kingAttacks(sq, enemy & board->pieces[KING  ]);
}

uint64_t attackersToKingSquare(Board* board){
    int kingsq = getlsb(board->colours[board->turn] & board->pieces[KING]);
    return attackersToSquare(board, board->turn, kingsq);
}