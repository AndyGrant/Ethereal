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

uint64_t pawnLeftAttacks(uint64_t pawns, uint64_t targets, int colour);
uint64_t pawnRightAttacks(uint64_t pawns, uint64_t targets, int colour);
uint64_t pawnAdvance(uint64_t pawns, uint64_t occupied, int colour);
uint64_t pawnEnpassCaptures(uint64_t pawns, int epsq, int colour);

void genAllLegalMoves(Board* board, uint16_t* moves, int* size);
void genAllMoves(Board* board, uint16_t* moves, int* size);
void genAllNoisyMoves(Board* board, uint16_t* moves, int* size);
void genAllQuietMoves(Board* board, uint16_t* moves, int* size);

int isNotInCheck(Board* board, int colour);
int squareIsAttacked(Board* board, int colour, int sq);

uint64_t attackersToSquare(Board* board, int colour, int sq);
uint64_t allAttackersToSquare(Board* board, uint64_t occupied, int sq);
uint64_t attackersToKingSquare(Board* board);

#endif
