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

uint64_t pawnAttacks(int sq, uint64_t targets, int colour);
uint64_t knightAttacks(int sq, uint64_t targets);
uint64_t bishopAttacks(int sq, uint64_t occupied, uint64_t targets);
uint64_t rookAttacks(int sq, uint64_t occupied, uint64_t targets);
uint64_t queenAttacks(int sq, uint64_t occupiedDiagonol, uint64_t occupiedStraight, uint64_t targets);
uint64_t kingAttacks(int sq, uint64_t tagets);

void genAllLegalMoves(Board* board, uint16_t* moves, int* size);
void genAllMoves(Board* board, uint16_t* moves, int* size);
void genAllNoisyMoves(Board* board, uint16_t* moves, int* size);
void genAllQuietMoves(Board* board, uint16_t* moves, int* size);

int isNotInCheck(Board* board, int turn);
int squareIsAttacked(Board* board, int turn, int sq);

uint64_t attackersToSquare(Board* board, int colour, int sq);
uint64_t attackersToKingSquare(Board* board);

#endif
