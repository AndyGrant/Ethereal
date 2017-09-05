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

#include "castle.h"
#include "piece.h"
#include "types.h"
#include "zorbist.h"

uint64_t ZorbistKeys[32][SQUARE_NB];
uint64_t PawnKeys[32][SQUARE_NB];

void initalizeZorbist(){
    
    int p, s;
    
    srand(0);
    
    // Fill ZorbistKeys for each piece type and square
    for (p = 0; p <= 5; p++){
        for(s = 0; s < SQUARE_NB; s++){
            ZorbistKeys[(p*4) + 0][s] = genRandomBitstring();
            ZorbistKeys[(p*4) + 1][s] = genRandomBitstring();
        }
    }
    
    // Fill ZorbistKeys for the file of the enpass square
    for (p = 0; p < 8; p++)
        ZorbistKeys[ENPASS][p] = genRandomBitstring();
    
    // Fill ZorbistKeys for the state of the castle rights
    ZorbistKeys[CASTLE][WHITE_KING_RIGHTS] = genRandomBitstring();
    ZorbistKeys[CASTLE][WHITE_QUEEN_RIGHTS] = genRandomBitstring();
    ZorbistKeys[CASTLE][BLACK_KING_RIGHTS] = genRandomBitstring();
    ZorbistKeys[CASTLE][BLACK_QUEEN_RIGHTS] = genRandomBitstring();
    
    // Set each location as a combination of the four we just defined
    for (p = 0; p < 16; p++){
        
        if (p & WHITE_KING_RIGHTS)
            ZorbistKeys[CASTLE][p] ^= ZorbistKeys[CASTLE][WHITE_KING_RIGHTS];
        
        if (p & WHITE_QUEEN_RIGHTS)
            ZorbistKeys[CASTLE][p] ^= ZorbistKeys[CASTLE][WHITE_QUEEN_RIGHTS];
        
        if (p & BLACK_KING_RIGHTS)
            ZorbistKeys[CASTLE][p] ^= ZorbistKeys[CASTLE][BLACK_KING_RIGHTS];
        
        if (p & BLACK_QUEEN_RIGHTS)
            ZorbistKeys[CASTLE][p] ^= ZorbistKeys[CASTLE][BLACK_QUEEN_RIGHTS];
    }
    
    // Fill in the key for side to move
    ZorbistKeys[TURN][0] = genRandomBitstring();
    
    // Fill PawnKeys for each pawn colour and square
    for (s = 0; s < SQUARE_NB; s++){
        PawnKeys[WHITE_PAWN][s] = ZorbistKeys[WHITE_PAWN][s];
        PawnKeys[BLACK_PAWN][s] = ZorbistKeys[BLACK_PAWN][s];
    }
}

uint64_t genRandomBitstring(){
    
    int i;
    uint64_t str;
    
    for(i = 0, str = 0ull; i < 64; i++)
        str ^= ((uint64_t)(rand())) << i;
    
    return str;
}