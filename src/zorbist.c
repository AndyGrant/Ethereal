#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "zorbist.h"

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

uint64_t genRandomBitstring(){
    uint64_t str = 0;
    int i;
    
    for(i = 0; i < 64; i++)
        str ^= ((uint64_t)(rand())) << (i);
    
    return str;
}