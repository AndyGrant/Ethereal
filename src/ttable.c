#include <stdio.h>
#include <stdlib.h>

#include "ttable.h"
#include "types.h"

extern ttable_t table;

void init_ttable_t(){
	printf("Intialized Table of size %.3fMb\n",(float)(sizeof(ttentry_t) * TableSize) / (1024 * 1024));
	table.entries = calloc(TableSize,sizeof(ttentry_t));
	
	int i;
	for(i = 0; i < TableSize; i++)
		table.entries[i] = 0;
}

int get_bound_type(int alpha, int beta, int value){
	if (value <= alpha)
		return LowerBound;
	if (value >= beta)
		return UpperBound;
	return ExactBound;
}

ttentry_t get_entry(int hash){
	ttentry_t entry = table.entries[abs(hash)%TableSize];
	
	if (ENTRY_GET_HASH(entry) == abs(hash))
		return entry;
	
	return 0;
}

void store_entry(int hash, int alpha, int beta, int value, int turn, int depth){
	ttentry_t cur = get_entry(hash);
	int bound = get_bound_type(alpha,beta,value);
	
	if (cur == 0 || ENTRY_GET_DEPTH(cur) < depth)
		table.entries[abs(hash)%TableSize] = (value + 32768) | (turn << 16) | (depth << 17) | (bound) | ((unsigned long long)(abs(hash)) << 32);
}