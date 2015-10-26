#include <stdio.h>
#include <assert.h>

#include "board.h"
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
		case 'e': return Empty; 
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
		case Empty:			return 'e';
		default:			assert(0);
	}
}

void print_board_t(board_t * board){
	int x, y;
	
	char files[8] = "87654321";
	
	for(x = 0; x < 8; x++){
		printf("\n     |----|----|----|----|----|----|----|----|\n");
		printf("    %c",files[x]);
		for(y = 0; y < 8; y++){
			int piece = board->squares[CONVERT_64_TO_256((x*8+y))];
			if (IS_PIECE(piece)){
				if (PIECE_IS_WHITE(piece))
					printf("| w%c ",piece_to_char(piece));
				else
					printf("| b%c ",piece_to_char(piece));
			}
			else
				printf("|    ");
		}
		
		printf("|");
	}
	
	printf("\n     |----|----|----|----|----|----|----|----|");
	printf("\n        A    B    C    D    E    F    G    H\n");
}

void print_move_t(move_t move){
	if (MOVE_IS_NORMAL(move)){
		int from = MOVE_GET_FROM(move);
		int to = MOVE_GET_TO(move);
		printf("Normal : %c%c%c%c\n",CONVERT_TO_FILE(from),
									 CONVERT_TO_RANK(from),
									 CONVERT_TO_FILE(to),
									 CONVERT_TO_RANK(to)
		);
	} else if (MOVE_IS_CASTLE(move)){
	} else if (MOVE_IS_ENPASS(move)){
	} else if (MOVE_IS_PROMOTION(move)){
	} else {
		assert("move_t has no move type" == 0);
	}
}

