#include <stdio.h>
#include <stdlib.h>

#include "Types.h"
#include "Board.h"
#include "Engine.h"
#include "Moves.h"

int NormalMove	= 0;
int PawnDoublePushMove = 1;
int BreaksLeftCastleMove = 2;
int BreaksRightCastleMove = 3;
int BreaksBothCastlesMove = 4;
int LeftCastleMove = 5;
int RightCastleMove = 6;
int PromoteBishopMove = 7;
int PromoteKnightMove = 8;
int PromoteRookMove = 9;
int PromoteQueenMove = 10;
int EnpassLeftMove = 11;
int EnpassRightMove = 12;

void (*ApplyTypes[7])(Board *, Move *, int) = {
	&ApplyNormalMove,
	&ApplyPawnDoublePushMove,
	&ApplyBreaksLeftCastleMove,
	&ApplyBreaksRightCastleMove,
	&ApplyBreaksBothCastlesMove,
	&ApplyLeftCastleMove,
	&ApplyRightCastleMove
};

void (*RevertTypes[7])(Board *, Move *, int) = {
	&RevertNormalMove,
	&RevertPawnDoublePushMove,
	&RevertBreaksLeftCastleMove,
	&RevertBreaksRightCastleMove,
	&RevertBreaksBothCastlesMove,
	&RevertLeftCastleMove,
	&RevertRightCastleMove
};

void ApplyNormalMove(Board * board, Move * move, int turn){
	BitBoard * friendly = board->Colors[turn];
	BitBoard * enemy = board->Colors[!turn];
	BitBoard * moved = board->Pieces[move->MovedType];
	BitBoard * captured = board->Pieces[move->CapturedType];
	
	
	*friendly ^= (1ull << move->Start);
	*friendly |= (1ull << move->End);
	
	if(move->CapturedType != EMPTY)
		*enemy ^= (1ull << move->End);
		
	*captured ^= (1ull << move->End);
	*moved |= (1ull << move->End);
	*moved ^= (1ull << move->Start);
}

void RevertNormalMove(Board * board, Move * move, int turn){
	BitBoard * friendly = board->Colors[turn];
	BitBoard * enemy = board->Colors[!turn];
	BitBoard * moved = board->Pieces[move->MovedType];
	BitBoard * captured = board->Pieces[move->CapturedType];
	
	*friendly |= (1ull << move->Start);
	*friendly ^= (1ull << move->End);
	
	if(move->CapturedType != EMPTY)
		*enemy |= (1ull << move->End);
		
	*moved |= (1ull << move->Start);
	*moved ^= (1ull << move->End);
	*captured |= (1ull << move->End);
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

void ApplyBreaksLeftCastleMove(Board * board, Move * move, int turn){
	ApplyNormalMove(board,move,turn);
	board->ValidCastles[turn][0] = 0;
}

void RevertBreaksLeftCastleMove(Board * board, Move * move, int turn){
	RevertNormalMove(board,move,turn);
	board->ValidCastles[turn][0] = 1;
}

void ApplyBreaksRightCastleMove(Board * board, Move * move, int turn){
	ApplyNormalMove(board,move,turn);
	board->ValidCastles[turn][1] = 0;
}

void RevertBreaksRightCastleMove(Board * board, Move * move, int turn){
	RevertNormalMove(board,move,turn);
	board->ValidCastles[turn][1] = 1;
}

void ApplyBreaksBothCastlesMove(Board * board, Move * move, int turn){
	ApplyNormalMove(board,move,turn);
	board->ValidCastles[turn][0] = 0;
	board->ValidCastles[turn][1] = 0;
}

void RevertBreaksBothCastlesMove(Board * board, Move * move, int turn){
	RevertNormalMove(board,move,turn);
	board->ValidCastles[turn][0] = 1;
	board->ValidCastles[turn][1] = 1;
}

void ApplyLeftCastleMove(Board * board, Move * move, int turn){
	BitBoard * friendly = board->Colors[turn];
	int kingStart = turn == WHITE ? 4 : 60;
	int rookStart = turn == WHITE ? 0 : 56;
	int kingEnd = kingStart - 2;
	int rookEnd = kingStart - 1;
	
	*friendly ^= (1ull << kingStart);
	*friendly ^= (1ull << rookStart);
	
	*friendly |= (1ull << kingEnd);
	*friendly |= (1ull << rookEnd);
	
	*(board->Pieces[KING]) ^= (1ull << kingStart);
	*(board->Pieces[ROOK]) ^= (1ull << rookStart);
	
	*(board->Pieces[KING]) |= (1ull << kingEnd);
	*(board->Pieces[ROOK]) |= (1ull << rookEnd);
	
	board->Castled[turn] = 1;	
}

void RevertLeftCastleMove(Board * board, Move * move, int turn){
	BitBoard * friendly = board->Colors[turn];
	int kingStart = turn == WHITE ? 4 : 60;
	int rookStart = turn == WHITE ? 0 : 56;
	int kingEnd = kingStart - 2;
	int rookEnd = kingStart - 1;
	
	*friendly |= (1ull << kingStart);
	*friendly |= (1ull << rookStart);
	
	*friendly ^= (1ull << kingEnd);
	*friendly ^= (1ull << rookEnd);
	
	*(board->Pieces[KING]) |= (1ull << kingStart);
	*(board->Pieces[ROOK]) |= (1ull << rookStart);
	
	*(board->Pieces[KING]) ^= (1ull << kingEnd);
	*(board->Pieces[ROOK]) ^= (1ull << rookEnd);
	
	board->Castled[turn] = 0;	
}

void ApplyRightCastleMove(Board * board, Move * move, int turn){
	BitBoard * friendly = board->Colors[turn];
	int kingStart = turn == WHITE ? 4 : 60;
	int rookStart = turn == WHITE ? 7 : 63;
	int kingEnd = kingStart + 2;
	int rookEnd = kingStart + 1;
	
	*friendly ^= (1ull << kingStart);
	*friendly ^= (1ull << rookStart);
	
	*friendly |= (1ull << kingEnd);
	*friendly |= (1ull << rookEnd);
	
	*(board->Pieces[KING]) ^= (1ull << kingStart);
	*(board->Pieces[ROOK]) ^= (1ull << rookStart);
	
	*(board->Pieces[KING]) |= (1ull << kingEnd);
	*(board->Pieces[ROOK]) |= (1ull << rookEnd);
	
	board->Castled[turn] = 1;	
}

void RevertRightCastleMove(Board * board, Move * move, int turn){
	BitBoard * friendly = board->Colors[turn];
	int kingStart = turn == WHITE ? 4 : 60;
	int rookStart = turn == WHITE ? 7 : 63;
	int kingEnd = kingStart + 2;
	int rookEnd = kingStart + 1;
	
	*friendly |= (1ull << kingStart);
	*friendly |= (1ull << rookStart);
	
	*friendly ^= (1ull << kingEnd);
	*friendly ^= (1ull << rookEnd);
	
	*(board->Pieces[KING]) |= (1ull << kingStart);
	*(board->Pieces[ROOK]) |= (1ull << rookStart);
	
	*(board->Pieces[KING]) ^= (1ull << kingEnd);
	*(board->Pieces[ROOK]) ^= (1ull << rookEnd);
	
	board->Castled[turn] = 0;	
}

	