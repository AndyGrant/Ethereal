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
	
	int turn, *location;
	int pawn_info[2][10] = {{0,0,0,0,0,0,0,0,0,0},{7,7,7,7,7,7,7,7,7,7}};
	for(turn = ColourWhite; turn <= ColourBlack; turn++){
		for(location = &(board->pawn_locations[turn][0]); *location != -1; location++){
			int sq64 = CONVERT_256_TO_64(board->pawn_locations[turn][*location]);
			int col = 1 + sq64 % 8;
			int row = 7 - sq64 / 8;
			
			if (turn == ColourWhite){
				if (row > pawn_info[ColourWhite][col])
					pawn_info[ColourWhite][col] = row;
			}
			else if(turn == ColourBlack){
				if (row < pawn_info[ColourBlack][col])
					pawn_info[ColourBlack][col] = row;
			}
		}
	}
	
	int white = evaluate_player(board,ColourWhite,pawn_info);
	int black = evaluate_player(board,ColourBlack,pawn_info);
	
	if (board->turn == ColourWhite)
		return white - black;
	return black - white;
}

int evaluate_player(board_t * board, int turn, int pawn_info[2][10]){
	int value = 0, *location, i;
	
	for(location = &(board->piece_locations[turn][0]); *location != -1; location++){
		int type = board->squares[*location];
		int sq256 = *location;
		int sq64 = CONVERT_256_TO_64(sq256);
		
		switch(type){
			case WhiteKnight:
				value += KnightValue;
				value += KnightValueMap[sq64];
				break;
			
			case BlackKnight:
				value += KnightValue;
				value += KnightValueMap[inv[sq64]];
				break;
			
			case WhiteBishop:
				value += BishopValue;
				value += BishopValueMap[sq64];
				break;
			
			case BlackBishop:
				value += BishopValue;
				value += BishopValueMap[inv[sq64]];
				break;
				
			case WhiteRook:
				value += RookValue;
				break;
			
			case BlackRook:
				value += RookValue;
				break;
				
			case WhiteQueen:
				value += QueenValue;
				break;
			
			case BlackQueen:
				value += QueenValue;
				break;
			
			case WhiteKing:
				if (board->piece_counts[0] + board->piece_counts[1] < 7)
					value += KingEndValueMap[sq64];
				else
					value += KingEarlyValueMap[sq64];
				break;
				
			case BlackKing:
				if (board->piece_counts[0] + board->piece_counts[1] < 7)
					value += KingEndValueMap[inv[sq64]];
				else
					value += KingEarlyValueMap[inv[sq64]];
				break;
			
		}
	}
	
	for(location = &(board->pawn_locations[turn][0]); *location != -1; location++){
		int sq256 = *location;
		int sq64 = CONVERT_256_TO_64(sq256);
		int col = 1 + sq64 % 8;
		int row = 7 - sq64 / 8;
		
		value += PawnValue;
		value += PawnValueMap[sq64];
		
		if (turn == ColourWhite){
			if (pawn_info[ColourWhite][col] > row)
				value += StackedPawnValue;
			
			if (pawn_info[ColourWhite][col+1] == 0 && pawn_info[ColourWhite][col-1] == 0)
				value += IsolatedPawnValue;
		}
		
		else if (turn == ColourBlack){
			if (pawn_info[ColourBlack][col] < row)
				value += StackedPawnValue;
			
			if (pawn_info[ColourBlack][col+1] == 0 && pawn_info[ColourBlack][col-1] == 0)
				value += IsolatedPawnValue;
		}
	}
	
	return value;
}