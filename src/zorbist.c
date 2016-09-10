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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "piece.h"
#include "zorbist.h"

uint64_t ZorbistKeys[32][64];
uint64_t PawnKeys[32][64];

/**
 * Fill the ZorbistKeys[type][square] and the
 * PawnKeys[type][square] arrays with randomly
 * generated 64-bit Integers. Seed with zero
 * to generate consistent values for testing.
 */
void initalizeZorbist(){
    
    int p, s;
    
    srand(0);
    
    // Zero out both arrays
    for(p = 0; p < 32; p++){
        for(s = 0; s < 64; s++){
            ZorbistKeys[p][s] = 0ull;
            PawnKeys[p][s] = 0ull;
        }
    }
    
    // Fill ZorbistKeys for each piece type and square
    for(p = 0; p <= 5; p++){
        for(s = 0; s < 64; s++){
            ZorbistKeys[(p*4) + 0][s] = genRandomBitstring();
            ZorbistKeys[(p*4) + 1][s] = genRandomBitstring();
        }
    }
    
    // Fill PawnKeys for each pawn colour and square
    for (s = 0; s < 64; s++){
        PawnKeys[WhitePawn][s] = ZorbistKeys[WhitePawn][s];
        PawnKeys[BlackPawn][s] = ZorbistKeys[BlackPawn][s];
    }
}

/**
 * Generate a random 64-bit Integer.
 *
 * @return  Random 64-bit Integer.
 */
uint64_t genRandomBitstring(){
    
    uint64_t str = 0;
    int i;
    
    for(i = 0; i < 64; i++)
        str ^= ((uint64_t)(rand())) << (i);
    
    return str;
}