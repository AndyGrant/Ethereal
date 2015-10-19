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
	
	int turn = board->turn;
	int location = board->piece_locations[turn];
	
	while(*location != -1){
		int piece = board->squares[*location];
		
		if (PIECE_IS_KNIGHT(piece)){
			
		}
		else if (PIECE_IS_BISHOP(piece)){
			
		}
		else if (PIECE_IS_ROOK(piece)){
			
		}
		else if (PIECE_IS_ROOK(piece)){
			
		}
		else if (PIECE_IS_KING(piece)){
			
		}
		else
			assert(0);
	}
}

void create_jumping_move(board_t * board, int from, int to){
	move_t move = 0;
	*move |= from;
	*move |= to << 8;
	*move |= capture << 16;
	*move |= NormalFlag;
	return move;
}