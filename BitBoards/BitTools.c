#include <stdio.h>
#include "Types.h"
#include "Board.h"
#include "BitTools.h"

int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

void printBitBoard(BitBoard b){
	int x, y;
	printf("Printing Bit Board \n");
	for(x = 7; x >= 0; x--){
		for(y = 0; y < 8; y++)
			printf("%d",(b & (1ull << (x*8 + y))) != 0);
		printf("\n");
	}
	printf("\n");
}

int getLSB(BitBoard b){
	BitBoard debruijn64 = 0x03f79d71b4cb0a89;
	return index64[((b ^ (b-1)) * debruijn64) >> 58];
}

int getSquare(int sq, Board * board){
	BitBoard mask = 1ull << sq;
	if (!(mask & (board->WhiteAll | board->BlackAll)))
		return EMPTY;
	if (mask & board->Pawns)
		return PAWN;
	if (mask & board->Bishops)
		return BISHOP;
	if (mask & board->Knights)
		return KNIGHT;
	if (mask & board->Rooks)
		return ROOK;
	if (mask & board->Queens)
		return QUEEN;
	if (mask & board->Kings)
		return KING;
}