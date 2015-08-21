#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Engine.h"

/*
	Move Encodings

	NormalMove: [Move Type, Start, End, Captured Type, Breaks Castles]
	
	CastleMove: [Move Type, Start, End, Rook Start, Rook End]
	
	PromotionMove: [Move Type, Start, End, Captured Type, Promte Type]
	
	EnpassMove: [Move Type, Start, End, Enpass]

*/

char base[135] = "31112141512111310101010101010101999999999999999999999999999999999999999999999999999999999999999900000000000000003010204050201030000000";

int MOVES_BUFFER[BUFFER_SIZE];
int EVALUTE_CHECK[8][8];

int * TYPES;
int * COLORS;

void (*GetPieceMoves[6])(Board *, int, int *, int, int, int) = {
	&getPawnMoves,
	&getKnightMoves,
	&getBishopMoves,
	&getRookMoves,
	&getQueenMoves,
	&getKingMoves
};

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

int main(){
	Board * board = createBoard(base);
	int move[5] = {0,52,52-16,9,0};
	printf("%d ",COLORS[52-16]);
	(*ApplyTypes[0])(board,move);
	printf("%d ",COLORS[52-16]);
	(*RevertTypes[0])(board,move);
	printf("%d ",COLORS[52-16]);
	
	return 1;
}

Board * createBoard(char setup[135]){
	// Setup Type|Color x64 Castlex2 ValidLRx2
	int x,y,i;
	Board * board = malloc(sizeof(Board));
	
	for(x = 0, i = 0; x < 8; x++){
		for(y = 0; y < 8; y++){
			board->Types[x][y] = setup[i++] - '0';
			board->Colors[x][y] = setup[i++] - '0';
			
			if (board->Types[x][y] == KING)
				board->KingLocations[board->Colors[x][y]] = x*8 + y;
		}
	}
	
	board->Castled[0] = setup[i++] - '0';
	board->Castled[1] = setup[i++] - '0';
	board->ValidCastles[0][0] = setup[i++] - '0';
	board->ValidCastles[0][1] = setup[i++] - '0';
	board->ValidCastles[1][0] = setup[i++] - '0';
	board->ValidCastles[1][1] = setup[i++] - '0';
	
	board->FiftyMoveRule = 50;
	board->PreviousMovesSize = 0;
	
	TYPES = *(board->Types);
	COLORS = *(board->Colors);
	
	return board;
}

int * getAllMoves(Board * board, int turn, int * size){
	int x,y,i;
	int * color = &(board->Colors[0][0]);
	int * type = &(board->Types[0][0]);
}

void getPawnMoves(Board * board, int turn, int * size, int x, int y, int check){
	
}

void getKnightMoves(Board * board, int turn, int * size, int x, int y, int check){
	
}

void getBishopMoves(Board * board, int turn, int * size, int x, int y, int check){
	
}

void getRookMoves(Board * board, int turn, int * size, int x, int y, int check){
	
}

void getQueenMoves(Board * board, int turn, int * size, int x, int y, int check){
	
}

void getKingMoves(Board * board, int turn, int * size, int x, int y, int check){
	
}

int validateMove(Board * board, int turn){
	return 1;
}

void createNormalMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyNormalMove(board,move);
		if (validateMove(board,turn))
			memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertNormalMove(board,move);	
	} else
		memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createCastleMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyCastleMove(board,move);
		if (validateMove(board,turn))
			memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertCastleMove(board,move);	
	} else
		memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createPromotionMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyPromotionMove(board,move);
		if (validateMove(board,turn))
			memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertPromotionMove(board,move);	
	} else
		memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createEnpassMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyEnpassMove(board,move);
		if (validateMove(board,turn))
			memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertEnpassMove(board,move);	
	} else
		memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void applyNormalMove(Board * board, int * move){
	TYPES[move[2]] = TYPES[move[1]];
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	if(TYPES[move[2]] == KING)
		board->KingLocations[COLORS[move[2]]] = move[2];
		
	if(move[4]){
		if(move[4] == 1)
			board->ValidCastles[COLORS[move[2]]][0] = 0;
		if(move[4] == 2)
			board->ValidCastles[COLORS[move[2]]][1] = 0;
	}
}

void applyCastleMove(Board * board, int * move){
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
	TYPES[move[2]] = move[4];
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
}

void applyEnpassMove(Board * board, int * move){
	TYPES[move[2]] = PAWN;
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	TYPES[move[3]] = EMPTY;
	COLORS[move[3]] = EMPTY;

}

void revertNormalMove(Board * board, int * move){
	TYPES[move[1]] = TYPES[move[2]];
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = move[3];
	COLORS[move[2]] = move[3] != EMPTY ? !COLORS[move[1]] : EMPTY;
	
	if(TYPES[move[1]] == KING)
		board->KingLocations[COLORS[move[1]]] = move[1];
		
	if(move[4]){
		if(move[4] == 1)
			board->ValidCastles[COLORS[move[1]]][0] = 1;
		if(move[4] == 2)
			board->ValidCastles[COLORS[move[1]]][1] = 1;
	}
}

void revertCastleMove(Board * board, int * move){
	TYPES[move[1]] = KING;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = EMPTY;
	COLORS[move[2]] = EMPTY;
	
	TYPES[move[3]] = ROOK;
	COLORS[move[3]] = COLORS[move[4]];
	
	TYPES[move[4]] = EMPTY;
	COLORS[move[4]] = EMPTY;
	
	board->KingLocations[COLORS[move[2]]] = move[1];
	board->Castled[COLORS[move[2]]] = 0;
}

void revertPromotionMove(Board * board, int * move){
	TYPES[move[1]] = PAWN;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = move[3];
	COLORS[move[2]] = move[3] != EMPTY ? !COLORS[move[1]] : EMPTY;
}

void revertEnpassMove(Board * board, int * move){
	TYPES[move[1]] = PAWN;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = EMPTY;
	COLORS[move[2]] = EMPTY;
	
	TYPES[move[3]] = PAWN;
	COLORS[move[3]] = !COLORS[move[1]];
}
