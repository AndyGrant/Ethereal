#include <stdio.h>
#include <stdlib.h>

#include "Types.h"
#include "Board.h"
#include "BitTools.h"
#include "Engine.h"

Move ** getAllMoves(Board * board, int * index, int turn){
	Move ** moves = malloc(sizeof(Move * ) * 256);
	
	getKingMoves(board, moves, index, turn);
	getQueenMoves(board, moves, index, turn);
	getRookMoves(board, moves, index, turn);
	getKnightMoves(board, moves, index, turn);
	getBishopMoves(board, moves, index, turn);
	getPawnMoves(board, moves, index, turn);
	
	return moves;
}

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
			move->Flags = -1;
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
		
		if(board->ValidCastles[turn][0]){
			if(board->ValidCastles[turn][1])
				move->Flags = 72;
			else
				move->Flags = 70;
		} else if(board->ValidCastles[turn][1]){
			move->Flags = 71;
		}
		
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

void getRookMoves(Board * board, Move ** moves, int * index, int turn){

	BitBoard friendly, enemy;
	if (turn == WHITE){
		friendly = board->WhiteAll;
		enemy = board->BlackAll;
	}
	else if (turn == BLACK){
		friendly = board->BlackAll;
		enemy = board->WhiteAll;
	}
	
	BitBoard friendlyRooks = board->Rooks & friendly;
	while(friendlyRooks != 0){
		int bit = getLSB(friendlyRooks);
		
		BitBoard bbBlockers = (board->WhiteAll | board->BlackAll) & board->OccupancyMaskRook[bit];
		int dbIndex = (int)((bbBlockers * board->MagicNumberRook[bit]) >> board->MagicShiftsRook[bit]);
		BitBoard attackable = board->MoveDatabaseRook[bit][dbIndex] & ~friendly;
		
		while (attackable != 0){
			int lsb = getLSB(attackable);
			Move * move = malloc(sizeof(Move));
			move->Turn = turn;
			move->Start = bit;
			move->End = lsb;
			move->FriendlyBefore = friendly;
			move->EnemyBefore = enemy;
			move->CapturedType = getSquare(lsb,board);
			move->MovedType = ROOK;
			
			if(board->ValidCastles[turn][0] && (bit ==  0 || bit == 56))
				move->Flags = 70;
				
			else if(board->ValidCastles[turn][1] && (bit == 7 || bit == 63))
				move->Flags = 71;
				
			moves[(*index)++] = move;
			attackable ^= 1ull << lsb;
		}
		
		friendlyRooks ^= 1ull << bit;
	}
}

void getBishopMoves(Board * board, Move ** moves, int * index, int turn){

	BitBoard friendly, enemy;
	if (turn == WHITE){
		friendly = board->WhiteAll;
		enemy = board->BlackAll;
	}
	else if (turn == BLACK){
		friendly = board->BlackAll;
		enemy = board->WhiteAll;
	}
	
	BitBoard friendlyBishops = board->Bishops & friendly;
	while(friendlyBishops != 0){
		int bit = getLSB(friendlyBishops);
		
		BitBoard bbBlockers = (board->WhiteAll | board->BlackAll) & board->OccupancyMaskBishop[bit];
		int dbIndex = (int)((bbBlockers * board->MagicNumberBishop[bit]) >> board->MagicShiftsBishop[bit]);
		BitBoard attackable = board->MoveDatabaseBishop[bit][dbIndex] & ~friendly;
		
		while (attackable != 0){
			int lsb = getLSB(attackable);
			Move * move = malloc(sizeof(Move));
			move->Turn = turn;
			move->Start = bit;
			move->End = lsb;
			move->FriendlyBefore = friendly;
			move->EnemyBefore = enemy;
			move->CapturedType = getSquare(lsb,board);
			move->MovedType = BISHOP;
			moves[(*index)++] = move;
			attackable ^= 1ull << lsb;
		}
		
		friendlyBishops ^= 1ull << bit;
	}
}

