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

#ifndef _BITUTILS_H
#define _BITUTILS_H

#include <stdint.h>

int countSetBits(uint64_t bb);
void getSetBits(uint64_t bb, int* arr);
int popcount(uint64_t bb);
int getlsb(uint64_t bb);
int getmsb(uint64_t bb);
int poplsb(uint64_t* bb);
int moreThanOne(uint64_t bb);

int BitCounts[0x10000];

#endif
