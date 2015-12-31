#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#include "board.h"
#include "colour.h"
#include "evaluate.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "types.h"
#include "util.h"

int evaluate_board(board_t * board){
	int value = 60;
	int turn = board->turn;
	int * location;
	
	for(location = &(board->piece_locations[turn][1]); *location != -1; location++){
		switch(PIECE_TYPE(board->squares[*location])){
			case QueenFlag: 	value += (QueenValue 	+ QUEEN_POSITION_VALUE(*location)); 	break;
			case RookFlag: 		value += (RookValue 	+ ROOK_POSITION_VALUE(*location)); 		break;
			case BishopFlag: 	value += (BishopValue 	+ BISHOP_POSITION_VALUE(*location)); 	break;
			case KnightFlag: 	value += (KnightValue 	+ KNIGHT_POSITION_VALUE(*location)); 	break;
		}
	}
	
	for(location = &(board->piece_locations[!turn][1]); *location != -1; location++){
		switch(PIECE_TYPE(board->squares[*location])){
			case QueenFlag: 	value -= (QueenValue 	+ QUEEN_POSITION_VALUE(*location)); 	break;
			case RookFlag: 		value -= (RookValue 	+ ROOK_POSITION_VALUE(*location)); 		break;
			case BishopFlag: 	value -= (BishopValue 	+ BISHOP_POSITION_VALUE(*location)); 	break;
			case KnightFlag: 	value -= (KnightValue 	+ KNIGHT_POSITION_VALUE(*location)); 	break;
		}
	}
	
	
	value += PawnValue * (board->pawn_counts[turn] - board->pawn_counts[!turn]);
	
	int pawn_delta = turn == ColourWhite ? -16 : 16;
	
	int fstack[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for(location = &(board->pawn_locations[turn][0]); *location != -1; location++){
		if (turn == ColourWhite)
			value += WHITE_PAWN_POSITION_VALUE(*location);
		else if (turn == ColourBlack)
			value += BLACK_PAWN_POSITION_VALUE(*location);
		if (++(fstack[*location%16]) != 1)
			value += PawnStackedValue;
		
		int self = board->squares[*location];
		
		if (board->squares[*location-pawn_delta-1] == self)
			value += DiagonallyConnectedPawnValue;
		else if (board->squares[*location-pawn_delta+1] == self)
			value += DiagonallyConnectedPawnValue;
		
		if (board->squares[*location+1] == self)
			value += HorizontallyConnectedPawnValue;
		else if (board->squares[*location-1] == self)
			value += HorizontallyConnectedPawnValue;
	}
	
	int estack[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for(location = &(board->pawn_locations[!turn][0]); *location != -1; location++){
		if (turn == ColourWhite)
			value -= WHITE_PAWN_POSITION_VALUE(*location);
		else if (turn == ColourBlack)
			value -= BLACK_PAWN_POSITION_VALUE(*location);
		if (++(estack[*location%16]) != 1)
			value -= PawnStackedValue;
		
		int self = board->squares[*location];
		
		if (board->squares[*location-pawn_delta-1] == self)
			value -= DiagonallyConnectedPawnValue;
		else if (board->squares[*location-pawn_delta+1] == self)
			value -= DiagonallyConnectedPawnValue;
		
		if (board->squares[*location+1] == self)
			value += HorizontallyConnectedPawnValue;
		else if (board->squares[*location-1] == self)
			value += HorizontallyConnectedPawnValue;
	}
	
	int i;
	for(i = 5; i < 11; i++){
		if (fstack[i] && !(fstack[i-1]) && !(fstack[i+1]))
			value += IsolatedPawnValue;
		if (estack[i] && !(estack[i-1]) && !(estack[i+1]))
			value -= IsolatedPawnValue;
	}
	
	return value;
}