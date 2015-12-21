#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "board.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "util.h"

int main2(){
	int i, milli;
	unsigned long long nodes;
	board_t board;
	
	init_board_t(&board,"eeeeeeeePPPkeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeKpppeeeeeeee00000001");
	
	print_board_t(&board);

	for(i = 0; i < 7; i++){
		clock_t start = clock();
		nodes = perft(&board,i);
		clock_t end = clock();
		milli = ((int)end) - ((int)start);
		printf("Depth %d \t| Time %d \t| Nodes %llu\n",i,milli,nodes);
	}
}