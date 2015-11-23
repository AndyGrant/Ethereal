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

unsigned long long foo(board_t * board, int depth){
	if (depth == 0)
		return 0;
	
	move_t moves[MaxMoves];
	int size = 0;
	gen_all_moves(board,&(moves[0]),&size);
	
	unsigned long long found = 0;
	
	for (size = size - 1; size >= 0; size--){
		apply_move(board,moves[size]);
		if (is_not_in_check(board,!board->turn))
			found += 1 + foo(board,depth-1);
		revert_move(board,moves[size]);
	}
	
	return found;
}

int main(){
	
	
	clock_t start = clock();
	
	board_t board;
	//init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	  init_board_t(&board,"eeerkeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeRKeee11110000");
	
	/*int i;
	for(i = 0; i <= 2; i++)
		printf("Depth=%d Found=%llu\n",i,foo(&board,i));
	*/
	int i;
	
	move_t moves[MaxMoves];
	int size = 0;
	gen_all_moves(&board,&(moves[0]),&size);
	
	
	print_board_t(&board);
	for(i = 0; i < size; i++){
		printf("\n#%d",i);
		print_move_t(moves[i]);
	}
	return;
	
	apply_move(&board,moves[10]);
	print_move_t(moves[10]);
	
	size = 0;
	gen_all_moves(&board,&(moves[0]),&size);
	
	print_board_t(&board);
	
	
	for(i = 0; i < size; i++){
		printf("\n#%d",i);
		print_move_t(moves[i]);
	}
	
	printf("Time Taken=%d\n",(int)((clock()-start)/CLOCKS_PER_SEC));
}

/*

53335553333323

*/