#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Board.h"
#include "Types.h"

int WHITE = 0;
int BLACK = 1;

int PAWN = 0;
int BISHOP = 1;
int KNIGHT = 2;
int ROOK = 3;
int QUEEN = 4;
int KING = 5;
int EMPTY = 6;

BitBoard startingWhiteAll 	= 0x0000F0000000FFFF;
BitBoard startingBlackALL 	= 0xFFFF000000000000;
BitBoard startingKings 		= 0x1000000000000010;
BitBoard startingQueens		= 0x0800000000000008;
BitBoard startingRooks 		= 0x8100000000000081;
BitBoard startingKnights		= 0x4200000000000042;
BitBoard startingBishops		= 0x2400000000000024;
BitBoard startingPawns 		= 0x00FFF0000000FF00;

BitBoard RANK_8 = 0xFF00000000000000;
BitBoard RANK_7 = 0x00FF000000000000;
BitBoard RANK_6 = 0x0000FF0000000000;
BitBoard RANK_5 = 0x000000FF00000000;
BitBoard RANK_4 = 0x00000000FF000000;
BitBoard RANK_3 = 0x0000000000FF0000;
BitBoard RANK_2 = 0x000000000000FF00;
BitBoard RANK_1 = 0x00000000000000FF;

BitBoard FILE_A = 9259542123273814144ull;
BitBoard FILE_B = 4629771061636907072ull;
BitBoard FILE_C = 2314885530818453536ull;
BitBoard FILE_D = 1157442765409226768ull;
BitBoard FILE_E = 578721382704613384ull;
BitBoard FILE_F = 289360691352306692ull;
BitBoard FILE_G = 144680345676153346ull;
BitBoard FILE_H = 72340172838076673ull;

int MagicNumberShiftsRook[64] = {
	52,53,53,53,53,53,53,52,53,54,54,54,54,54,54,53,
	53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,
	53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,
	53,54,54,54,54,54,54,53,52,53,53,53,53,53,53,52
};

int MagicNumberShiftsBishop[64] = {
	58,59,59,59,59,59,59,58,59,59,59,59,59,59,59,59,
	59,59,57,57,57,57,59,59,59,59,57,55,55,57,59,59,
	59,59,57,55,55,57,59,59,59,59,57,57,57,57,59,59,
	59,59,59,59,59,59,59,59,58,59,59,59,59,59,59,58
};


Board * BoardInit(){

	Board * b = malloc(sizeof(Board));
	
	b->WhiteAll 			= startingWhiteAll;
	b->BlackAll			= startingBlackALL;
	b->Kings 				= startingKings;
	b->Queens				= startingQueens;
	b->Rooks				= startingRooks;
	b->Knights			= startingKnights;
	b->Bishops			= startingBishops;
	b->Pawns				= startingPawns;
	
	b->Turn				= 0;
	b->Enpass				= 100;
	b->FiftyMoveRule		= 50;
	
	b->ValidCastles[0][0] 	= 1;
	b->ValidCastles[0][1] 	= 1;
	b->ValidCastles[1][0] 	= 1;
	b->ValidCastles[1][1] 	= 1;
	b->Castled[0] 			= 0;
	b->Castled[1] 			= 0;
	
	b->KingMap 			= generateKingMap();
	b->KnightMap 			= generateKnightMap();
	
	b->OccupancyMaskRook 	= generateOccupancyMaskRook();
	b->MagicShiftsRook		= MagicNumberShiftsRook;
	
	b->OccupancyMaskBishop 	= generateOccupancyMaskBishop();
	b->MagicShiftsBishop 	= MagicNumberShiftsBishop;
	
	
	return b;
}

