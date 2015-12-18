#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "types.h"
#include "board.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "util.h"


int main(){
	clock_t start = clock();
	
	board_t board;
	//init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	//init_board_t(&board,"reeekeereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeKeee11110000");
	  init_board_t(&board,"reeekeereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeReeQKeeR11110000");
	
	print_board_t(&board);
	
	int i;
	for (i = 0; i < 6; i++)
		printf("PERFT Depth %d : Nodes %d\n",i,perft(&board,i));
	
	printf("Time Taken=%d\n",(int)((clock()-start)/CLOCKS_PER_SEC));
}