#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "board.h"
#include "castle.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "types.h"
#include "util.h"

void gen_all_moves(move_t * list, int * size, board_t * board){
	assert(*size == 0);
	assert(board->turn == 0 || board->turn == 1);
	assert(board->piece_counts[board->turn] != 0);
	assert(PIECE_IS_KING(board->piece_locations[board->turn][0]));
	assert(list != NULL);
	
	int to, from, piece;
	int turn = board->turn;
	int * location = board->piece_locations[turn];
	
	while(*location != -1){
		switch(PIECE_TYPE(board->squares[*location])){
			case KnightFlag:
				
				to = from - 33;
				if (IS_EMPTY_OR_ENEMY(board->squares[to]))
					ADD_MOVE(list,size,MAKE_NORMAL_MOVE(from,to,0));
		}
	}
}