#include <stdint.h>
#include <assert.h>

#include "bitboards.h"
#include "bitutils.h"


/*
 *  Determine number of set bits in a given 
 *  unsigned 64-bit integer BitBoard
 */
int countSetBits(uint64_t bb){
    if (!bb)
        return 0;
    
    int count = 0;
    while(bb){
        bb ^= (1ull << getLSB(bb));
        count += 1;
    }
    return count;
}

/* 
 * Fill given int array with bit locations of
 * set bits in a given unsigned 64-bit integer BitBoard
 */
void getSetBits(uint64_t bb, int * arr){
    int count = 0;
    while(bb){
        int lsb = getLSB(bb);
        arr[count] = lsb;
        count += 1;
        bb ^= 1ull << lsb;
    }
    
    arr[count] = -1;    
}