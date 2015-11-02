#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "types.h"
#include "board.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "util.h"

int foo(board_t * board, int depth){
	if (depth == 0)
		return 0;
	
	move_t moves[MaxMoves];
	int size = 0;
	gen_all_moves(board,&(moves[0]),&size);
	
	int found = size;
	
	for (size = size - 1; size >= 0; size--){
		apply_move(board,moves[size]);
		found += foo(board,depth-1);
		revert_move(board,moves[size]);
	}
	
	return found;
}



int main(){
	board_t board;
	init_board_t(&board,"rebqkbnrppppppppneeeeeeeeeeeeeeeeeeeeeeeNeeeeeeePPPPPPPPReBQKBNR11110000");
	
	int size = 0;
	move_t moves[MaxMoves];
	gen_all_moves(&board,&(moves[0]),&size);
	
	for(size-=1; size >= 0; size--)
		print_move_t(moves[size]);
	
	return;
	
	init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	
	int size1=0, size2=0, size3=0, size4=0;
	move_t moves1[MaxMoves], moves2[MaxMoves], moves3[MaxMoves], moves4[MaxMoves];

	
	print_board_t(&board);
	gen_all_moves(&board,&(moves1[0]),&size1);
	print_move_t(moves1[0]);
	apply_move(&board,moves1[0]);
	
	print_board_t(&board);
	gen_all_moves(&board,&(moves2[0]),&size2);
	print_move_t(moves2[0]);
	apply_move(&board,moves2[0]);
	
	print_board_t(&board);
	gen_all_moves(&board,&(moves3[0]),&size3);
	
	char b[73];
	encode_board_t(&board,&b[0]);
	printf("Board : %s \n",b);
	
	print_move_t(moves3[0]);
	apply_move(&board,moves3[0]);
	
	print_board_t(&board);
	
	
	
	
}