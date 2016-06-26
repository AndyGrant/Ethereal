#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "piece.h"
#include "zorbist.h"

int INITALIZED_ZORBIST = 0;

uint64_t ZorbistKeys[32][64];
uint64_t PawnKeys[32][64];

/**
 * Fill the ZorbistKeys[type][square] arrays with randomly
 * generated 64-bit Integers. Set flag INITALIZED_ZORBIST
 * so this process is not repeated. Seed rand with 0 sopen
 * when testing results are the same each time.
 */
void initalizeZorbist(){
    
    int p, s;
    
    if (INITALIZED_ZORBIST)
        return;
    
    srand(0);
    
    for(p = 0; p < 32; p++){
        for(s = 0; s < 64; s++){
            ZorbistKeys[p][s] = 0ull;
            PawnKeys[p][s] = 0ull;
        }
    }
        
    for(p = 0; p <= 5; p++){
        for(s = 0; s < 64; s++){
            ZorbistKeys[(p*4) + 0][s] = genRandomBitstring();
            ZorbistKeys[(p*4) + 1][s] = genRandomBitstring();
        }
    }
    
    for (s = 0; s < 64; s++){
        PawnKeys[WhitePawn][s] = ZorbistKeys[WhitePawn][s];
        PawnKeys[BlackPawn][s] = ZorbistKeys[BlackPawn][s];
    }
    
    INITALIZED_ZORBIST = 1;
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