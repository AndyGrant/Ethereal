#ifndef EVALTABLE_H
#define EVALTABLE_H

#define EvalTableSize 		(1 << 12)
#define NoValidEntryFound	(10000000)

void init_evaltable_t();
int get_evalentry(unsigned long long hash, int turn);
void store_evalentry(unsigned long long hash, int turn, int value);

#endif