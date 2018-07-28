#ifndef TTABLE_H
#define TTABLE_H

#include "types.h"

#define ExactBound	(1 << 24)
#define LowerBound	(1 << 25)
#define UpperBound	(1 << 26)
#define TableSize	(1 << 25)

#define ENTRY_GET_DEPTH(n)	(((n) & (0b1111111 << 17)) >> 17)
#define ENTRY_GET_VALUE(n)	(((n) & (0xFFFF)) - 32768)
#define ENTRY_GET_BOUNDS(n)	((n) & (ExactBound | LowerBound | UpperBound))
#define ENTRY_GET_TURN(n)	(((n) & (1 << 16)) >> 16)
#define ENTRY_GET_MOVE(n)	((n) >> 32)
void init_ttable_t();
int get_bound_type(int alpha, int beta, int value);
ttentry_t get_entry(unsigned long long hash);
void store_entry(unsigned long long hash, int alpha, int beta, int value, int turn);

#endif