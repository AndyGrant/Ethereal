#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "zorbist.h"

/*
 *  Fill ZorbistKeys[32][64] (zorbist.h) for a 2d-array
 *  for [PieceType][SquareNum] with randomly generated
 *  64-bit Integers. 
 *  
 *  Sets Flag INITALIZED_ZORBIST (zorbist.h) to indicate
 *  that the ZorbistKeys have been initalized.
 *
 *  Initialized when a struct Board is created via 
 *  init_board(...) (board.h)
 */
void initalizeZorbist(){
    if (INITALIZED_ZORBIST)
        return;
    
    int p, s;
    
    srand(0);
    
    for(p = 0; p < 32; p++){
        for(s = 0; s < 64; s++){
            ZorbistKeys[p][s] = 0ull;
        }
    }
        
    for(p = 0; p <= 5; p++){
        for(s = 0; s < 64; s++){
            ZorbistKeys[(p*4) + 0][s] = genRandomBitstring();
            ZorbistKeys[(p*4) + 1][s] = genRandomBitstring();
        }
    }
    
    INITALIZED_ZORBIST = 1;
}

/*
 *  Generates random 64-bit Integer for use in 
 *  ZorbistKeys (zorbist.h). XORs 64 randomly generated
 *  Integers whose bit-lengths are not defined by rand()
 */
uint64_t genRandomBitstring(){
    uint64_t str = 0;
    int i;
    
    for(i = 0; i < 64; i++)
        str ^= ((uint64_t)(rand())) << (i);
    
    return str;
}