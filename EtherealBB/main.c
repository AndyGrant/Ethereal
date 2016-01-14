#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "castle.h"
#include "magics.h"
#include "piece.h"
#include "types.h"
#include "move.h"
#include "movegen.h"
#include "movegentest.h"
#include "zorbist.h"

int main(){
	move_gen_test();
	return 0;
	
	Board board;	
	init_board(&board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");	
	//init_board(&board,"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1 ");	
	//init_board(&board,"r3k3/1K6/8/8/8/8/8/8 w q - 0 1 ");
	

	print_board(&board);
	//printf("Moves : %d\n",perft(&board,1));
	//printf("Moves : %d\n",perft(&board,2));
	//printf("Moves : %d\n",perft(&board,3));
	//printf("Moves : %d\n",perft(&board,4));
	//printf("Moves : %d\n",perft(&board,5));
	printf("Moves : %d\n",perft(&board,6));
}