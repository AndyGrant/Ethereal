#include <stdint.h>
#include <assert.h>

#include "bitboards.h"
#include "bitutils.h"

/**
 * Count the number of set bits in a given 64-bit Integer
 *
 * @param   bb  BitBoard to count set bits in
 * @return      Count of all set bits in bb
 */
int countSetBits(uint64_t bb){
    int count = 0;
    
    while(bb){
        bb ^= (1ull << getLSB(bb));
        count += 1;
    }
    
    return count;
}

/**
 * Fill an array with the bit indexes of all set bits
 * in a given 64-bit Integer. Set the array index after
 * the last bit index location to -1 to indicate that
 * all bit indexes have been traversed
 *
 * @param   bb  BitBoard to get set bits in
 * @param   arr Integer Array to fill with bit indexes
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