#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "castle.h"
#include "magics.h"
#include "piece.h"
#include "types.h"
#include "search.h"
#include "move.h"
#include "movegen.h"
#include "movegentest.h"
#include "zorbist.h"

int main(){
	
	
	//move_gen_test();
	//return 0;
	
	Board board;
	init_board(&board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	printf("%d",get_best_move(&board,60));
}