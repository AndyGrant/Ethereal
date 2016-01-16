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