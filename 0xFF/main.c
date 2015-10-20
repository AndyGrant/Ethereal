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
	init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	print_board_t(&board);
	
	move_t moves[MaxMoves];
	
	int size = 0;
	gen_all_moves(moves,&size,&board);
	
	int i;
	for(i = 0; i < size; i++){
		printf("To %d, From %d \n",CONVERT_256_TO_64(MOVE_GET_TO(moves[i])),
								   CONVERT_256_TO_64(MOVE_GET_FROM(moves[i])));
		printf("%x\n\n",moves[i]);
	}
}