#ifndef TTABLE_H
#define TTABLE_H

#include "types.h"

#define ExactBound	(1 << 24)
#define LowerBound	(1 << 25)
#define UpperBound	(1 << 26)

#define TableSize	(1 << 16)

extern ttable_t table;

void init_ttable_t();
int get_bound_type(int alpha, int beta, int value);
ttentry_t get_entry(int hash);
void store_entry(int hash, int alpha, int beta, int value, int turn, int depth);



#endif