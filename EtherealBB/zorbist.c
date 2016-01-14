#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "zorbist.h"

void init_zorbist(){
	if (INITALIZED_ZORBIST)
		return;
	
	int p, s;
	
	srand(time(NULL));
	
	for(p = 0; p < 32; p++){
		for(s = 0; s < 64; s++){
			ZorbistKeys[p][s] = 0ull;
		}
	}
		
	for(p = 0; p <= 5; p++){
		for(s = 0; s < 64; s++){
			ZorbistKeys[(p*4) + 0][s] = gen_random_bitstring();
			ZorbistKeys[(p*4) + 1][s] = gen_random_bitstring();
		}
	}
	
	INITALIZED_ZORBIST = 1;
}

void validate_zorbist(){
	int pieces[12] = {0,1,4,5,8,9,12,13,16,17,20,21};
	int empty[20]  = {
		 2,  3,  6,  7, 10,
		11, 14, 15, 18, 19,
		22, 23, 24, 25, 26,
		27, 28, 29, 30, 31
	};
	
	int i, s;
	
	for(i = 0; i < 12; i++)
		for(s = 0; s < 64; s++)
			assert(ZorbistKeys[pieces[i]][s] != 0 && "Invalid Zorbist Entry!");
		
	for(i = 0; i < 20; i++)
		for(s = 0; s < 64; s++)
			assert(ZorbistKeys[empty[i]][s] == 0 && "Invalid Zorbist Entry!");
}

uint64_t gen_rand_64bit(){
	return (uint64_t)(rand());
}

uint64_t gen_random_bitstring(){
	uint64_t str = 0;
	int i;
	
	for(i = 0; i < 64; i++)
		str ^= gen_rand_64bit() << (i);
	
	return str;
}