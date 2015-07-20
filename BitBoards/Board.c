#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Board.h"
#include "Types.h"
#include "Engine.h"

int WHITE = 0;
int BLACK = 1;

int PAWN = 0;
int BISHOP = 1;
int KNIGHT = 2;
int ROOK = 3;
int QUEEN = 4;
int KING = 5;
int EMPTY = 6;


BitBoard startingWhiteAll 	= 0x00000000000000FF;
BitBoard startingBlackALL 	= 0xFFFF000000000000;
BitBoard startingKings 		= 0x1000000000000010;
BitBoard startingQueens		= 0x0800000000000008;
BitBoard startingRooks 		= 0x8100000000000081;
BitBoard startingKnights		= 0x4200000000000042;
BitBoard startingBishops		= 0x2400000000000024;
BitBoard startingPawns 		= 0x00FF000000000000;

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

BitBoard magicNumberRook[64] = {
	0xa180022080400230, 0x40100040022000, 0x80088020001002, 0x80080280841000,
	0x4200042010460008, 0x4800a0003040080, 0x400110082041008, 0x8000a041000880,
	0x10138001a080c010, 0x804008200480, 0x10011012000c0, 0x22004128102200,
	0x200081201200c, 0x202a001048460004, 0x81000100420004, 0x4000800380004500,
	0x208002904001, 0x90004040026008, 0x208808010002001, 0x2002020020704940,
	0x8048010008110005, 0x6820808004002200, 0xa80040008023011, 0xb1460000811044, 
	0x4204400080008ea0, 0xb002400180200184, 0x2020200080100380, 0x10080080100080,
	0x2204080080800400, 0xa40080360080, 0x2040604002810b1, 0x8c218600004104,
	0x8180004000402000, 0x488c402000401001, 0x4018a00080801004, 0x1230002105001008, 
	0x8904800800800400, 0x42000c42003810, 0x8408110400b012, 0x18086182000401,
	0x2240088020c28000, 0x1001201040c004, 0xa02008010420020, 0x10003009010060,
	0x4008008008014, 0x80020004008080, 0x282020001008080, 0x50000181204a0004, 
	0x102042111804200, 0x40002010004001c0, 0x19220045508200, 0x20030010060a900, 
	0x8018028040080, 0x88240002008080, 0x10301802830400, 0x332a4081140200, 
	0x8080010a601241, 0x1008010400021, 0x4082001007241, 0x211009001200509, 
	0x8015001002441801, 0x801000804000603, 0xc0900220024a401, 0x1000200608243
};

BitBoard magicNumberBishop[64] = {
	0x2910054208004104, 0x2100630a7020180, 0x5822022042000000, 0x2ca804a100200020, 
	0x204042200000900, 0x2002121024000002, 0x80404104202000e8, 0x812a020205010840,
	0x8005181184080048, 0x1001c20208010101, 0x1001080204002100, 0x1810080489021800, 
	0x62040420010a00, 0x5028043004300020, 0xc0080a4402605002, 0x8a00a0104220200, 
	0x940000410821212, 0x1808024a280210, 0x40c0422080a0598, 0x4228020082004050, 
	0x200800400e00100, 0x20b001230021040, 0x90a0201900c00, 0x4940120a0a0108,
	0x20208050a42180, 0x1004804b280200, 0x2048020024040010, 0x102c04004010200,
	0x20408204c002010, 0x2411100020080c1, 0x102a008084042100, 0x941030000a09846, 
	0x244100800400200, 0x4000901010080696, 0x280404180020, 0x800042008240100,
	0x220008400088020, 0x4020182000904c9, 0x23010400020600, 0x41040020110302, 
	0x412101004020818, 0x8022080a09404208, 0x1401210240484800, 0x22244208010080, 
	0x1105040104000210, 0x2040088800c40081, 0x8184810252000400, 0x4004610041002200, 
	0x40201a444400810, 0x4611010802020008, 0x80000b0401040402, 0x20004821880a00, 
	0x8200002022440100, 0x9431801010068, 0x1040c20806108040, 0x804901403022a40, 
	0x2400202602104000, 0x208520209440204, 0x40c000022013020, 0x2000104000420600,
	0x400000260142410, 0x800633408100500, 0x2404080a1410, 0x138200122002900    
};

