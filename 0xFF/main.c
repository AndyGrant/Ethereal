#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "types.h"
#include "board.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "util.h"

int main(){
	board_t board;
	//init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	//init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPePeeeRNBQKBNR11110000");
	
	// Test Promotion
	  init_board_t(&board,"eeeekeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePpPeeeeeeeeeeeeeeeeeeeeeeKeee00001371");
	
	print_board_t(&board);
	
	
	
	move_t moves[MaxMoves];
	
	int size = 0;
	gen_all_moves(&moves[0],&size,&board);
	
	int i;
	for(i = 0; i < size; i++){
		printf("#%d To %d, From %d Cap %d\n",i, (MOVE_GET_TO(moves[i])),
										 (MOVE_GET_FROM(moves[i])),
										 (MOVE_GET_CAPTURE(moves[i])));
	}
}