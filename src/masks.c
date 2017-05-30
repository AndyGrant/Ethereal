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
#include "piece.h"
#include "types.h"

uint64_t IsolatedPawnMasks[SQUARE_NB];
uint64_t PassedPawnMasks[COLOUR_NB][SQUARE_NB];
uint64_t PawnAttackMasks[COLOUR_NB][SQUARE_NB];
uint64_t PawnConnectedMasks[COLOUR_NB][SQUARE_NB];
uint64_t OutpostSquareMasks[COLOUR_NB][SQUARE_NB];
uint64_t OutpostRanks[COLOUR_NB];

/**
 * Fill the various masks used to aid in the evaluation
 * function. These masks provide an easy and cheap way
 * to determine various components for the evaluation.
 */
void initalizeMasks(){
    
    int i, j, file, rank;
    uint64_t files;
    
    // Initalize isolated pawn masks
    for (i = 0; i < SQUARE_NB; i++){
        
        file = File(i);
        
        if (file > 0 && file < 7)
            IsolatedPawnMasks[i] = Files[file+1] | Files[file-1];
        else if (file > 0)
            IsolatedPawnMasks[i] = Files[file-1];
        else
            IsolatedPawnMasks[i] = Files[file+1];
    }
    
    // Initalize passed pawn masks and outpost masks
    for (i = 0; i < SQUARE_NB; i++){
        
        file = File(i); rank = Rank(i);
        files = IsolatedPawnMasks[i] | Files[file];
        
        PassedPawnMasks[WHITE][i] = files;
        for (j = rank; j >= 0; j--)
            PassedPawnMasks[WHITE][i] &= ~(Ranks[j]);
        
        PassedPawnMasks[BLACK][i] = files;
        for (j = rank; j <= 7; j++)
            PassedPawnMasks[BLACK][i] &= ~(Ranks[j]);
        
        OutpostSquareMasks[WHITE][i] = PassedPawnMasks[WHITE][i] & ~Files[file];
        OutpostSquareMasks[BLACK][i] = PassedPawnMasks[BLACK][i] & ~Files[file];
    }
    
    // Initalize relative outpost ranks
    OutpostRanks[WHITE] = RANK_4 | RANK_5 | RANK_6;
    OutpostRanks[BLACK] = RANK_3 | RANK_4 | RANK_5;
    
    // Initalize attack square pawn masks
    for (i = 0; i < SQUARE_NB; i++){
        
        file = File(i); rank = Rank(i);
        PawnAttackMasks[WHITE][i] = 0ull;
        PawnAttackMasks[BLACK][i] = 0ull;
        
        if (rank == 0){
            PawnAttackMasks[BLACK][i] |= (1ull << i) << 7;
            PawnAttackMasks[BLACK][i] |= (1ull << i) << 9;
        }
        
        else if (rank == 7){
            PawnAttackMasks[WHITE][i] |= (1ull << i) >> 7;
            PawnAttackMasks[WHITE][i] |= (1ull << i) >> 9;
        }
        
        else {
            PawnAttackMasks[WHITE][i] |= (1ull << i) >> 7;
            PawnAttackMasks[WHITE][i] |= (1ull << i) >> 9;
            PawnAttackMasks[BLACK][i] |= (1ull << i) << 7;
            PawnAttackMasks[BLACK][i] |= (1ull << i) << 9;
        }
        
        if (file == 0){
            PawnAttackMasks[WHITE][i] &= ~FILE_H;
            PawnAttackMasks[BLACK][i] &= ~FILE_H;
        }
        
        else if (file == 7){
            PawnAttackMasks[WHITE][i] &= ~FILE_A;
            PawnAttackMasks[BLACK][i] &= ~FILE_A;
        }
    }
    
    // Initalize pawn connected masks
    for (i = 8 ; i < 56; i++){
        
        file = File(i);
        
        if (file == 0){
            PawnConnectedMasks[WHITE][i] = (1ull << (i+1)) | (1ull << (i-7));
            PawnConnectedMasks[BLACK][i] = (1ull << (i+1)) | (1ull << (i+9));
        }
        
        else if (file == 7){
            PawnConnectedMasks[WHITE][i] = (1ull << (i-1)) | (1ull << (i-9));
            PawnConnectedMasks[BLACK][i] = (1ull << (i-1)) | (1ull << (i+7));
        }
        
        else {
            PawnConnectedMasks[WHITE][i] = (1ull << (i-1)) | (1ull << (i-9)) 
                                         | (1ull << (i+1)) | (1ull << (i-7));
                                         
            PawnConnectedMasks[BLACK][i] = (1ull << (i-1)) | (1ull << (i+7)) 
                                         | (1ull << (i+1)) | (1ull << (i+9));
        }
    }
}