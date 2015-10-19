#include <stdio.h>
#include <assert.h>

#include "piece.h"
#include "util.h"


int char_to_piece(char c){
	switch(c){
		case 'P': return WhitePawn;
		case 'p': return BlackPawn;
		case 'N': return WhiteKnight; 
		case 'n': return BlackKnight; 
		case 'B': return WhiteBishop; 
		case 'b': return BlackBishop; 
		case 'R': return WhiteRook; 
		case 'r': return BlackRook; 
		case 'Q': return WhiteQueen; 
		case 'q': return BlackQueen; 
		case 'K': return WhiteKing; 
		case 'k': return BlackKing; 
		case 'e': return NonePiece; 
		default : assert(0);
	}
}

char piece_to_char(int p){
	switch(p){
		case WhitePawn: 	return 'P';
		case BlackPawn: 	return 'p';
		case WhiteKnight: 	return 'N';
		case BlackKnight: 	return 'n';
		case WhiteBishop: 	return 'B';
		case BlackBishop: 	return 'b';
		case WhiteRook: 	return 'R';
		case BlackRook: 	return 'r';
		case WhiteQueen: 	return 'Q';
		case BlackQueen: 	return 'q';
		case WhiteKing: 	return 'K';
		case BlackKing: 	return 'k';
		case NonePiece:		return 'e';
		default:			assert(0);
	}
}

