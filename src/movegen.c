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

#include "bitboards.h"
#include "bitutils.h"
#include "castle.h"
#include "magics.h"
#include "move.h"
#include "movegen.h"
#include "piece.h"
#include "types.h"

void genAllLegalMoves(Board * board, uint16_t * moves, int * size){
    
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

void genAllMoves(Board * board, uint16_t * moves, int * size){
     
    uint64_t pawnForwardOne, pawnForwardTwo, pawnLeft, pawnRight;
    uint64_t pawnPromoForward, pawnPromoLeft, pawnPromoRight;
    
    int bit, lsb, forwardShift, leftShift, rightShift;
    int epSquare = board->epSquare, castleKing, castleQueen;
    
    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy = board->colours[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    uint64_t notFriendly = ~friendly;
    uint64_t attackable;
    
    uint64_t myPawns   = friendly &  board->pieces[PAWN];
    uint64_t myKnights = friendly &  board->pieces[KNIGHT];
    uint64_t myBishops = friendly & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t myRooks   = friendly & (board->pieces[ROOK]   | board->pieces[QUEEN]);
    uint64_t myKings   = friendly &  board->pieces[KING];
    
    // Define pawn bitboards and find enpass moves
    if (board->turn == WHITE){
        forwardShift = -8;
        leftShift = -7;
        rightShift = -9;
        
        pawnForwardOne = (myPawns << 8) & empty;
        pawnForwardTwo = ((pawnForwardOne & RANK_3) << 8) & empty;
        pawnLeft = ((myPawns << 7) & (~FILE_H)) & enemy;
        pawnRight = ((myPawns << 9) & (~FILE_A)) & enemy;
        
        pawnPromoForward = pawnForwardOne & RANK_8;
        pawnPromoLeft = pawnLeft & RANK_8;
        pawnPromoRight = pawnRight & RANK_8;
        
        pawnForwardOne &= ~RANK_8;
        pawnLeft &= ~RANK_8;
        pawnRight &= ~RANK_8;
        
        if(epSquare != -1){
            if (board->squares[epSquare - 7] == WHITE_PAWN && epSquare != 47)
                moves[(*size)++] = MoveMake(epSquare-7, epSquare, ENPASS_MOVE);
            
            if (board->squares[epSquare - 9] == WHITE_PAWN && epSquare != 40)
                moves[(*size)++] = MoveMake(epSquare-9, epSquare, ENPASS_MOVE);
        }
    } 
    
    else {
        forwardShift = 8;
        leftShift = 7;
        rightShift = 9;
        
        pawnForwardOne = (myPawns >> 8) & empty;
        pawnForwardTwo = ((pawnForwardOne & RANK_6) >> 8) & empty;
        pawnLeft = ((myPawns >> 7) & (~FILE_A)) & enemy;
        pawnRight = ((myPawns >> 9) & (~FILE_H)) & enemy;
        
        pawnPromoForward = pawnForwardOne & RANK_1;
        pawnPromoLeft = pawnLeft & RANK_1;
        pawnPromoRight = pawnRight & RANK_1;
        
        pawnForwardOne &= ~RANK_1;
        pawnLeft &= ~RANK_1;
        pawnRight &= ~RANK_1;
        
        if(epSquare != -1){
            if (board->squares[epSquare + 7] == BLACK_PAWN && epSquare != 16)
                moves[(*size)++] = MoveMake(epSquare+7, epSquare, ENPASS_MOVE);
            
            if (board->squares[epSquare + 9] == BLACK_PAWN && epSquare != 23)
                moves[(*size)++] = MoveMake(epSquare+9, epSquare, ENPASS_MOVE);
        }
    }
    
    // Generate all Pawn moves aside from Promotions and Enpass
    buildPawnMoves(moves, size, pawnForwardOne, forwardShift);
    buildPawnMoves(moves, size, pawnForwardTwo, (2*forwardShift));
    buildPawnMoves(moves, size, pawnLeft, leftShift);
    buildPawnMoves(moves, size, pawnRight, rightShift);
    
    // Generate all Pawn promotion moves
    buildPawnPromotions(moves, size, pawnPromoForward, forwardShift);
    buildPawnPromotions(moves, size, pawnPromoLeft, leftShift);
    buildPawnPromotions(moves, size, pawnPromoRight, rightShift);
    
    // Generate all moves for all non pawns aside from Castles
    buildKnightMoves(moves, size, myKnights, notFriendly);
    buildBishopAndQueenMoves(moves, size, myBishops, notEmpty, notFriendly);
    buildRookAndQueenMoves(moves, size, myRooks, notEmpty, notFriendly);
    buildKingMoves(moves, size, myKings, notFriendly);
    
    // Generate all the castling moves
    if (board->turn == WHITE){
        castleKing  =   ((notEmpty & WHITE_CASTLE_KING_SIDE_MAP) == 0)
                     && (board->castleRights & WHITE_KING_RIGHTS)
                     && !squareIsAttacked(board, WHITE, 5);
        castleQueen =   ((notEmpty & WHITE_CASTLE_QUEEN_SIDE_MAP) == 0)
                     && (board->castleRights & WHITE_QUEEN_RIGHTS)
                     && !squareIsAttacked(board, WHITE, 3);
                     
        if ((castleKing || castleQueen) && isNotInCheck(board, board->turn)){
            if (castleKing)  moves[(*size)++] = MoveMake(4, 6, CASTLE_MOVE);
            if (castleQueen) moves[(*size)++] = MoveMake(4, 2, CASTLE_MOVE);
        }
    }
    
    else {
        castleKing  =   ((notEmpty & BLACK_CASTLE_KING_SIDE_MAP) == 0)
                     && (board->castleRights & BLACK_KING_RIGHTS)
                     && !squareIsAttacked(board, BLACK, 61);
        castleQueen =   ((notEmpty & BLACK_CASTLE_QUEEN_SIDE_MAP) == 0)
                     && (board->castleRights & BLACK_QUEEN_RIGHTS)
                     && !squareIsAttacked(board, BLACK, 59);
                     
        if ((castleKing || castleQueen) && isNotInCheck(board, board->turn)){
            if (castleKing)  moves[(*size)++] = MoveMake(60, 62, CASTLE_MOVE);
            if (castleQueen) moves[(*size)++] = MoveMake(60, 58, CASTLE_MOVE);
        }
    }
}

void genAllNoisyMoves(Board * board, uint16_t * moves, int * size){
    
    uint64_t pawnLeft;
    uint64_t pawnRight;
    
    uint64_t pawnPromoForward;
    uint64_t pawnPromoLeft;
    uint64_t pawnPromoRight;
    
    int bit, lsb;
    int forwardShift, leftShift, rightShift;
    int epSquare = board->epSquare;
    
    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy = board->colours[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    uint64_t attackable;
    
    uint64_t myPawns   = friendly &  board->pieces[PAWN];
    uint64_t myKnights = friendly &  board->pieces[KNIGHT];
    uint64_t myBishops = friendly & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t myRooks   = friendly & (board->pieces[ROOK]   | board->pieces[QUEEN]);
    uint64_t myKings   = friendly &  board->pieces[KING];
    
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
    buildPawnMoves(moves, size, pawnLeft, leftShift);
    buildPawnMoves(moves, size, pawnRight, rightShift);
    
    // Generate all pawn promotions
    buildPawnPromotions(moves, size, pawnPromoForward, forwardShift);
    buildPawnPromotions(moves, size, pawnPromoLeft, leftShift);
    buildPawnPromotions(moves, size, pawnPromoRight, rightShift);
    
    // Generate attacks for all non pawn pieces
    buildKnightMoves(moves, size, myKnights, enemy);
    buildBishopAndQueenMoves(moves, size, myBishops, notEmpty, enemy);
    buildRookAndQueenMoves(moves, size, myRooks, notEmpty, enemy);
    buildKingMoves(moves, size, myKings, enemy);
}

void genAllQuietMoves(Board * board, uint16_t * moves, int * size){
     
    int bit, lsb, castleKing, castleQueen;
    
    uint64_t pawnForwardOne;
    uint64_t pawnForwardTwo;
    
    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy = board->colours[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    uint64_t attackable;
    
    uint64_t myPawns   = friendly &  board->pieces[PAWN];
    uint64_t myKnights = friendly &  board->pieces[KNIGHT];
    uint64_t myBishops = friendly & (board->pieces[BISHOP] | board->pieces[QUEEN]);
    uint64_t myRooks   = friendly & (board->pieces[ROOK]   | board->pieces[QUEEN]);
    uint64_t myKings   = friendly &  board->pieces[KING];
    
    // Generate the pawn advances
    if (board->turn == WHITE){
        pawnForwardOne = (myPawns << 8) & empty & ~RANK_8;
        pawnForwardTwo = ((pawnForwardOne & RANK_3) << 8) & empty;
        buildPawnMoves(moves, size, pawnForwardOne, -8);
        buildPawnMoves(moves, size, pawnForwardTwo, -16);
    } 
    
    else {
        pawnForwardOne = (myPawns >> 8) & empty & ~RANK_1;
        pawnForwardTwo = ((pawnForwardOne & RANK_6) >> 8) & empty;
        buildPawnMoves(moves, size, pawnForwardOne, 8);
        buildPawnMoves(moves, size, pawnForwardTwo, 16);
    }
    
    // Generate all moves for all non pawns aside from Castles
    buildKnightMoves(moves, size, myKnights, empty);
    buildBishopAndQueenMoves(moves, size, myBishops, notEmpty, empty);
    buildRookAndQueenMoves(moves, size, myRooks, notEmpty, empty);
    buildKingMoves(moves, size, myKings, empty);
    
    // Generate all the castling moves
    if (board->turn == WHITE){
        castleKing  =   ((notEmpty & WHITE_CASTLE_KING_SIDE_MAP) == 0)
                     && (board->castleRights & WHITE_KING_RIGHTS)
                     && !squareIsAttacked(board, WHITE, 5);
        castleQueen =   ((notEmpty & WHITE_CASTLE_QUEEN_SIDE_MAP) == 0)
                     && (board->castleRights & WHITE_QUEEN_RIGHTS)
                     && !squareIsAttacked(board, WHITE, 3);
                     
        if ((castleKing || castleQueen) && isNotInCheck(board, board->turn)){
            if (castleKing)  moves[(*size)++] = MoveMake(4, 6, CASTLE_MOVE);
            if (castleQueen) moves[(*size)++] = MoveMake(4, 2, CASTLE_MOVE);
        }
    }
    
    else {
        castleKing  =   ((notEmpty & BLACK_CASTLE_KING_SIDE_MAP) == 0)
                     && (board->castleRights & BLACK_KING_RIGHTS)
                     && !squareIsAttacked(board, BLACK, 61);
        castleQueen =   ((notEmpty & BLACK_CASTLE_QUEEN_SIDE_MAP) == 0)
                     && (board->castleRights & BLACK_QUEEN_RIGHTS)
                     && !squareIsAttacked(board, BLACK, 59);
                     
        if ((castleKing || castleQueen) && isNotInCheck(board, board->turn)){
            if (castleKing)  moves[(*size)++] = MoveMake(60, 62, CASTLE_MOVE);
            if (castleQueen) moves[(*size)++] = MoveMake(60, 58, CASTLE_MOVE);
        }
    }
}

int isNotInCheck(Board * board, int turn){
    int kingsq = getLSB(board->colours[turn] & board->pieces[KING]);
    assert(board->squares[kingsq] == WHITE_KING + turn);
    return !squareIsAttacked(board, turn, kingsq);
}

int squareIsAttacked(Board * board, int turn, int sq){
    
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
    if (enemyKnights && KnightAttacks(sq, enemyKnights)) return 1;
    
    // Bishops and Queens
    if (enemyBishops && BishopAttacks(sq, notEmpty, enemyBishops)) return 1;
    
    // Rooks and Queens
    if (enemyRooks && RookAttacks(sq, notEmpty, enemyRooks)) return 1;
    
    // King
    if (KingAttacks(sq, enemyKings)) return 1;
    
    return 0;
}