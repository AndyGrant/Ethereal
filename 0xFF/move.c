#include <assert.h>
#include <stdio.h>
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
	assert(PIECE_IS_KING(board->squares[board->piece_locations[board->turn][0]]));
	assert(list != NULL);
	
	int to, from, piece;
	int turn = board->turn;
	int * location = &(board->piece_locations[turn][0]);
	
	while(*location != -1){
		switch(PIECE_TYPE(board->squares[*location])){
			from = *location;
			
			
			case KnightFlag:
				printf("Finding moves of Knight at %d\n",CONVERT_256_TO_64(from));
				to = from - 33;
				if (IS_EMPTY_OR_ENEMY(board->squares[to],turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(board,to,from,0);
				
				to = from - 31;
				if (IS_EMPTY_OR_ENEMY(board->squares[to],turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(board,to,from,0);
				
				to = from - 18;
				if (IS_EMPTY_OR_ENEMY(board->squares[to],turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(board,to,from,0);
				
				to = from - 14;
				if (IS_EMPTY_OR_ENEMY(board->squares[to],turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(board,to,from,0);
				
				to = from + 14;
				if (IS_EMPTY_OR_ENEMY(board->squares[to],turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(board,to,from,0);
				
				to = from + 18;
				if (IS_EMPTY_OR_ENEMY(board->squares[to],turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(board,to,from,0);
				
				to = from + 31;
				if (IS_EMPTY_OR_ENEMY(board->squares[to],turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(board,to,from,0);
				
				to = from + 33;
				if (IS_EMPTY_OR_ENEMY(board->squares[to],turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(board,to,from,0);
				
				break;
			default: break;
		}
		
		location++;
	}
}

//	bits	00-07: From Square
//	bits	08-15: To Square 
//	bits	16-23: Capture Type
//	bits	24-27: Move Type
//	bits	28-31: Castle Changes