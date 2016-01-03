#include <stdio.h>
#include <stdlib.h>

#include "ttable.h"
#include "types.h"

#define ENTRY_GET_DEPTH(n)	(((n) & (0b1111111 << 17)) >> 17)
#define ENTRY_GET_VALUE(n)	(((n) & (0xFFFF)) - 32768)
#define ENTRY_GET_BOUND(n)	(((n) & (0b111 << 24)))
#define ENTRY_GET_TURN(n)	(((n) & (1 << 16)) >> 16)

extern ttable_t table;

void init_ttable_t(){
	printf("Intialized Table of size %.3fMb\n",(float)(sizeof(ttentry_t) * TableSize) / (1024 * 1024));
	table.entries = calloc(TableSize,sizeof(ttentry_t));
	if (table.entries == NULL)
		printf("Unable to Allocate Table\n");
}

int get_bound_type(int alpha, int beta, int value){
	if (value <= alpha)
		return LowerBound;
	if (value >= beta)
		return UpperBound;
	return ExactBound;
}

ttentry_t get_entry(int hash){
	return table.entries[abs(hash)%TableSize];
}

void store_entry(int hash, int alpha, int beta, int value, int turn, int depth){
	ttentry_t cur = get_entry(hash);
	int bound = get_bound_type(alpha,beta,value);
	
	if (cur == 0 || ENTRY_GET_DEPTH(cur) > depth || (bound == ExactBound && ENTRY_GET_BOUND(cur) != ExactBound))
		table.entries[hash%TableSize] = (value + 32768) | (turn << 16) | (depth << 17) | (bound);
}