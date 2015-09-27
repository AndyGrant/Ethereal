#include <stdio.h>
#include <stdlib.h>

#include "Evaluate.h"
#include "Board.h"
#include "MoveGen.h"
#include "Search.h"

int VALUE_KNIGHT_RANGE = 2;
int VALUE_BISHOP_RANGE = 2;
int VALUE_CENTER_ATTACKED = 8;
int VALUE_CENTRAL_KNIGHT = 15;
int VALUE_KING_SURROUNDINGS_ATTACKED = 5;
int VALUE_CASTLED = 100;
int VALUE_ABLE_TO_CASTLE = 25;


int evaluateBoard(Board * board, int turn){
	TOTAL_BOARDS_SEARCHED += 1;
	return 	evaluateMaterial(board, turn) + 
          evaluatePosition(board, turn) +
          evaluateMoves(board,turn)	-
          evaluateMoves(board,!turn);
}

int evaluateMaterial(Board * board, int turn){
	int i, value = 0;
	for(i = 0; i < 64; i++){
		if (board->TYPES[i] != EMPTY){
			if (board->COLORS[i] == turn)
				value += MATERIAL_VALUES[board->TYPES[i]];
			else
				value -= MATERIAL_VALUES[board->TYPES[i]];
		}
	}
	return value;
}

int evaluatePosition(Board * board, int turn){
	int x,y, value = 0;
	for(x = 3; x < 5; x++)
		for(y = 3; y < 5; y++)
			if (board->Types[x][y] == KNIGHT)
				value += board->Colors[x][y] == turn ? VALUE_CENTRAL_KNIGHT : -VALUE_CENTRAL_KNIGHT;
	
	for(x = 3; x < 5; x++)
		for(y = 3; y < 5; y++)
			if (board->Types[x][y] != EMPTY)
				value += board->Colors[x][y] == turn ? VALUE_CENTER_ATTACKED : -VALUE_CENTER_ATTACKED;
				
	int whitePawnStart = 6;
	int blackPawnStart = 1;
	
	for(x = 1; x < 7; x++)
		for(y = 0; y < 8; y++)
			if (board->Types[x][y] == PAWN)
				value += board->Colors[x][y] == turn ? PAWN_SCORE_MAP[turn][x] : -PAWN_SCORE_MAP[!turn][x];
				//value += board->Colors[x][y] == turn ? (whitePawnStart - x) : -(x - blackPawnStart);
				
	if (board->Castled[turn])
		value += VALUE_CASTLED;
	if (board->Castled[!turn])
		value -= VALUE_CASTLED;
	if (board->ValidCastles[turn][0] || board->ValidCastles[turn][1])
		value += VALUE_ABLE_TO_CASTLE;
	if (board->ValidCastles[!turn][0] || board->ValidCastles[!turn][1])
		value -= VALUE_ABLE_TO_CASTLE;

	return value;
}

int evaluateMoves(Board * board, int turn){
	int size = 0;
	int * moves = getAllMoves(board,turn,&size);
	int * moves_p = moves;
	
	TOTAL_MOVES_FOUND += size;
	
	float nv = 0;
	int i, value=0;
	for(i = 0; i < size; i++, moves += 5){
		if (board->TYPES[moves[1]] == BISHOP)
			value += VALUE_BISHOP_RANGE;
		else if (board->TYPES[moves[1]] == KNIGHT)
			value += VALUE_KNIGHT_RANGE;
		if (moves[2] / 8 > 2 && moves[2] / 8 < 5 && moves[2] % 8 > 2 && moves[2] % 8 < 5)
			value += VALUE_CENTER_ATTACKED;
		if (moves[0] == 0 && moves[3] != 9)
			nv += CAPTURE_VALUES[board->TYPES[moves[2]]] / CAPTURE_VALUES[board->TYPES[moves[1]]];	
	}
	
	free(moves_p);
	return value + (int)(10*nv);
}

