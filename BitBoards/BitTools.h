#ifndef BitTools
#define BitTools

#include "Types.h"
#include "Board.h"

extern int index64[64];

void printBitBoard(BitBoard b);
#define getLSB(b) index64[((b ^ (b-1)) * 0x03f79d71b4cb0a89ull) >> 58]
int getSquare(int sq, Board * board);
int countSetBits(BitBoard b);
void getSetBits(BitBoard b, int * arr);

#endif