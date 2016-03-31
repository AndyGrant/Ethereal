#include <stdint.h>

#include "bitutils.h"

/*
 *	Determine number of set bits in a given 
 *	unsigned 64-bit integer BitBoard
 */
int count_set_bits(uint64_t bb){
	if (!bb)
		return 0;
	
	int count = 0;
	while(bb){
		int lsb = get_lsb(bb);
		count += 1;
		bb ^= (1ull << lsb);
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

