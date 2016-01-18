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
	printf("Opening=%d Endgame=%d\n",board.opening,board.endgame);
	print_board(&board);
	
	
	//printf("Moves : %d\n",perft(&board,6));
}