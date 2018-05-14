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
#include <stdio.h>

#include "bitboards.h"
#include "masks.h"
#include "movegen.h"
#include "piece.h"
#include "types.h"

uint64_t BitsBetweenMasks[SQUARE_NB][SQUARE_NB];
uint64_t RanksAtOrAboveMasks[COLOUR_NB][RANK_NB];
uint64_t IsolatedPawnMasks[SQUARE_NB];
uint64_t PassedPawnMasks[COLOUR_NB][SQUARE_NB];
uint64_t PawnConnectedMasks[COLOUR_NB][SQUARE_NB];
uint64_t OutpostSquareMasks[COLOUR_NB][SQUARE_NB];
uint64_t OutpostRanks[COLOUR_NB];

void initializeMasks(){
    
    int i, j;
    uint64_t files;
    
    for (i = 0; i < SQUARE_NB; i++){
        for (j = 0; j < SQUARE_NB; j++){
            
            // Aligned on a diagonal
            if (bishopAttacks(i, 0ull, 1ull << j))
                BitsBetweenMasks[i][j] = bishopAttacks(i, 1ull << j, ~0ull) & bishopAttacks(j, 1ull << i, ~0ull);
            
            // Aligned on a straight
            if (rookAttacks(i, 0ull, 1ull << j))
                BitsBetweenMasks[i][j] = rookAttacks(i, 1ull << j, ~0ull) & rookAttacks(j, 1ull << i, ~0ull);
        }
    }
    
    // Initalize ranks above masks
    for (i = 0; i < RANK_NB; i++){
        for (j = i; j < RANK_NB; j++)
            RanksAtOrAboveMasks[WHITE][i] |= Ranks[j];
        for (j = i; j >= 0; j--)
            RanksAtOrAboveMasks[BLACK][i] |= Ranks[j];
    }
    
    // Initalize isolated pawn masks
    for (i = 0; i < SQUARE_NB; i++){
        if (fileOf(i) > 0 && fileOf(i) < 7)
            IsolatedPawnMasks[i] = Files[fileOf(i)+1] | Files[fileOf(i)-1];
        else if (fileOf(i) > 0)
            IsolatedPawnMasks[i] = Files[fileOf(i)-1];
        else
            IsolatedPawnMasks[i] = Files[fileOf(i)+1];
    }
    
    // Initalize passed pawn masks and outpost masks
    for (i = 0; i < SQUARE_NB; i++){
        
        files = IsolatedPawnMasks[i] | Files[fileOf(i)];
        
        PassedPawnMasks[WHITE][i] = files;
        for (j = rankOf(i); j >= 0; j--)
            PassedPawnMasks[WHITE][i] &= ~(Ranks[j]);
        
        PassedPawnMasks[BLACK][i] = files;
        for (j = rankOf(i); j <= 7; j++)
            PassedPawnMasks[BLACK][i] &= ~(Ranks[j]);
        
        OutpostSquareMasks[WHITE][i] = PassedPawnMasks[WHITE][i] & ~Files[fileOf(i)];
        OutpostSquareMasks[BLACK][i] = PassedPawnMasks[BLACK][i] & ~Files[fileOf(i)];
    }
    
    // Initalize relative outpost ranks
    OutpostRanks[WHITE] = RANK_4 | RANK_5 | RANK_6;
    OutpostRanks[BLACK] = RANK_3 | RANK_4 | RANK_5;
    
    // Initalize pawn connected masks
    for (i = 8 ; i < 56; i++){
        if (fileOf(i) == 0){
            PawnConnectedMasks[WHITE][i] = (1ull << (i+1)) | (1ull << (i-7));
            PawnConnectedMasks[BLACK][i] = (1ull << (i+1)) | (1ull << (i+9));
        }
        
        else if (fileOf(i) == 7){
            PawnConnectedMasks[WHITE][i] = (1ull << (i-1)) | (1ull << (i-9));
            PawnConnectedMasks[BLACK][i] = (1ull << (i-1)) | (1ull << (i+7));
        }
        
        else {
            PawnConnectedMasks[WHITE][i] = (1ull << (i-1)) | (1ull << (i-9)) | (1ull << (i+1)) | (1ull << (i-7));
            PawnConnectedMasks[BLACK][i] = (1ull << (i-1)) | (1ull << (i+7)) | (1ull << (i+1)) | (1ull << (i+9));
        }
    }
}
