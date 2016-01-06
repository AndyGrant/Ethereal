#include <stdlib.h>

#include "evaltable.h"
#include "types.h"

extern evaltable_t evaltable;

void init_evaltable_t(){
	evaltable.hashes = calloc(EvalTableSize,sizeof(unsigned long long));
	evaltable.turns  = calloc(EvalTableSize,sizeof(int));
	evaltable.values = calloc(EvalTableSize,sizeof(int));
	evaltable.skipped = 0;
	
	int i;
	for(i = 0; i < EvalTableSize; i++)
		evaltable.values[i] = NoValidEntryFound;
}

int get_evalentry(unsigned long long hash, int turn){
	int index = hash % EvalTableSize;
	
	if (evaltable.hashes[index] == hash && evaltable.turns[index] == turn)
		return evaltable.values[index];
	
	return NoValidEntryFound;
}


void store_evalentry(unsigned long long hash, int turn, int value){
	int index = hash % EvalTableSize;
	
	evaltable.hashes[index] = hash;
	evaltable.turns[index]  = turn;
	evaltable.values[index] = value;
}