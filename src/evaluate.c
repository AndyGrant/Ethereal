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

extern board_t board;

int evaluate_board(){
	int turn, *location;
	int pawn_info[2][10] = {{0,0,0,0,0,0,0,0,0,0},{7,7,7,7,7,7,7,7,7,7}};
	for(turn = ColourWhite; turn <= ColourBlack; turn++){
		for(location = &(board.pawn_locations[turn][0]); *location != -1; location++){
			int sq64 = CONVERT_256_TO_64(*location);
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
	
	int white = evaluate_player(ColourWhite,pawn_info);
	int black = evaluate_player(ColourBlack,pawn_info);
	
	if (board.turn == ColourWhite)
		return 0 + white - black;
	return 0 + black - white;
}

int evaluate_player(int turn, int pawn_info[2][10]){	
	
	int value 		= 0;
	int *location	= NULL;
	int pstart 		= turn == ColourWhite ? 0 : 7;
	int pend   		= turn == ColourWhite ? 7 : 0;
	int is_endgame  = (board.piece_counts[0] + board.piece_counts[1] <= 7);
	int rook_sq     = -1;
	int rook_stack  = 0;
	
	for(location = &(board.piece_locations[turn][0]); *location != -1; location++){
		int type = board.squares[*location] & ~1;
		int sq256 = *location;
		int sq64= CONVERT_256_TO_64(sq256);
		int col = 1 + sq64 % 8;
		
		int truesq = turn == ColourWhite ? sq64 : inv[sq64];
		
		
		switch(type){
			case WhiteKnight:
				value += KnightValue + KnightValueMap[truesq];
			break;
			
			case WhiteBishop:
				value += BishopValue + BishopValueMap[truesq];
			break;
			
			case WhiteRook:
				value += RookValue;
			
				if (rook_sq == -1)
					rook_sq = truesq;
				else if (truesq % 8 == rook_sq % 8)
					rook_stack = 1;
				
				if (pawn_info[board.turn][col] == pstart){
					if (pawn_info[!board.turn][col] == pend){
						value += OpenFileRookValue;
						if (rook_stack)
							value += RooksOnSameOpenFileValue;
					} else{
						value += SemiOpenFileRookValue;
						if (rook_stack)
							value += RooksOnSameSemiOpenFileValue;
					}
				}
				
				if (truesq / 8 == 1)
					value += SeventhRankRookValue;
				else if (truesq / 8 == 1)
					value += EighthRankRookValue;
				
			break;
				
			case WhiteQueen:
				value += QueenValue;
			break;
			
			case WhiteKing:
				if (is_endgame)
					value += KingEndValueMap[truesq];
				else
					value += KingEarlyValueMap[truesq];
			break;
		}
	}
		
	for(location = &(board.pawn_locations[turn][0]); *location != -1; location++){
		int sq256 = *location;
		int sq64 = CONVERT_256_TO_64(sq256);
		int col = 1 + sq64 % 8;
		int row = 7 - sq64 / 8;
		
		value += PawnValue;
		
		if (turn == ColourWhite){
			if (is_endgame)
				value += PawnEndValueMap[sq64];
			else
				value += PawnEarlyValueMap[sq64];
		}
		else{
			if (is_endgame)
				value += PawnEndValueMap[inv[sq64]];
			else
				value += PawnEarlyValueMap[inv[sq64]];
		}
		
		if (turn == ColourWhite){
			if (pawn_info[ColourWhite][col] > row)
				value += StackedPawnValue;
			
			if (pawn_info[ColourWhite][col+1] == 0 && pawn_info[ColourWhite][col-1] == 0)
				value += IsolatedPawnValue;
			
			if (pawn_info[ColourBlack][col] < row &&
				pawn_info[ColourBlack][col-1] <= row &&
				pawn_info[ColourBlack][col+1] <= row)
				value += row * PassedPawnValue;
		}
		
		else if (turn == ColourBlack){
			if (pawn_info[ColourBlack][col] < row)
				value += StackedPawnValue;
			
			if (pawn_info[ColourBlack][col+1] == 0 && pawn_info[ColourBlack][col-1] == 0)
				value += IsolatedPawnValue;
			
			if (pawn_info[ColourBlack][col] > row &&
				pawn_info[ColourBlack][col-1] >= row &&
				pawn_info[ColourBlack][col+1] >= row)
				value += (7-row) * PassedPawnValue;
		}
	}
	
	return value;
}