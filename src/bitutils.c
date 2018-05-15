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

#include "bitutils.h"

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

int poplsb(uint64_t* bb) {
    int lsb = getlsb(*bb);
    *bb &= *bb - 1;
    return lsb;
}

bool several(uint64_t bb) {
    return bb & (bb - 1);
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
    assert(0 <= i && i < 64);
    return bb & (1ull << i);
}
