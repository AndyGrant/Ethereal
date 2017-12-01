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

#include "bitutils.h"

uint8_t BitCounts[0x10000];

int countSetBits(uint64_t bb){
    
    int count = 0;
    
    while (bb){
        bb ^= 1ull << getlsb(bb);
        count += 1;
    }
    
    return count;
}

int popcount(uint64_t bb){
    return  BitCounts[(bb >>  0) & 0xFFFF]
          + BitCounts[(bb >> 16) & 0xFFFF]
          + BitCounts[(bb >> 32) & 0xFFFF]
          + BitCounts[(bb >> 48) & 0xFFFF];
}

void getSetBits(uint64_t bb, int * arr){
    
    int lsb, count = 0;
    
    while (bb){
        lsb = poplsb(&bb);
        arr[count++] = lsb;
    }
    
    arr[count] = -1;
}

int getlsb(uint64_t bb){
    #if defined (__GNUC__)
        return __builtin_ctzll(bb);
    #else
        static const int LsbTable[64] = {
           0, 47,  1, 56, 48, 27,  2, 60,
          57, 49, 41, 37, 28, 16,  3, 61,
          54, 58, 35, 52, 50, 42, 21, 44,
          38, 32, 29, 23, 17, 11,  4, 62,
          46, 55, 26, 59, 40, 36, 15, 53,
          34, 51, 20, 43, 31, 22, 10, 45,
          25, 39, 14, 33, 19, 30,  9, 24,
          13, 18,  8, 12,  7,  6,  5, 63
        };
        return LsbTable[((bb ^ (bb - 1)) * 0x03f79d71b4cb0a89ull) >> 58];
    #endif
}

int poplsb(uint64_t * bb){
    int lsb = getlsb(*bb);
    *bb ^= 1ull << lsb;
    return lsb;
}