Board * BoardInit(){

	Board * b = malloc(sizeof(Board));
	
	b->WhiteAll 				= startingWhiteAll;
	b->BlackAll				= startingBlackALL;
	b->Kings 					= startingKings;
	b->Queens					= startingQueens;
	b->Rooks					= startingRooks;
	b->Knights				= startingKnights;
	b->Bishops				= startingBishops;
	b->Pawns					= startingPawns;
	
	b->Pieces[0]				= &(b->Pawns);
	b->Pieces[1]				= &(b->Bishops);
	b->Pieces[2]				= &(b->Knights);
	b->Pieces[3]				= &(b->Rooks);
	b->Pieces[4]				= &(b->Queens);
	b->Pieces[5]				= &(b->Kings);	

	b->Turn					= 0;
	b->Enpass					= 100;
	b->FiftyMoveRule			= 50;
	
	b->ValidCastles[0][0] 		= 1;
	b->ValidCastles[0][1] 		= 1;
	b->ValidCastles[1][0] 		= 1;
	b->ValidCastles[1][1] 		= 1;
	b->Castled[0] 				= 0;
	b->Castled[1] 				= 0;
	
	b->KingMap 				= generateKingMap();
	b->KnightMap 				= generateKnightMap();
	
	b->OccupancyMaskRook 		= generateOccupancyMaskRook();
	b->MagicShiftsRook			= MagicNumberShiftsRook;
	b->MagicNumberRook			= magicNumberRook;
	b->OccupancyVariationsRook	= generateOccupancyVariationRook(b->OccupancyMaskRook);
	b->MoveDatabaseRook			= generateMoveDatabaseRook(b);
	
	b->OccupancyMaskBishop 		= generateOccupancyMaskBishop();
	b->MagicShiftsBishop 		= MagicNumberShiftsBishop;
	b->MagicNumberBishop		= magicNumberBishop;
	b->OccupancyVariationsBishop	= generateOccupancyVariationBishop(b->OccupancyMaskBishop);
	b->MoveDatabaseBishop		= generateMoveDatabaseBishop(b);
	
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
	for (bit = 0, mask = 0; bit <= 63; bit++, mask = 0){
		for (i=bit+8; i<=55; i+=8)
			mask |= (1ull << i);
		for (i=bit-8; i>=8; i-=8)
			mask |= (1ull << i);
		for (i=bit+1; i%8!=7 && i%8!=0 ; i++)
			mask |= (1ull << i);
		for (i=bit-1; i%8!=7 && i%8!=0 && i>=0; i--)
			mask |= (1ull << i);
		occupancyMaskRook[bit] = mask;
	}
	
	return occupancyMaskRook;
}

BitBoard * generateOccupancyMaskBishop(){

	BitBoard * occupancyMaskBishop = malloc(sizeof(BitBoard) * 64);
	
	int i, bit;	
	BitBoard mask;
	for (bit = 0, mask = 0; bit <= 63; bit++, mask = 0){
		for (i=bit+9; i%8!=7 && i%8!=0 && i<=55; i+=9)
			mask |= (1ull << i);
		for (i=bit-9; i%8!=7 && i%8!=0 && i>=8; i-=9)
			mask |= (1ull << i);
		for (i=bit+7; i%8!=7 && i%8!=0 && i<=55; i+=7)
			mask |= (1ull << i);
		for (i=bit-7; i%8!=7 && i%8!=0 && i>=8; i-=7)
			mask |= (1ull << i);
		occupancyMaskBishop[bit] = mask;
	}
	
	return occupancyMaskBishop;
}

BitBoard ** generateOccupancyVariationRook(BitBoard * occupancyMaskRook){	

	int j, bitRef, variationCount;
	BitBoard mask, i;
	int setBitsInMask[20], setBitsInIndex[20];
	
	BitBoard ** OccupancyVariation = malloc(sizeof(BitBoard *) * 64);
	
	for(i = 0; i < 64; i++)
		OccupancyVariation[i] = malloc(sizeof(BitBoard) * 8192);
	
	for (bitRef = 0; bitRef < 64; bitRef++){
		mask = occupancyMaskRook[bitRef];
		getSetBits(mask,setBitsInMask);
		variationCount = (int)(1ull << countSetBits(mask));
		for (i = 0; i < variationCount; i++){
			OccupancyVariation[bitRef][i] = 0; 
			getSetBits(i,setBitsInIndex);
			for (j = 0; setBitsInIndex[j] != -1; j++){
				OccupancyVariation[bitRef][i] |= (1ull << setBitsInMask[setBitsInIndex[j]]);
			}
		}
	}
	
	return OccupancyVariation;
}

BitBoard ** generateOccupancyVariationBishop(BitBoard * occupancyMaskBishop){

	int j, bitRef, variationCount;
	BitBoard mask, i;
	int setBitsInMask[20], setBitsInIndex[20];
	
	BitBoard ** OccupancyVariation = malloc(sizeof(BitBoard *) * 64);
	
	for(i = 0; i < 64; i++)
		OccupancyVariation[i] = malloc(sizeof(BitBoard) * 1024);
		
	for (bitRef = 0; bitRef < 64; bitRef++){
		mask = occupancyMaskBishop[bitRef];
		getSetBits(mask,setBitsInMask);
		variationCount = (int)(1ull << countSetBits(mask));
		for (i = 0; i < variationCount; i++){
			OccupancyVariation[bitRef][i] = 0; 
			getSetBits(i,setBitsInIndex);
			for (j = 0; setBitsInIndex[j] != -1; j++)
				OccupancyVariation[bitRef][i] |= (1ull << setBitsInMask[setBitsInIndex[j]]);
		}
	}
	
	return OccupancyVariation;
}

