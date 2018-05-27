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
#include "zobrist.h"

uint64_t ZobristKeys[32][SQUARE_NB];
uint64_t PawnKingKeys[32][SQUARE_NB];

void initializeZorbist(){
    
    int p, s;
    
    // Fill ZobristKeys for each piece type and square
    for (p = 0; p <= 5; p++){
        for(s = 0; s < SQUARE_NB; s++){
            ZobristKeys[(p*4) + 0][s] = rand64();
            ZobristKeys[(p*4) + 1][s] = rand64();
        }
    }
    
    // Fill ZobristKeys for the file of the enpass square
    for (p = 0; p < 8; p++)
        ZobristKeys[ENPASS][p] = rand64();
    
    // Fill ZobristKeys for the state of the castle rights
    ZobristKeys[CASTLE][WHITE_KING_RIGHTS ] = rand64();
    ZobristKeys[CASTLE][WHITE_QUEEN_RIGHTS] = rand64();
    ZobristKeys[CASTLE][BLACK_KING_RIGHTS ] = rand64();
    ZobristKeys[CASTLE][BLACK_QUEEN_RIGHTS] = rand64();
    
    // Set each location as a combination of the four we just defined
    for (p = 0; p < 16; p++){
        
        if (p & WHITE_KING_RIGHTS)
            ZobristKeys[CASTLE][p] ^= ZobristKeys[CASTLE][WHITE_KING_RIGHTS];
        
        if (p & WHITE_QUEEN_RIGHTS)
            ZobristKeys[CASTLE][p] ^= ZobristKeys[CASTLE][WHITE_QUEEN_RIGHTS];
        
        if (p & BLACK_KING_RIGHTS)
            ZobristKeys[CASTLE][p] ^= ZobristKeys[CASTLE][BLACK_KING_RIGHTS];
        
        if (p & BLACK_QUEEN_RIGHTS)
            ZobristKeys[CASTLE][p] ^= ZobristKeys[CASTLE][BLACK_QUEEN_RIGHTS];
    }
    
    // Fill in the key for side to move
    ZobristKeys[TURN][0] = rand64();
    
    // Fill PawnKingKeys for each Pawn And King colour and square
    for (s = 0; s < SQUARE_NB; s++){
        PawnKingKeys[WHITE_PAWN][s] = ZobristKeys[WHITE_PAWN][s];
        PawnKingKeys[BLACK_PAWN][s] = ZobristKeys[BLACK_PAWN][s];
        PawnKingKeys[WHITE_KING][s] = ZobristKeys[WHITE_KING][s];
        PawnKingKeys[BLACK_KING][s] = ZobristKeys[BLACK_KING][s];
    }
}

uint64_t rand64(){
    
    // http://vigna.di.unimi.it/ftp/papers/xorshift.pdf
    
    static uint64_t seed = 1070372ull;
    
    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    
    return seed * 2685821657736338717ull;
}
