#include <stdio.h>
#include <stdlib.h>

#include "Types.h"
#include "Board.h"
#include "Engine.h"
#include "Moves.h"

int NormalMove	= 1;
int PawnDoublePushMove = 2;
int BreaksLeftCastleMove = 3;
int BreaksRightCastleMove = 4;
int BreaksBothCastlesMove = 5;
int LeftCastleMove = 6;
int RightCastleMove = 7;
int PromoteBishopMove = 8;
int PromoteKnightMove = 9;
int PromoteRookMove = 10;
int PromoteQueenMove = 11;
int EnpassLeftMove = 12;
int EnpassRightMove = 13;

void ApplyNormalMove(Board * board, Move * move, int turn){
	BitBoard friendly = board->Colors[turn];
	BitBoard enemy = board->Colors[!turn];
	
	friendly ^= (1ull << move->start);
	friendly |= (1ull << move->end);
	
	board->Pieces[move->MovedType] ^= (1ull << move->start);
	board->Pieces[move->MovedType] |= (1ull << move->end);
	
	
}