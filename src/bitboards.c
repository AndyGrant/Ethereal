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

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bitboards.h"
#include "types.h"

const uint64_t Files[FILE_NB] = {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
const uint64_t Ranks[RANK_NB] = {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};

int fileOf(int sq) {
    assert(0 <= sq && sq < SQUARE_NB);
    return sq % FILE_NB;
}

int mirrorFile(int file) {
    static const int Mirror[] = {0,1,2,3,3,2,1,0};
    assert(0 <= file && file < FILE_NB);
    return Mirror[file];
}

int rankOf(int sq) {
    assert(0 <= sq && sq < SQUARE_NB);
    return sq / FILE_NB;
}

int relativeRankOf(int colour, int sq) {
    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= sq && sq < SQUARE_NB);
    return colour == WHITE ? rankOf(sq) : 7 - rankOf(sq);
}

int square(int rank, int file) {
    assert(0 <= rank && rank < RANK_NB);
    assert(0 <= file && file < FILE_NB);
    return rank * FILE_NB + file;
}

int relativeSquare(int colour, int sq) {
    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= sq && sq < SQUARE_NB);
    return square(relativeRankOf(colour, sq), fileOf(sq));
}

int relativeSquare32(int colour, int sq) {
    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= sq && sq < SQUARE_NB);
    return 4 * relativeRankOf(colour, sq) + mirrorFile(fileOf(sq));
}

uint64_t squaresOfMatchingColour(int sq) {
    assert(0 <= sq && sq < SQUARE_NB);
    return testBit(WHITE_SQUARES, sq) ? WHITE_SQUARES : BLACK_SQUARES;
}

int frontmost(int colour, uint64_t bb) {
    assert(0 <= colour && colour < COLOUR_NB);
    return colour == WHITE ? getmsb(bb) : getlsb(bb);
}

int backmost(int colour, uint64_t bb) {
    assert(0 <= colour && colour < COLOUR_NB);
    return colour == WHITE ? getlsb(bb) : getmsb(bb);
}

int popcount(uint64_t bb) {
    return __builtin_popcountll(bb);
}

int getlsb(uint64_t bb) {
    assert(bb);  // lsb(0) is undefined
    return __builtin_ctzll(bb);
}

int getmsb(uint64_t bb) {
    assert(bb);  // msb(0) is undefined
    return __builtin_clzll(bb) ^ 63;
}

int poplsb(uint64_t *bb) {
    int lsb = getlsb(*bb);
    *bb &= *bb - 1;
    return lsb;
}

int popmsb(uint64_t *bb) {
    int msb = getmsb(*bb);
    *bb ^= 1ull << msb;
    return msb;
}

bool several(uint64_t bb) {
    return bb & (bb - 1);
}

bool onlyOne(uint64_t bb) {
    return bb && !several(bb);
}

void setBit(uint64_t *bb, int i) {
    assert(!testBit(*bb, i));
    *bb ^= 1ull << i;
}

void clearBit(uint64_t *bb, int i) {
    assert(testBit(*bb, i));
    *bb ^= 1ull << i;
}

bool testBit(uint64_t bb, int i) {
    assert(0 <= i && i < SQUARE_NB);
    return bb & (1ull << i);
}

void printBitboard(uint64_t bb) {

    for (int rank = 7; rank >= 0; rank--) {
        char line[] = ". . . . . . . .";

        for (int file = 0; file < FILE_NB; file++)
            if (testBit(bb, square(rank, file)))
                line[2 * file] = 'X';

        printf("%s\n", line);
    }

    printf("\n");
}