void getQueenMoves(Board * board, Move ** moves, int * index, int turn){
	BitBoard friendly, enemy;
	if (turn == WHITE){
		friendly = board->WhiteAll;
		enemy = board->BlackAll;
	}
	else if (turn == BLACK){
		friendly = board->BlackAll;
		enemy = board->WhiteAll;
	}
	
	BitBoard friendlyQueens = board->Queens & friendly;
	while(friendlyQueens != 0){
		int bit = getLSB(friendlyQueens);
		
		BitBoard bbBlockers = (board->WhiteAll | board->BlackAll) & board->OccupancyMaskRook[bit];
		int dbIndex = (int)((bbBlockers * board->MagicNumberRook[bit]) >> board->MagicShiftsRook[bit]);
		BitBoard attackable = board->MoveDatabaseRook[bit][dbIndex] & ~friendly;
		
		while (attackable != 0){
			int lsb = getLSB(attackable);
			Move * move = malloc(sizeof(Move));
			move->Turn = turn;
			move->Start = bit;
			move->End = lsb;
			move->FriendlyBefore = friendly;
			move->EnemyBefore = enemy;
			move->CapturedType = getSquare(lsb,board);
			move->MovedType = QUEEN;
			moves[(*index)++] = move;
			attackable ^= 1ull << lsb;
		}
		
		bbBlockers = (board->WhiteAll | board->BlackAll) & board->OccupancyMaskBishop[bit];
		dbIndex = (int)((bbBlockers * board->MagicNumberBishop[bit]) >> board->MagicShiftsBishop[bit]);
		attackable = board->MoveDatabaseBishop[bit][dbIndex] & ~friendly;
		
		while (attackable != 0){
			int lsb = getLSB(attackable);
			Move * move = malloc(sizeof(Move));
			move->Turn = turn;
			move->Start = bit;
			move->End = lsb;
			move->FriendlyBefore = friendly;
			move->EnemyBefore = enemy;
			move->CapturedType = getSquare(lsb,board);
			move->MovedType = QUEEN;
			moves[(*index)++] = move;
			attackable ^= 1ull << lsb;
		}
		
		friendlyQueens ^= 1ull << bit;
	}
}

void applyMove(Board * board, Move * move, int turn){
	BitBoard * friendly;
	BitBoard * enemy;
	
	if (turn == WHITE){
		friendly = &(board->WhiteAll);
		enemy = &(board->BlackAll);
	}
	else if (turn == BLACK){
		friendly = &(board->BlackAll);
		enemy = &(board->WhiteAll);
	}
	
	
	if (move->Flags <= 64 || (move->Flags >= 70 && move->Flags <= 72)){
		*friendly ^= (1ull << move->Start);
		*friendly |= (1ull << move->End);
		
		*(board->Pieces[move->MovedType]) ^= (1ull << move->Start);
		*(board->Pieces[move->MovedType]) |= (1ull << move->End);
		
		if (move->CapturedType != EMPTY){
			*enemy ^= (1ull << move->End);
			if (move->CapturedType != move->MovedType)
				*(board->Pieces[move->CapturedType]) ^= (1ull << move->End);
		}
	}
	
	if (move->Flags >= 65 && move->Flags <= 68){
		*friendly ^= (1ull << move->Start);
		*friendly |= (1ull << move->End);
		
		*(board->Pieces[PAWN]) ^= (1ull << move->Start);
		*(board->Pieces[69 - move->Flags]) |= (1ull << move->End);
		
		if (move->CapturedType != EMPTY){
			*enemy ^= (1ull << move->End);
			if (move->CapturedType != move->MovedType)
				*(board->Pieces[move->CapturedType]) ^= (1ull << move->End);
		}
	}
	
	if (move->Flags ==  80 || move->Flags == 81){
		int start = turn == WHITE ? 4 : 60;
	}

	
	if (move->Flags == 70)
		board->ValidCastles[turn][0] = 0;
	
	if (move->Flags == 71)
		board->ValidCastles[turn][1] = 0;
		
	if (move->Flags == 72){
		board->ValidCastles[turn][0] = 0;
		board->ValidCastles[turn][1] = 0;
	}
}