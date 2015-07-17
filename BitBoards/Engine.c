#include <stdio.h>
#include <stdlib.h>

#include "Types.h"
#include "Board.h"
#include "BitTools.h"
#include "Engine.h"

void getKnightMoves(Board * board, Move ** moves, int * index, int turn){

	BitBoard friendly, enemy;
	if (turn == WHITE){
		friendly = board->WhiteAll;
		enemy = board->BlackAll;
	}
	else if (turn == BLACK){
		friendly = board->BlackAll;
		enemy = board->WhiteAll;
	}
	
	BitBoard friendlyKnights = board->Knights & friendly;
	while(friendlyKnights != 0){
		int bit = getLSB(friendlyKnights);
		BitBoard attackable = board->KnightMap[bit] & ~friendly;
		
		while (attackable != 0){
			int lsb = getLSB(attackable);
			Move * move = malloc(sizeof(Move));
			move->Turn = turn;
			move->Start = bit;
			move->End = lsb;
			move->FriendlyBefore = friendly;
			move->EnemyBefore = enemy;
			move->CapturedType = getSquare(lsb,board);
			move->MovedType = KNIGHT;
			moves[(*index)++] = move;
			attackable ^= 1ull << lsb;
		}
		
		friendlyKnights ^= 1ull << bit; 	
	}
}

void getKingMoves(Board * board, Move ** moves, int * index, int turn){
	
	BitBoard friendly, enemy;
	if (turn == WHITE){
		friendly = board->WhiteAll;
		enemy = board->BlackAll;
	}
	else if (turn == BLACK){
		friendly = board->BlackAll;
		enemy = board->WhiteAll;
	}
	
	int bit = getLSB(board->Kings & friendly);
	BitBoard attackable = board->KingMap[bit] & ~friendly;
	
	while (attackable != 0){
		int lsb = getLSB(attackable);
		Move * move = malloc(sizeof(Move));
		move->Turn = turn;
		move->Start = bit;
		move->End = lsb;
		move->FriendlyBefore = friendly;
		move->EnemyBefore = enemy;
		move->CapturedType = getSquare(lsb,board);
		move->MovedType = KING;
		moves[(*index)++] = move;
		attackable ^= 1ull << lsb;
	}	
}

void getPawnMoves(Board * board, Move ** moves, int * index, int turn){

	int forwardShift, leftShift, rightShift;
	BitBoard friendly, enemy;
	BitBoard pawns, oneMove, twoMove, leftMove, rightMove;
	
	if (turn == WHITE){
		friendly = board->WhiteAll;
		enemy = board->BlackAll;
		forwardShift = 8;
		leftShift = 7;
		rightShift = 9;
		
	}
	else if (turn == BLACK){
		friendly = board->BlackAll;
		enemy = board->WhiteAll;
		forwardShift = -8;
		leftShift = -7;
		rightShift = -9;
	}
	
	pawns = board->Pawns & friendly;
	
	if (turn == WHITE){
		oneMove = (pawns << 8) & ~(friendly | enemy);
		twoMove = ((oneMove & RANK_3) << 8) & ~(friendly | enemy);
		leftMove = ((pawns << 7) & ~(FILE_A)) & enemy;
		rightMove = ((pawns << 9) & ~(FILE_H)) & enemy;
	}
	else if (turn == BLACK){
		oneMove = (pawns >> 8) & ~(friendly | enemy);
		twoMove = ((oneMove & RANK_6) >> 8) & ~(friendly | enemy);
		leftMove = ((pawns >> 7) & ~(FILE_H)) & enemy;
		rightMove = ((pawns >> 9) & ~(FILE_A)) & enemy;
	}
	
	while (oneMove != 0){
		int lsb = getLSB(oneMove);
		Move * move = malloc(sizeof(Move));
		move->Turn = turn;
		move->Start = lsb - forwardShift;
		move->End = lsb;
		move->FriendlyBefore = friendly;
		move->EnemyBefore = enemy;
		move->CapturedType = EMPTY;
		move->MovedType = PAWN;
		moves[(*index)++] = move;
		oneMove ^= 1ull << lsb;
	}
	
	while (twoMove != 0){
		int lsb = getLSB(twoMove);
		Move * move = malloc(sizeof(Move));
		move->Turn = turn;
		move->Start = lsb - (2 * forwardShift);
		move->End = lsb;
		move->FriendlyBefore = friendly;
		move->EnemyBefore = enemy;
		move->CapturedType = EMPTY;
		move->MovedType = PAWN;
		moves[(*index)++] = move;
		twoMove ^= 1ull << lsb;
	}
	
	while (leftMove != 0){
		int lsb = getLSB(leftMove);
		Move * move = malloc(sizeof(Move));
		move->Turn = turn;
		move->Start = lsb - leftShift;
		move->End = lsb;
		move->FriendlyBefore = friendly;
		move->EnemyBefore = enemy;
		move->CapturedType = getSquare(lsb,board);
		move->MovedType = PAWN;
		moves[(*index)++] = move;
		leftMove ^= 1ull << lsb;
	}
	
	while (rightMove != 0){
		int lsb = getLSB(rightMove);
		Move * move = malloc(sizeof(Move));
		move->Turn = turn;
		move->Start = lsb - rightShift;
		move->End = lsb;
		move->FriendlyBefore = friendly;
		move->EnemyBefore = enemy;
		move->CapturedType = getSquare(lsb,board);
		move->MovedType = PAWN;
		moves[(*index)++] = move;
		rightMove ^= 1ull << lsb;
	}
}




