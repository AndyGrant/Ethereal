#include <stdio.h>

#include "board.h"
#include "magics.h"
#include "piece.h"
#include "types.h"

void init_board(Board * board, char * fen){
	int i, j, sq, p;
	
	// Init board->squares from fen notation;
	for(i = 0, sq = 63; fen[i] != ' '; i++){
		if (fen[i] == '/' || fen[i] == '\\')
			continue;
		
		else if (fen[i] <= '8' && fen[i] >= '1')
			for(j = 0; j < fen[i] - '0'; j++, sq--)
				board->squares[sq] = Empty;
			
		else {
			switch(fen[i]){
				case 'P': board->squares[sq--] = WhitePawn;   break;
				case 'N': board->squares[sq--] = WhiteKnight; break;
				case 'B': board->squares[sq--] = WhiteBishop; break;
				case 'R': board->squares[sq--] = WhiteRook;   break;
				case 'Q': board->squares[sq--] = WhiteQueen;  break;
				case 'K': board->squares[sq--] = WhiteKing;   break;
				case 'p': board->squares[sq--] = BlackPawn;   break;
				case 'n': board->squares[sq--] = BlackKnight; break;
				case 'b': board->squares[sq--] = BlackBishop; break;
				case 'r': board->squares[sq--] = BlackRook;   break;
				case 'q': board->squares[sq--] = BlackQueen;  break;
				case 'k': board->squares[sq--] = BlackKing;   break;
			}
		}
	}
	
	// Empty all BitBoards
	board->colourBitBoards[0] = 0;
	board->colourBitBoards[1] = 0;
	board->pieceBitBoards[0] = 0;
	board->pieceBitBoards[1] = 0;
	board->pieceBitBoards[2] = 0;
	board->pieceBitBoards[3] = 0;
	board->pieceBitBoards[4] = 0;
	board->pieceBitBoards[5] = 0;
	
	// Fill BitBoards
	for(i = 0; i < 64; i++){
		board->colourBitBoards[PIECE_COLOUR(board->squares[i])] |= (1ull << i);
		board->pieceBitBoards[PIECE_TYPE(board->squares[i])] 	|= (1ull << i);
	}
	
	board->turn = ColourWhite;
	board->castlerights = 0b1111;
	board->fiftymoverule = 0;
	board->epsquare = 0;
	board->lastcap = Empty;	
}

void print_board(Board * board){
	
}

int main(){
	Board board;	
	init_board(&board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	init_magics();	
}