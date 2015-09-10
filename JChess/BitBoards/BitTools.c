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

int getSquare(int sq, Board * board){
	BitBoard mask = 1ull << sq;
	if ((mask & (board->Colors[0] | board->Colors[1])) == 0)
		return EMPTY;
	if (mask & board->Pieces[PAWN])
		return PAWN;
	if (mask & board->Pieces[BISHOP])
		return BISHOP;
	if (mask & board->Pieces[KNIGHT])
		return KNIGHT;
	if (mask & board->Pieces[ROOK])
		return ROOK;
	if (mask & board->Pieces[QUEEN])
		return QUEEN;
	if (mask & board->Pieces[KING])
		return KING;
}

int countSetBits(BitBoard b){
	if (!b)
		return 0;
	int count = 0;
	while(b){
		int lsb = getLSB(b);
		count += 1;
		b ^= (1ull << lsb);
	}
	return count;
}

void getSetBits(BitBoard b, int * arr){
	int count = 0;
	while(b){
		int lsb = getLSB(b);
		arr[count] = lsb;
		count += 1;
		b ^= 1ull << lsb;
	}
	
	arr[count] = -1;	
}
