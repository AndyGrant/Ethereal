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
#include <stdint.h>
#include <stdio.h>

#include "bitboards.h"
#include "piece.h"

const uint64_t Files[FILE_NB] = {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
const uint64_t Ranks[RANK_NB] = {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};

int fileOf(int s) {
    assert(0 <= s && s < SQUARE_NB);
    return s % FILE_NB;
}

int rankOf(int s) {
    assert(0 <= s && s < SQUARE_NB);
    return s / FILE_NB;
}

int relativeRankOf(int c, int s) {
    assert(0 <= c && c < COLOUR_NB);
    assert(0 <= s && s < SQUARE_NB);
    return c == WHITE ? rankOf(s) : 7 - rankOf(s);
}

int square(int r, int f) {
    assert(0 <= r && r < RANK_NB);
    assert(0 <= f && f < FILE_NB);
    return r * FILE_NB + f;
}

int popcount(uint64_t b) {
    return __builtin_popcountll(b);
}

int getlsb(uint64_t b) {
    assert(b);  // lsb(0) is undefined
    return __builtin_ctzll(b);
}

int getmsb(uint64_t b) {
    assert(b);  // msb(0) is undefined
    return __builtin_clzll(b) ^ 63;
}

int poplsb(uint64_t *b) {
    int lsb = getlsb(*b);
    *b &= *b - 1;
    return lsb;
}

bool several(uint64_t b) {
    return b & (b - 1);
}

void setBit(uint64_t *b, int i) {
    assert(!testBit(*b, i));
    *b ^= 1ull << i;
}

void clearBit(uint64_t *b, int i) {
    assert(testBit(*b, i));
    *b ^= 1ull << i;
}

bool testBit(uint64_t b, int i) {
    assert(0 <= i && i < 64);
    return b & (1ull << i);
}

void printBitboard(uint64_t b) {

    for (int r = 7; r >= 0; r--) {
        char line[] = ". . . . . . . .";

        for (int f = 0; f < FILE_NB; f++) {
            if (testBit(b, square(r, f)))
                line[2 * f] = 'X';
        }

        puts(line);
    }

    puts("");
}
