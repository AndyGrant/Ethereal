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
	init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	
	int i;
	for(i = 1; i < 6; i++){
		printf("Depth %d : %d\n",i,foo(&board,i));
	}
	
	
}