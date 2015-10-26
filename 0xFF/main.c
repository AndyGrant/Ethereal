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

int main(){
	board_t board;
	init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	//init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPePeeeRNBQKBNR11110000");
	
	// Test Promotion
	//init_board_t(&board,"eeeekeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePpPeeeeeeeeeeeeeeeeeeeeeeKeee00001371");
	
	print_board_t(&board);
	
	
	
	move_t moves[MaxMoves];
	
	clock_t start = clock();
	
	int a, size = 0;
	for(a = 0; a < 20000000; a++, size=0)
		gen_all_moves(&moves[0],&size,&board);
	
	clock_t end = clock();
	
	printf("Time in milli : %d",end-start);
	
	int i;
	for(i = 0; i < size; i++){
		printf("#%d ",i);print_move_t(moves[i]);
	}
}