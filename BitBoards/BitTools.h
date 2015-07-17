#ifndef BitTools
#define BitTools

#include "Types.h"
#include "Board.h"

void printBitBoard(BitBoard b);
int getLSB(BitBoard b);
int getSquare(int sq, Board * board);

#endif