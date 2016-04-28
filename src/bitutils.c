#include <stdint.h>
#include <assert.h>

#include "bitboards.h"
#include "bitutils.h"


/*
 *  Determine number of set bits in a given 
 *  unsigned 64-bit integer BitBoard
 */
int count_set_bits(uint64_t bb){
    if (!bb)
        return 0;
    
    int count = 0;
    while(bb){
        bb ^= (1ull << get_lsb(bb));
        count += 1;
    }
    return count;
}

/* 
 * Fill given int array with bit locations of
 * set bits in a given unsigned 64-bit integer BitBoard
 */
void get_set_bits(uint64_t bb, int * arr){
    int count = 0;
    while(bb){
        int lsb = get_lsb(bb);
        arr[count] = lsb;
        count += 1;
        bb ^= 1ull << lsb;
    }
    
    arr[count] = -1;    
}

/*
 * Determine the Most Significant Bit of a BitBoard
 *
 * This can ONLY be used when we know that
 * all bits will be in the same column, (FILE)
 */
int get_msb_special(uint64_t bb){
    if (bb & RANK_7) return 48;
    if (bb & RANK_6) return 40;
    if (bb & RANK_5) return 32;
    if (bb & RANK_4) return 24;
    if (bb & RANK_3) return 16;
    if (bb & RANK_2) return  8;
    assert("Invalid uint64_t send!\n" && 0);
}

