#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Move.h"
#include "MoveGen.h"
#include "Board.h"

void (*ApplyTypes[5])(Board *, int *) = {
	&applyNormalMove,
	&applyCastleMove,
	&applyPromotionMove,
	&applyEnpassMove,
	&applyNormalMove
};

void (*RevertTypes[5])(Board *, int *) = {
	&revertNormalMove,
	&revertCastleMove,
	&revertPromotionMove,
	&revertEnpassMove,
	&revertNormalMove
};

void createNormalMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyNormalMove(board,move);
		if (validateMove(board,turn))
			memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertNormalMove(board,move);	
	} else
		memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createCastleMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyCastleMove(board,move);
		if (validateMove(board,turn))
			memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertCastleMove(board,move);	
	} else
		memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createPromotionMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyPromotionMove(board,move);
		if (validateMove(board,turn))
			memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertPromotionMove(board,move);	
	} else
		memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createEnpassMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyEnpassMove(board,move);
		if (validateMove(board,turn))
			memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertEnpassMove(board,move);	
	} else
		memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void applyNormalMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	
	if (move[3] != EMPTY)
		board->PieceCount[!(COLORS[move[1]])][move[3]] -= 1;
	
	TYPES[move[2]] = TYPES[move[1]];
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	if(TYPES[move[2]] == KING)
		board->KingLocations[COLORS[move[2]]] = move[2];
		
	if(move[4]){
		if(move[4] <= 2)
			board->ValidCastles[COLORS[move[2]]][0] = 0;
		if(move[4] >= 2)
			board->ValidCastles[COLORS[move[2]]][1] = 0;
	}
}

void applyCastleMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	TYPES[move[2]] = KING;
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	TYPES[move[4]] = ROOK;
	COLORS[move[4]] = COLORS[move[3]];
	
	TYPES[move[3]] = EMPTY;
	COLORS[move[3]] = EMPTY;
	
	board->KingLocations[COLORS[move[2]]] = move[2];
	board->Castled[COLORS[move[2]]] = 1;
}

void applyPromotionMove(Board * board, int * move){	
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	if (move[3] != EMPTY)
		board->PieceCount[!(COLORS[move[1]])][move[3]] -= 1;
	board->PieceCount[COLORS[move[1]]][move[4]] += 1;
	board->PieceCount[COLORS[move[1]]][PAWN] -= 1;
	
	TYPES[move[2]] = move[4];
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
}

void applyEnpassMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	board->PieceCount[!(COLORS[move[1]])][PAWN] -= 1;
	
	TYPES[move[2]] = PAWN;
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	TYPES[move[3]] = EMPTY;
	COLORS[move[3]] = EMPTY;

}

void revertNormalMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	if (move[3] != EMPTY)
		board->PieceCount[!(COLORS[move[2]])][move[3]] += 1;
	
	TYPES[move[1]] = TYPES[move[2]];
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = move[3];
	COLORS[move[2]] = move[3] != EMPTY ? !COLORS[move[1]] : EMPTY;
	
	if(TYPES[move[1]] == KING)
		board->KingLocations[COLORS[move[1]]] = move[1];
		
	if(move[4]){
		if(move[4] <= 2)
			board->ValidCastles[COLORS[move[1]]][0] = 1;
		if(move[4] >= 2)
			board->ValidCastles[COLORS[move[1]]][1] = 1;
	}
}

void revertCastleMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	TYPES[move[1]] = KING;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = EMPTY;
	COLORS[move[2]] = EMPTY;
	
	TYPES[move[3]] = ROOK;
	COLORS[move[3]] = COLORS[move[4]];
	
	TYPES[move[4]] = EMPTY;
	COLORS[move[4]] = EMPTY;
	
	board->KingLocations[COLORS[move[1]]] = move[1];
	board->Castled[COLORS[move[1]]] = 0;
}

void revertPromotionMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	if (move[3] != EMPTY)
		board->PieceCount[!(COLORS[move[2]])][move[3]] += 1;
	board->PieceCount[(COLORS[move[2]])][move[4]] -= 1;
	board->PieceCount[(COLORS[move[2]])][PAWN] += 1;
	
	TYPES[move[1]] = PAWN;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = move[3];
	COLORS[move[2]] = move[3] != EMPTY ? !COLORS[move[1]] : EMPTY;
}

void revertEnpassMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	board->PieceCount[!(COLORS[move[2]])][PAWN] += 1;
	
	TYPES[move[1]] = PAWN;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = EMPTY;
	COLORS[move[2]] = EMPTY;
	
	TYPES[move[3]] = PAWN;
	COLORS[move[3]] = !COLORS[move[1]];
}
