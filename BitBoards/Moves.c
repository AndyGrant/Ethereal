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
	BitBoard * friendly = board->Colors[turn];
	BitBoard * enemy = board->Colors[!turn];
	
	*friendly ^= (1ull << move->Start);
	*friendly |= (1ull << move->End);
	
	*(board->Pieces[move->MovedType]) ^= (1ull << move->Start);
	*(board->Pieces[move->MovedType]) |= (1ull << move->End);
	
	if (move->CapturedType != EMPTY){
		*enemy ^= (1ull << move->End);

		if (move->CapturedType != move->MovedType)4
			*(board->Pieces[move->CapturedType]) ^= (1ull << move->End);
	}
}

void RevertNormalMove(Board * board, Move * move, int turn){
	BitBoard * friendly = board->Colors[turn];
	BitBoard * enemy = board->Colors[!turn];
	
	*friendly ^= (1ull << move->End);
	*friendly |= (1ull << move->Start);
	
	*(board->Pieces[move->MovedType]) ^= (1ull << move->End);
	*(board->Pieces[move->MovedType]) |= (1ull << move->Start);
	
	if (move->CapturedType != EMPTY){
		*enemy |= (1ull << move->End);
		
		if (move->CapturedType != move->MovedType)
			*(board->Pieces[move->CapturedType]) |= (1ull << move->End);
	}	
}

void ApplyPawnDoublePushMove(Board * board, Move * move, int turn){
	BitBoard * friendly = board->Colors[turn];
	
	*friendly ^= (1ull << move->Start);
	*friendly |= (1ull << move->End);
	
	board->Pawns ^= (1ull << move->Start);
	board->Pawns |= (1ull << move->End);
}

void RevertPawnDoublePushMove(Board * board, Move * move, int turn){
	BitBoard * friendly = board->Colors[turn];
	
	*friendly ^= (1ull << move->End);
	*friendly |= (1ull << move->Start);
	
	board->Pawns ^= (1ull << move->End);
	board->Pawns |= (1ull << move->Start);
}
	