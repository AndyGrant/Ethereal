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

void getSetBits(uint64_t bb, int* arr){
    
    int lsb, count = 0;
    
    while (bb){
        lsb = poplsb(&bb);
        arr[count++] = lsb;
    }
    
    arr[count] = -1;
}

int getlsb(uint64_t bb){
    return __builtin_ctzll(bb);
}

int getmsb(uint64_t bb){
    return __builtin_clzll(bb) ^ 63;
}

int poplsb(uint64_t* bb){
    int lsb = getlsb(*bb);
    *bb ^= 1ull << lsb;
    return lsb;
}