BitBoard ** generateMoveDatabaseRook(Board * board){
	BitBoard validMoves;
	int variations, bitCount;
	int bitRef, i, j, magicIndex;

	BitBoard ** MoveDatabaseRook = malloc(sizeof(BitBoard *) * 64);
	
	for(i = 0; i < 64; i++)
		MoveDatabaseRook[i] = malloc(sizeof(BitBoard) * 8096);	
	
	for (bitRef=0; bitRef<=63; bitRef++){
		bitCount = countSetBits(board->OccupancyMaskRook[bitRef]);
		variations = (int)(1ull << bitCount);

		for (i = 0; i < variations; i++){
			validMoves = 0;
			magicIndex = (int)((board->OccupancyVariationsRook[bitRef][i] * board->MagicNumberRook[bitRef]) >> board->MagicShiftsRook[bitRef]);

			for (j=bitRef+8; j<=63; j+=8) { validMoves |= (1ull << j); if ((board->OccupancyVariationsRook[bitRef][i] & (1ull << j)) != 0) break; }
			for (j=bitRef-8; j>=0; j-=8) { validMoves |= (1ull << j); if ((board->OccupancyVariationsRook[bitRef][i] & (1ull << j)) != 0) break; }
			for (j=bitRef+1; j%8!=0; j++) { validMoves |= (1ull << j); if ((board->OccupancyVariationsRook[bitRef][i] & (1ull << j)) != 0) break; }
			for (j=bitRef-1; j%8!=7 && j>=0; j--) { validMoves |= (1ull << j); if ((board->OccupancyVariationsRook[bitRef][i] & (1ull << j)) != 0) break; }

			MoveDatabaseRook[bitRef][magicIndex] = validMoves;
		}
	}
	
	return MoveDatabaseRook;
}

BitBoard ** generateMoveDatabaseBishop(Board * board){
	BitBoard validMoves;
	int variations, bitCount;
	int bitRef, i, j, magicIndex;

	BitBoard ** MoveDatabaseBishop = malloc(sizeof(BitBoard *) * 64);
	
	for(i = 0; i < 64; i++)
		MoveDatabaseBishop[i] = malloc(sizeof(BitBoard) * 8096);	
	
	for (bitRef=0; bitRef<=63; bitRef++){
		bitCount = countSetBits(board->OccupancyMaskBishop[bitRef]);
		variations = (int)(1ull << bitCount);

		for (i = 0; i < variations; i++){
			validMoves = 0;
			magicIndex = (int)((board->OccupancyVariationsBishop[bitRef][i] * board->MagicNumberBishop[bitRef]) >> board->MagicShiftsBishop[bitRef]);

			for (j=bitRef+9; j%8!=0 && j<=63; j+=9) { validMoves |= (1ull << j); if ((board->OccupancyVariationsBishop[bitRef][i] & (1ull << j)) != 0) break; }
			for (j=bitRef-9; j%8!=7 && j>=0; j-=9) { validMoves |= (1ull << j); if ((board->OccupancyVariationsBishop[bitRef][i] & (1ull << j)) != 0) break; }
			for (j=bitRef+7; j%8!=7 && j<=63; j+=7) { 
				validMoves |= (1ull << j); 
				if ((board->OccupancyVariationsBishop[bitRef][i] & (1ull << j)) != 0) 
					break; 
			}
			for (j=bitRef-7; j%8!=0 && j>=0; j-=7) { 
				validMoves |= (1ull << j); 
				if ((board->OccupancyVariationsBishop[bitRef][i] & (1ull << j)) != 0) 
					break; 
			}

			MoveDatabaseBishop[bitRef][magicIndex] = validMoves;
		}
	}
	
	return MoveDatabaseBishop;
}


int main(){
	Board * b = BoardInit();
	time_t start = time(NULL);
	
	int index = 0;
	Move ** moves;
	moves = getAllMoves(b,&index,WHITE);
	
	int i;
	for(i = 0; i < index; i++){
		printf("Index %d ",i);
		printf("Move found from %d to %d using %d taking %d\n",moves[i]->Start,moves[i]->End,moves[i]->MovedType,moves[i]->CapturedType);
	}
		
	applyMove(b,moves[27],WHITE);
	
	printBitBoard(b->WhiteAll);
	printBitBoard(b->BlackAll);
	printBitBoard(b->Rooks);
	printBitBoard(b->Pawns);
	
}
