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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "types.h"

enum {
    RANK_1 = 0x00000000000000FFull,
    RANK_2 = 0x000000000000FF00ull,
    RANK_3 = 0x0000000000FF0000ull,
    RANK_4 = 0x00000000FF000000ull,
    RANK_5 = 0x000000FF00000000ull,
    RANK_6 = 0x0000FF0000000000ull,
    RANK_7 = 0x00FF000000000000ull,
    RANK_8 = 0xFF00000000000000ull,

    FILE_A = 0x0101010101010101ull,
    FILE_B = 0x0202020202020202ull,
    FILE_C = 0x0404040404040404ull,
    FILE_D = 0x0808080808080808ull,
    FILE_E = 0x1010101010101010ull,
    FILE_F = 0x2020202020202020ull,
    FILE_G = 0x4040404040404040ull,
    FILE_H = 0x8080808080808080ull,

    WHITE_SQUARES = 0x55AA55AA55AA55AAull,
    BLACK_SQUARES = 0xAA55AA55AA55AA55ull,

    LONG_DIAGONALS = 0x8142241818244281ull,
    CENTER_SQUARES = 0x0000001818000000ull,
    CENTER_BIG     = 0x00003C3C3C3C0000ull,

    LEFT_FLANK  = FILE_A | FILE_B | FILE_C | FILE_D,
    RIGHT_FLANK = FILE_E | FILE_F | FILE_G | FILE_H,

    PROMOTION_RANKS = RANK_1 | RANK_8
};

extern const uint64_t Files[FILE_NB];
extern const uint64_t Ranks[RANK_NB];

int fileOf(int sq);
int mirrorFile(int file);
int rankOf(int sq);
int relativeRankOf(int colour, int sq);
int square(int rank, int file);
int relativeSquare(int colour, int sq);
int relativeSquare32(int colour, int sq);
uint64_t squaresOfMatchingColour(int sq);

int frontmost(int colour, uint64_t bb);
int backmost(int colour, uint64_t bb);

int popcount(uint64_t bb);
int getlsb(uint64_t bb);
int getmsb(uint64_t bb);
int poplsb(uint64_t *bb);
int popmsb(uint64_t *bb);
bool several(uint64_t bb);
bool onlyOne(uint64_t bb);

void setBit(uint64_t *bb, int i);
void clearBit(uint64_t *bb, int i);
bool testBit(uint64_t bb, int i);

void printBitboard(uint64_t bb);
