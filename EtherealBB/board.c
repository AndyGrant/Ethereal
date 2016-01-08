#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "board.h"
#include "magics.h"
#include "piece.h"
#include "types.h"
#include "move.h"
#include "movegen.h"

void init_board(Board * board, char * fen){
	int i, j, sq, p;
	
	// Init board->squares from fen notation;
	for(i = 0, sq = 56; fen[i] != ' '; i++){
		if (fen[i] == '/' || fen[i] == '\\'){
			sq -= 16;
			continue;
		}
		
		else if (fen[i] <= '8' && fen[i] >= '1')
			for(j = 0; j < fen[i] - '0'; j++, sq++)
				board->squares[sq] = Empty;
			
		else {
			switch(fen[i]){
				case 'P': board->squares[sq++] = WhitePawn;   break;
				case 'N': board->squares[sq++] = WhiteKnight; break;
				case 'B': board->squares[sq++] = WhiteBishop; break;
				case 'R': board->squares[sq++] = WhiteRook;   break;
				case 'Q': board->squares[sq++] = WhiteQueen;  break;
				case 'K': board->squares[sq++] = WhiteKing;   break;
				case 'p': board->squares[sq++] = BlackPawn;   break;
				case 'n': board->squares[sq++] = BlackKnight; break;
				case 'b': board->squares[sq++] = BlackBishop; break;
				case 'r': board->squares[sq++] = BlackRook;   break;
				case 'q': board->squares[sq++] = BlackQueen;  break;
				case 'k': board->squares[sq++] = BlackKing;   break;
			}
		}
	}
	
	// Set BitBoards to default values
	board->colourBitBoards[0] = 0;
	board->colourBitBoards[1] = 0;
	board->colourBitBoards[2] = 0xFFFFFFFFFFFFFFFF;
	board->pieceBitBoards[0] = 0;
	board->pieceBitBoards[1] = 0;
	board->pieceBitBoards[2] = 0;
	board->pieceBitBoards[3] = 0;
	board->pieceBitBoards[4] = 0;
	board->pieceBitBoards[5] = 0;
	board->pieceBitBoards[6] = 0xFFFFFFFFFFFFFFFF;
	
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
	int i, j;
	for(i = 56; i >= 0; i-=8){
		for(j = 0; j < 8; j++)
			printf("%d ", board->squares[i+j]);
		printf("\n");
	}
}

int perft(Board * board, int depth){
	if (depth == 0)
		return 1;
	
	uint16_t moves[256];
	int size = 0;
	gen_all_moves(board,moves,&size);
	Undo undo;
	
	int found = 0;
	for(size -= 1; size >= 0; size--){		
		apply_move(board,moves[size],&undo);
		if (is_not_in_check(board,!board->turn))
			found += perft(board,depth-1);
		revert_move(board,moves[size],&undo);
	}
	
	return found;
}

int main(){
	Board board;	
	//init_board(&board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	init_board(&board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");	
	//init_board(&board,"K7/8/2n5/1n6/8/8/8/k6N w - - 0 1");
	//init_board(&board,"8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1");
	init_magics();
	
	printf("Moves : %d\n",perft(&board,4));
	//printf("Moves : %d\n",perft(&board,6));
	//printf("Moves : %d\n",perft(&board,6));
	printf("Moves : %d\n",perft(&board,6));
}