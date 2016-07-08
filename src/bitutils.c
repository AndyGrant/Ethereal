#include <stdint.h>
#include <assert.h>

#include "bitboards.h"
#include "bitutils.h"

int LsbLookupTable[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

/**
 * Count the number of set bits in a given 64-bit Integer
 *
 * @param   bb  BitBoard to count set bits in
 * @return      Count of all set bits in bb
 */
unsigned int countSetBits(uint64_t bb){
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