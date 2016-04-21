#include <stdio.h>
#include <stdlib.h>

#include "ttable.h"
#include "types.h"

extern ttable_t table;

void init_ttable_t(){
	printf("Intialized Table of size %.3fMb\n",(float)(sizeof(ttentry_t) * TableSize) / (1024 * 1024));
	table.entries = calloc(TableSize,sizeof(ttentry_t));
	table.hashes = calloc(TableSize,sizeof(unsigned long long));
	
	int i;
	for(i = 0; i < TableSize; i++){
		table.entries[i] = 0;
		table.hashes[i] = 0;
	}
}

int get_bound_type(int alpha, int beta, int value){
	if (value <= alpha)
		return LowerBound;
	if (value >= beta)
		return UpperBound;
	return ExactBound;
}

ttentry_t get_entry(unsigned long long hash){
	ttentry_t entry = table.entries[hash%TableSize];
	unsigned long long entryhash = table.hashes[hash%TableSize];
	
	if (entryhash == hash)
		return entry;
	
	return 0;
}

void store_entry(unsigned long long hash, int alpha, int beta, int value, int turn){
	ttentry_t cur = get_entry(hash);
	int bound = get_bound_type(alpha,beta,value);

	if (cur == 0 || bound == ExactBound){
		table.entries[hash%TableSize] = (value + 32768) | (turn << 16) | (bound);
		table.hashes[hash%TableSize] = hash;
	}
}