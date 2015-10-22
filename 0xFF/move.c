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
	
	int to, from, cap, piece;
	int turn = board->turn;
	int r, rights = board->castle_rights;
	int * location = (board->piece_locations[turn]);
	
	while (*location != -1){
		from = *location;
		switch(PIECE_TYPE(board->squares[from])){
			
			case KnightFlag:
				
				if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=from-33]),turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=from-31]),turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=from-18]),turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=from-14]),turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=from+14]),turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=from+18]),turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=from+31]),turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=from+33]),turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				break;
			
			case BishopFlag:
				
				to = from; cap = Empty;
				while(cap == Empty){
					if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=to-17]),turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=to-15]),turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=to+15]),turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=to+17]),turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				break;
				
			case RookFlag:
				
				r = 0;
				if (KING_HAS_RIGHTS(turn,rights) && IS_RIGHT_ROOK(turn,from))
					r |= CREATE_KING_RIGHTS(turn);
				if (QUEEN_HAS_RIGHTS(turn,rights) && IS_LEFT_ROOK(turn,from))
					r |= CREATE_QUEEN_RIGHTS(turn);
				
				to = from; cap = Empty;
				while(cap == Empty){
					if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=to-16]),turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,r);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=to-1]),turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,r);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=to+1]),turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,r);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					if (IS_EMPTY_OR_ENEMY((cap = board->squares[to=to+16]),turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,r);
					else break;
				}
				
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