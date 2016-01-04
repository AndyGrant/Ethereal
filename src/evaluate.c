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
		return 10 + white - black;
	return 10 + black - white;
}

int evaluate_player(int turn, int pawn_info[2][10]){
	int value = 0, *location, i;
	
	int num_knights = 0;
	int num_bishops = 0;
	int rook_location = -1;
	
	for(location = &(board.piece_locations[turn][0]); *location != -1; location++){
		int type = board.squares[*location];
		int sq256 = *location;
		int sq64 = CONVERT_256_TO_64(sq256);
		int col = 1 + sq64 % 8;
		
		switch(type){
			case WhiteKnight:
				value += KnightValue;
				value += KnightValueMap[sq64];
				num_knights++;
				break;
			
			case BlackKnight:
				value += KnightValue;
				value += KnightValueMap[inv[sq64]];
				num_knights++;
				break;
			
			case WhiteBishop:
				value += BishopValue;
				value += BishopValueMap[sq64];
				num_bishops++;
				break;
			
			case BlackBishop:
				value += BishopValue;
				value += BishopValueMap[inv[sq64]];
				num_bishops++;
				break;
				
			case WhiteRook:
				value += RookValue;
				if (pawn_info[ColourWhite][col] == 0){
					if (pawn_info[ColourBlack][col] == 7){
						value += OpenFileRookValue;
						if (rook_location == -1) 
							rook_location = sq64;
						else if (rook_location % 8 == sq64 % 8)
							value += RooksOnSameOpenFileValue;
					}
					else{
						value += SemiOpenFileRookValue;
						if (rook_location == -1) 
							rook_location = sq64;
						else if (rook_location % 8 == sq64 % 8)
							value += RooksOnSameOpenFileValue;
					}
				}
				
				if (sq64 / 16 == 1)
					value += SeventhRankRookValue;
				if (sq64 / 16 == 0)
					value += EightRankRookValue;
				
				break;
			
			case BlackRook:
				value += RookValue;
				if (pawn_info[ColourBlack][col] == 7){
					if (pawn_info[ColourWhite][col] == 0){
						value += OpenFileRookValue;
						if (rook_location == -1) 
							rook_location = sq64;
						else if (rook_location % 8 == sq64 % 8)
							value += RooksOnSameOpenFileValue;
					}
					else{
						value += SemiOpenFileRookValue;
						if (rook_location == -1) 
							rook_location = sq64;
						else if (rook_location % 8 == sq64 % 8)
							value += RooksOnSameSemiOpenFileValue;
					}
				}
				
				if (sq64 / 16 == 6)
					value += SeventhRankRookValue;
				if (sq64 / 16 == 7)
					value += EightRankRookValue;
				
				break;
				
			case WhiteQueen:
				value += QueenValue;
				break;
			
			case BlackQueen:
				value += QueenValue;
				break;
			
			case WhiteKing:
				if (board.piece_counts[0] + board.piece_counts[1] <= 7)
					value += KingEndValueMap[sq64] / 2;
				else{
					value += KingEarlyValueMap[sq64];
					if (board.squares[sq256-16] == WhitePawn)
						value += PawnInfrontOfKingValue;
					if (board.squares[sq256-15] == WhitePawn)
						value += PawnInfrontOfKingValue;
					if (board.squares[sq256-17] == WhitePawn)
						value += PawnInfrontOfKingValue;
				}
				
				break;
				
			case BlackKing:
				if (board.piece_counts[0] + board.piece_counts[1] <= 7)
					value += KingEndValueMap[inv[sq64]] / 2;
				else{
					value += KingEarlyValueMap[inv[sq64]];
					if (board.squares[sq256+16] == BlackPawn)
						value += PawnInfrontOfKingValue;
					if (board.squares[sq256+17] == BlackPawn)
						value += PawnInfrontOfKingValue;
					if (board.squares[sq256+15] == BlackPawn)
						value += PawnInfrontOfKingValue;
				}
				break;
			
		}
	}
	
	if (num_bishops == 2)	
		value += HasBothBishopsValue;
	
	int is_end_game = (board.piece_counts[0] + board.piece_counts[1] <= 7);
	
	for(location = &(board.pawn_locations[turn][0]); *location != -1; location++){
		int sq256 = *location;
		int sq64 = CONVERT_256_TO_64(sq256);
		int col = 1 + sq64 % 8;
		int row = 7 - sq64 / 8;
		
		value += PawnValue;
		
		if (turn == ColourWhite){
			if (is_end_game)
				value += PawnEndValueMap[sq64];
			else
				value += PawnEarlyValueMap[sq64];
		}
		else{
			if (is_end_game)
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