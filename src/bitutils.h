#ifndef _BITUTILS_H
#define _BITUTILS_H

#include <stdint.h>

unsigned int countSetBits(uint64_t bb);
void getSetBits(uint64_t bb, int * arr);

extern int LsbLookupTable[64];

#define getLSB(bb) (LsbLookupTable[(((bb) ^ ((bb)-1)) * 0x03f79d71b4cb0a89ull) >> 58])

#endif