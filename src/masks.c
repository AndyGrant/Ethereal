#include <stdint.h>
#include <stdio.h>

#include "bitboards.h"
#include "masks.h"
#include "psqt.h"

int INITALIZED_MASKS = 0;

uint64_t IsolatedPawnMasks[64];
uint64_t PassedPawnMasks[2][64];
uint64_t PawnAttackMasks[2][64];
uint64_t PawnAdvanceMasks[2][64];
uint64_t PawnConnectedMasks[2][64];
uint64_t OutpostSquareMasks[2][64];

/**
 * Fill the various masks used to aid in the evaluation
 * function. These masks provide an easy way to determine
 * if a pawn is passed or isolated; if a pawn can attack
 * a given square; if a knight or bishop is on an outpost
 * square; as well as a quick way to determine if a pawn
 * may advance to a given square.
 */
void initalizeMasks(){
    
    if (INITALIZED_MASKS)
        return;
    
    int i, j, file, rank;
    uint64_t files;
    
    // INITALIZE ISOLATED PAWN MASKS
    for (i = 0; i < 64; i++){
        file = i % 8;
        
        if (file > 0 && file < 7)
            IsolatedPawnMasks[i] = FILES[file+1] | FILES[file-1];
        else if (file > 0)
            IsolatedPawnMasks[i] = FILES[file-1];
        else
            IsolatedPawnMasks[i] = FILES[file+1];
    }
    
    // INITALIZE PASSED PAWN MASKS AND OUTPOST MASKS
    for (i = 0; i < 64; i++){
        file = i % 8;
        rank = i / 8;
        
        files = IsolatedPawnMasks[i] | FILES[file];
        
        PassedPawnMasks[0][i] = files;
        for (j = rank; j >= 0; j--)
            PassedPawnMasks[0][i] &= ~(RANKS[j]);
        
        PassedPawnMasks[1][i] = files;
        for (j = rank; j <= 7; j++)
            PassedPawnMasks[1][i] &= ~(RANKS[j]);
        
        OutpostSquareMasks[0][i] = PassedPawnMasks[0][i] & ~FILES[file];
        OutpostSquareMasks[1][i] = PassedPawnMasks[1][i] & ~FILES[file];
    }
    
    // INITALIZE ATTACK-SQ PAWN MASKS
    for (i = 0; i < 64; i++){
        file = i % 8;
        rank = i / 8;
        
        PawnAttackMasks[0][i] = 0ull;
        PawnAttackMasks[1][i] = 0ull;
        
        if (rank == 0){
            PawnAttackMasks[1][i] |= (1ull << i) << 7;
            PawnAttackMasks[1][i] |= (1ull << i) << 9;
        }
        
        else if (rank == 7){
            PawnAttackMasks[0][i] |= (1ull << i) >> 7;
            PawnAttackMasks[0][i] |= (1ull << i) >> 9;
        }
        
        else {
            PawnAttackMasks[0][i] |= (1ull << i) >> 7;
            PawnAttackMasks[0][i] |= (1ull << i) >> 9;
            PawnAttackMasks[1][i] |= (1ull << i) << 7;
            PawnAttackMasks[1][i] |= (1ull << i) << 9;
        }
        
        if (file == 0){
            PawnAttackMasks[0][i] &= ~FILE_H;
            PawnAttackMasks[1][i] &= ~FILE_H;
        }
        
        else if (file == 7){
            PawnAttackMasks[0][i] &= ~FILE_A;
            PawnAttackMasks[1][i] &= ~FILE_A;
        }
    }
    
    // INITALIZE PAWN-MAY-ADVANCE MASKS
    for (i = 0; i < 64; i++){
        rank = i / 8;
        
        PawnAdvanceMasks[0][i] = 0ull;
        PawnAdvanceMasks[1][i] = 0ull;
        
        if (rank == 0)
            PawnAdvanceMasks[0][i] = (1ull << i) << 8;
        else if (rank == 7)
            PawnAdvanceMasks[1][i] = (1ull << i) >> 8;
        else {
            PawnAdvanceMasks[0][i] = (1ull << i) << 8;
            PawnAdvanceMasks[1][i] = (1ull << i) >> 8;
        }
    }
    
    // INITALIZE PAWN-CONNECTED MASKS
    for (i = 8 ; i < 54; i++){
        file = i % 8;
        
        if (file == 0){
            PawnConnectedMasks[0][i] = (1ull << (i+1)) | (1ull << (i-7));
            PawnConnectedMasks[1][i] = (1ull << (i+1)) | (1ull << (i+9));
        }
        
        else if (file == 7){
            PawnConnectedMasks[0][i] = (1ull << (i-1)) | (1ull << (i-9));
            PawnConnectedMasks[1][i] = (1ull << (i-1)) | (1ull << (i+7));
        }
        
        else {
            PawnConnectedMasks[0][i] = (1ull << (i-1)) | (1ull << (i-9)) | (1ull << (i+1)) | (1ull << (i-7));
            PawnConnectedMasks[1][i] = (1ull << (i-1)) | (1ull << (i+7)) | (1ull << (i+1)) | (1ull << (i+9));
        }
    }
    
    INITALIZED_MASKS = 1;
}