BitBoard * generateKingMap(){
	BitBoard * bb = calloc(64,sizeof(BitBoard));
	int i; BitBoard z = 1;
	for(i = 0; i < 64; i++){		
		if (i + 9 < 64 && i % 8 != 7)
			bb[i] |= z << (i + 9);
		if (i - 9 >= 0 && i % 8 != 0)
			bb[i] |= z << (i - 9);	
		if (i + 7 < 64 && i % 8 != 0)
			bb[i] |= z << (i + 7);
		if (i - 7 >= 0 && i % 8 != 7)
			bb[i] |= z << (i - 7);	
		if (i + 1 < 64 && i % 8 != 7)
			bb[i] |= z << (i + 1);
		if (i - 1 >= 0 && i % 8 != 0)
			bb[i] |= z << (i - 1);
		if (i + 8 < 64)
			bb[i] |= z << (i + 8);
		if (i - 8 >= 0)
			bb[i] |= z << (i - 8);
	}
	return bb;
}

BitBoard * generateKnightMap(){
	BitBoard * bb = calloc(64,sizeof(BitBoard));
	int i; BitBoard z = 1;
	for(i = 0; i < 64; i++){
		if (i + 17 < 64 && i % 8 != 7)
			bb[i] |= z << ( i + 17);
		if (i - 17 >= 0 && i % 8 != 0)
			bb[i] |= z << ( i - 17);
			
		if (i + 15 < 64 && i % 8 != 0)
			bb[i] |= z << ( i + 15);
		if (i - 15 >= 0 && i % 8 != 7)
			bb[i] |= z << ( i - 15);
			
		if (i + 10 < 64 && i % 8 <= 5)
			bb[i] |= z << ( i + 10);
		if (i - 10 >= 0 && i % 8 >= 2)
			bb[i] |= z << ( i - 10);
			
		if (i + 6  < 64 && i % 8 >= 2)
			bb[i] |= z << ( i + 6);
		if (i - 6  >= 0 && i % 8 <= 5)
			bb[i] |= z << ( i - 6);
	}
	return bb;
}

BitBoard * generateOccupancyMaskRook(){

	BitBoard * occupancyMaskRook = malloc(sizeof(BitBoard) * 64);

	int i, bit;
	BitBoard mask;
	for (bit=0; bit<=63; bit++, mask = 0){
		for (i=bit+8; i<=55; i+=8)
			mask |= (1L << i);
		for (i=bit-8; i>=8; i-=8)
			mask |= (1L << i);
		for (i=bit+1; i%8!=7 && i%8!=0 ; i++)
			mask |= (1L << i);
		for (i=bit-1; i%8!=7 && i%8!=0 && i>=0; i--)
			mask |= (1L << i);
		occupancyMaskRook[bit] = mask;
	}
	
	return occupancyMaskRook;
}

BitBoard * generateOccupancyMaskBishop(){

	BitBoard * occupancyMaskBishop = malloc(sizeof(BitBoard) * 64);
	
	int i, bit;	
	BitBoard mask;
	for (bit=0; bit<=63; bit++, mask = 0){
		for (i=bit+9; i%8!=7 && i%8!=0 && i<=55; i+=9)
			mask |= (1L << i);
		for (i=bit-9; i%8!=7 && i%8!=0 && i>=8; i-=9)
			mask |= (1L << i);
		for (i=bit+7; i%8!=7 && i%8!=0 && i<=55; i+=7)
			mask |= (1L << i);
		for (i=bit-7; i%8!=7 && i%8!=0 && i>=8; i-=7)
			mask |= (1L << i);
		occupancyMaskBishop[bit] = mask;
	}
	
	return occupancyMaskBishop;
}

int main(){
	Board * b = BoardInit();
	time_t start = time(NULL);
	
	Move * moves[256];
	int index = 0;
		
	getKnightMoves(b,moves,&index,BLACK);
	getPawnMoves(b,moves,&index,BLACK);
		
	
		
	printf("Found %d moves\n",index);
	
	printf("Time Taken : %d \n",(int)(time(NULL)-start));
	
	int i;
	for(i = 0; i < index; i++)
		printf("Move found from %d to %d using %d taking %d\n",moves[i]->Start,moves[i]->End,moves[i]->MovedType,moves[i]->CapturedType);
}
