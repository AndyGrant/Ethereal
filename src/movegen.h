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

#ifndef _MOVEGEN_H
#define _MOVEGEN_H

#include <stdint.h>

#include "types.h"
#include "magics.h"

void genAllMoves(Board * board, uint16_t * moves, int * size);
void genAllNonQuiet(Board * board, uint16_t * moves, int * size);
int isNotInCheck(Board * board, int turn);
int squareIsAttacked(Board * board, int turn, int sq);

#define KnightAttacks(sq, tg)       (KnightMap[(sq)] & (tg))

#define BishopAttacks(sq, ne, tg)   (MoveDatabaseBishop[MagicBishopIndexes[(sq)] +              \
                                    ((((ne) & OccupancyMaskBishop[(sq)]) *                      \
                                    MagicNumberBishop[(sq)]) >> MagicShiftsBishop[(sq)])] & (tg))

#define RookAttacks(sq, ne, tg)     (MoveDatabaseRook[MagicRookIndexes[(sq)] +                  \
                                    ((((ne) & OccupancyMaskRook[(sq)]) *                        \
                                    MagicNumberRook[(sq)]) >> MagicShiftsRook[(sq)])] & (tg))
                                    
#define KingAttacks(sq, tg)         (KingMap[(sq)] & (tg))


// Documentation for the move generation macros:
//     arr     = uint16_t array of moves
//     size    = a pointer to an int for the size
//     bb      = uint64_t, either represents pieces, or destinations
//     delta   = int to represent how a pawn will move
//     sq      = int for the square that the piece is moving from
//     ne      = uint64_t of non empty squares on the chess board
//     tg      = uint64_t of valid targets. For the regular gen, this is simply
//               non friendly pieces. For the noisy gen, this is enemy peices

#define buildPawnMoves(arr, size, bb, delta) do {                               \
        while ((bb) != 0ull) {                                                  \
            lsb = getLSB((bb));                                                 \
            (bb) ^= (1ull << lsb);                                              \
            (arr)[(*(size))++] = MoveMake(lsb+(delta), lsb, NORMAL_MOVE);       \
        }} while(0)
                
#define buildPawnPromotions(arr, size, bb, delta) do {                          \
        while ((bb) != 0ull) {                                                  \
            lsb = getLSB((bb));                                                 \
            (bb) ^= (1ull << lsb);                                              \
            (arr)[(*(size))++] = MoveMake(lsb+(delta), lsb, QUEEN_PROMO_MOVE);  \
            (arr)[(*(size))++] = MoveMake(lsb+(delta), lsb, ROOK_PROMO_MOVE);   \
            (arr)[(*(size))++] = MoveMake(lsb+(delta), lsb, BISHOP_PROMO_MOVE); \
            (arr)[(*(size))++] = MoveMake(lsb+(delta), lsb, KNIGHT_PROMO_MOVE); \
        }} while(0)
            
#define buildNonPawnMoves(arr, size, bb, sq) do {                               \
        while ((bb) != 0ull) {                                                  \
            lsb = getLSB((bb));                                                 \
            (bb) ^= (1ull << lsb);                                              \
            (arr)[(*(size))++] = MoveMake(sq, lsb, NORMAL_MOVE);                \
        }} while(0)
            
#define buildKnightMoves(arr, size, bb, tg) do {                                \
        while ((bb) != 0ull) {                                                  \
            bit = getLSB((bb));                                                 \
            (bb) ^= (1ull << bit);                                              \
            attackable = KnightAttacks(bit, tg);                                \
            buildNonPawnMoves(arr, size, attackable, bit);                      \
        }} while(0)
            
#define buildBishopAndQueenMoves(arr, size, bb, ne, tg) do {                    \
        while ((bb) != 0ull) {                                                  \
            bit = getLSB((bb));                                                 \
            (bb) ^= (1ull << bit);                                              \
            attackable = BishopAttacks(bit, ne, tg);                            \
            buildNonPawnMoves(arr, size, attackable, bit);                      \
        }} while(0)
            
#define buildRookAndQueenMoves(arr, size, bb, ne, tg) do {                      \
        while ((bb) != 0ull) {                                                  \
            bit = getLSB((bb));                                                 \
            (bb) ^= (1ull << bit);                                              \
            attackable = RookAttacks(bit, ne, tg);                              \
            buildNonPawnMoves(arr, size, attackable, bit);                      \
        }} while(0)
            
#define buildKingMoves(arr, size, bb, tg) do {                                  \
        bit = getLSB((bb));                                                     \
        attackable = KingAttacks(bit, tg);                                      \
        buildNonPawnMoves(arr, size, attackable, bit);                          \
        } while(0)

#endif