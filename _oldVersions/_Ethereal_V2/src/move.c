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

extern board_t board;
extern zorbist_t zorbist;

void gen_all_legal_moves(move_t * list, int * size){
	int raw_size = 0;
	move_t raw_moves[MaxMoves];
	gen_all_moves(&(raw_moves[0]),&raw_size);
	
	int i;
	for(i = 0; i < raw_size; i++){
		apply_move(raw_moves[i]);
		if (is_not_in_check(!board.turn))
			list[(*size)++] = raw_moves[i];
		revert_move(raw_moves[i]);
	}
}

void gen_all_captures(move_t * list, int * size){
	assert(*size == 0);
	assert(board.turn == 0 || board.turn == 1);
	assert(board.piece_counts[board.turn] != 0);
	assert(PIECE_IS_KING(board.squares[board.piece_locations[board.turn][0]]));
	assert(list != NULL);
	
	unsigned int to, from, cap, flag;
	int * pcap;
	int turn = board.turn;
	int direction = board.turn == ColourWhite ? -16 : 16;
	int * location = &(board.piece_locations[turn][0]);
	
	while (*location != -1){
		from = *location;
		switch(PIECE_TYPE(board.squares[from])){
			
			case KnightFlag:
				to = from - 33; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from - 31; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from - 18; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from - 14; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 14; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 18; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 31; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 33; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				break;
			
			case BishopFlag:

				to = from - 17;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += -17;pcap += -17;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);

				to = from - 15;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += -15;pcap += -15;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from + 15;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += 15;pcap += 15;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from + 17;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += 17;pcap += 17;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				break;
				
			case RookFlag:				
				to = from - 16;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += -16;pcap += -16;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from - 1;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += -1;pcap += -1;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from + 1;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += 1;pcap += 1;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from + 16;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += 16;pcap += 16;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				break;
				
			case QueenFlag:
				to = from - 17;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += -17;pcap += -17;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);

				to = from - 15;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += -15;pcap += -15;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from + 15;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += 15;pcap += 15;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from + 17;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += 17;pcap += 17;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from - 16;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += -16;pcap += -16;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from - 1;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += -1;pcap += -1;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from + 1;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += 1;pcap += 1;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				to = from + 16;
				pcap = board.squares + to;
				while (IS_EMPTY(*pcap)){to += 16;pcap += 16;} 
				if (IS_ENEMY(*pcap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,*pcap,0);
				
				break;
				
			case KingFlag:				
				to = from - 17; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from - 16; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from - 15; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from - 1; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 1; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 15; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 16; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 17; cap = board.squares[to];
				if (IS_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				break;
			
			default: 
				print_board_t();
				int z;
				for(z = 0; z < 16; z++)
					printf("%d ", board.piece_locations[turn][z]);
				printf("\n");
				for(z = 0; z < 16; z++)
					printf("%d ", board.pawn_locations[turn][z]);
				printf("\n");
				assert("Error in board_locations[][]" == 0);
		}
		
		location++;
	}
	
	location = &(board.pawn_locations[turn][0]);
	
	while (*location != -1){
		from = *location;
		int promo = 0;
		
		if (turn == ColourWhite && from / 16 == 5)
			promo = 1;
		else if(turn == ColourBlack && from / 16 == 10)
			promo = 1;
		
		to = from + direction;
		if (IS_PIECE(board.squares[to+1]) && PIECE_COLOUR(board.squares[to+1]) == !turn){
			
			if (promo){
				for(flag = PromoteQueenFlag; flag >= PromoteKnightFlag; flag = flag >> 1)
					list[(*size)++] = MAKE_PROMOTION_MOVE(from,to+1,board.squares[to+1],flag);
			}
			else
				list[(*size)++] = MAKE_NORMAL_MOVE(from,to+1,board.squares[to+1],0);
		}
		
		if (IS_PIECE(board.squares[to-1]) && PIECE_COLOUR(board.squares[to-1]) == !turn){
				
			if (promo){
				for(flag = PromoteQueenFlag; flag >= PromoteKnightFlag; flag = flag >> 1)
					list[(*size)++] = MAKE_PROMOTION_MOVE(from,to-1,board.squares[to-1],flag);
			}
			else
				list[(*size)++] = MAKE_NORMAL_MOVE(from,to-1,board.squares[to-1],0);
		}
		
		int ep = board.ep_history[board.depth];
		if (PIECE_COLOUR(board.squares[ep]) == !turn){
			if (ep == from-1)
				list[(*size)++] = MAKE_ENPASS_MOVE(from,ep+direction,ep);
			if (ep == from+1)
				list[(*size)++] = MAKE_ENPASS_MOVE(from,ep+direction,ep);
		}
		
		
		location++;
	}
}

void gen_all_moves(move_t * list, int * size){
	assert(*size == 0);
	assert(board.turn == 0 || board.turn == 1);
	assert(board.piece_counts[board.turn] != 0);
	assert(PIECE_IS_KING(board.squares[board.piece_locations[board.turn][0]]));
	assert(list != NULL);
	
	unsigned int to, from, cap, flag;
	int turn = board.turn;
	int direction = board.turn == ColourWhite ? -16 : 16;
	unsigned int r, rights = board.castle_rights;
	unsigned int my_rights = GET_RIGHTS(turn);
	int psuedo_valid_king = 0;
	int psuedo_valid_queen = 0;
	int one_step_valid_king = 0;
	int one_step_valid_queen = 0;
	int * location = &(board.piece_locations[turn][0]);
	
	while (*location != -1){
		from = *location;
		switch(PIECE_TYPE(board.squares[from])){
			
			case KnightFlag:
				to = from - 33; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from - 31; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from - 18; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from - 14; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 14; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 18; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 31; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				
				to = from + 33; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
				break;
			
			case BishopFlag:
				to = from; cap = Empty;
				while(cap == Empty){
					to += -17; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += -15; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}	
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += 15; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}	
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += 17; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}				
				break;
				
			case RookFlag:
				r = 0;
				if (KING_HAS_RIGHTS(turn) && IS_RIGHT_ROOK(turn,from))
					r |= CREATE_KING_RIGHTS(turn);
				if (QUEEN_HAS_RIGHTS(turn) && IS_LEFT_ROOK(turn,from))
					r |= CREATE_QUEEN_RIGHTS(turn);
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += -16; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,r);
					else break;
				}	
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += -1; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,r);
					else break;
				}		
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += 1; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,r);
					else break;
				}				
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += 16; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,r);
					else break;
				}
				break;
				
			case QueenFlag:
				to = from; cap = Empty;
				while(cap == Empty){
					to += -17; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += -15; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += 15; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += 17; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += -16; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}	
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += -1; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}	
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += 1; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}
				
				to = from; cap = Empty;
				while(cap == Empty){
					to += 16; cap = board.squares[to];
					if (IS_EMPTY_OR_ENEMY(cap,turn))
						list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,0);
					else break;
				}				
				break;
				
			case KingFlag:
				psuedo_valid_king = 0;
				psuedo_valid_queen = 0;
				one_step_valid_king = 0;
				one_step_valid_queen = 0;
			
				if (is_not_in_check(turn)){					
					if (KING_HAS_RIGHTS(turn))
						if (IS_EMPTY(board.squares[from+1]))
							if (IS_EMPTY(board.squares[from+2]))
								if (PIECE_IS_ROOK(board.squares[from+3]))
									psuedo_valid_king = 1;	
								
					if (QUEEN_HAS_RIGHTS(turn))
						if (IS_EMPTY(board.squares[from-1]))
							if (IS_EMPTY(board.squares[from-2]))
								if (IS_EMPTY(board.squares[from-3]))
									if (PIECE_IS_ROOK(board.squares[from-4]))
										psuedo_valid_queen = 1;
				}
				
				to = from - 17; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,my_rights);
				
				to = from - 16; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,my_rights);
				
				to = from - 15; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,my_rights);
				
				to = from - 1; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn)){
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,my_rights);
					if (psuedo_valid_queen){
						apply_move(list[(*size)-1]);
						one_step_valid_queen = is_not_in_check(!(board.turn));
						revert_move(list[(*size)-1]);
						
						if (one_step_valid_queen)
							list[(*size)++] = MAKE_CASTLE_MOVE(from,from-2,my_rights);
					}
				}
				
				to = from + 1; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn)){
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,my_rights);
					if (psuedo_valid_king){
						apply_move(list[(*size)-1]);
						one_step_valid_king = is_not_in_check(!(board.turn));
						revert_move(list[(*size)-1]);
						
						if (one_step_valid_king)
							list[(*size)++] = MAKE_CASTLE_MOVE(from,from+2,my_rights);
					}
				}
				
				to = from + 15; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,my_rights);
				
				to = from + 16; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,my_rights);
				
				to = from + 17; cap = board.squares[to];
				if (IS_EMPTY_OR_ENEMY(cap,turn))
					list[(*size)++] = MAKE_NORMAL_MOVE(from,to,cap,my_rights);
				break;
			
			default: 
				print_board_t();
				int z;
				for(z = 0; z < 16; z++)
					printf("%d ", board.piece_locations[turn][z]);
				printf("\n");
				for(z = 0; z < 16; z++)
					printf("%d ", board.pawn_locations[turn][z]);
				printf("\n");
				assert("Error in board_locations[][]" == 0);
		}
		
		location++;
	}
	
	location = &(board.pawn_locations[turn][0]);
	
	while (*location != -1){
		from = *location;
		int promo = 0;
		
		if (turn == ColourWhite && from / 16 == 5)
			promo = 1;
		else if(turn == ColourBlack && from / 16 == 10)
			promo = 1;
		
		to = from + direction;
		if (IS_EMPTY(board.squares[to])){
			
			if (promo){
				for(flag = PromoteQueenFlag; flag >= PromoteKnightFlag; flag = flag >> 1)
					list[(*size)++] = MAKE_PROMOTION_MOVE(from,to,Empty,flag);
			}
				
			else
				list[(*size)++] = MAKE_NORMAL_MOVE(from,to,Empty,0);
			
			to = to + direction;
			if (IS_EMPTY(board.squares[to]) && ((from/16) + (5 * (turn)) == 10))
				list[(*size)++] = MAKE_NORMAL_MOVE(from,to,Empty,0);
			
		}
		
		to = from + direction;
		if (IS_PIECE(board.squares[to+1]) && PIECE_COLOUR(board.squares[to+1]) == !turn){
			
			if (promo){
				for(flag = PromoteQueenFlag; flag >= PromoteKnightFlag; flag = flag >> 1)
					list[(*size)++] = MAKE_PROMOTION_MOVE(from,to+1,board.squares[to+1],flag);
			}
			else
				list[(*size)++] = MAKE_NORMAL_MOVE(from,to+1,board.squares[to+1],0);
		}
		
		if (IS_PIECE(board.squares[to-1]) && PIECE_COLOUR(board.squares[to-1]) == !turn){
				
			if (promo){
				for(flag = PromoteQueenFlag; flag >= PromoteKnightFlag; flag = flag >> 1)
					list[(*size)++] = MAKE_PROMOTION_MOVE(from,to-1,board.squares[to-1],flag);
			}
			else
				list[(*size)++] = MAKE_NORMAL_MOVE(from,to-1,board.squares[to-1],0);
		}
		
		int ep = board.ep_history[board.depth];
		if (PIECE_COLOUR(board.squares[ep]) == !turn){
			if (ep == from-1)
				list[(*size)++] = MAKE_ENPASS_MOVE(from,ep+direction,ep);
			if (ep == from+1)
				list[(*size)++] = MAKE_ENPASS_MOVE(from,ep+direction,ep);
		}
		
		
		location++;
	}
}


void apply_move(move_t move){
	int turn = board.turn;
	int from = MOVE_GET_FROM(move);
	int to = MOVE_GET_TO(move);
	int cap = MOVE_GET_CAPTURE(move);

	board.depth++;
	if (PIECE_IS_PAWN(board.squares[from]) && abs(from-to) == 32)
		board.ep_history[board.depth] = to;
	else
		board.ep_history[board.depth] = 0;
	
	if (MOVE_IS_NORMAL(move)){
		
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(from)][CONVERT_PIECE256_TO_PIECE12(board.squares[from])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(board.squares[from])];
		
		if (cap != Empty)
			board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(board.squares[to])];
		
		if (cap != Empty)
			remove_position(to,!turn);
		
		board.squares[to] = board.squares[from];
		board.squares[from] = Empty;
		
		board.positions[to] = board.positions[from];
		board.positions[from] = -1;
		
		if (PIECE_IS_PAWN(board.squares[to]))
			board.pawn_locations[turn][board.positions[to]] = to;
		else
			board.piece_locations[turn][board.positions[to]] = to;
		
		board.castle_rights ^= MOVE_GET_CASTLE_FLAGS(move);
		board.turn = !(board.turn);
	}
	
	else if (MOVE_IS_CASTLE(move)){
		
		int rfrom = from + ((to-from) == 2 ? 3 : -4);
		int rto = from + ((to-from) == 2 ? 1 : -1);
		
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(from)][CONVERT_PIECE256_TO_PIECE12(board.squares[from])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(board.squares[from])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(rfrom)][CONVERT_PIECE256_TO_PIECE12(board.squares[rfrom])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(rto)][CONVERT_PIECE256_TO_PIECE12(board.squares[rfrom])];
		
		board.squares[to] = board.squares[from];
		board.squares[from] = Empty;
		
		board.positions[to] = board.positions[from];
		board.positions[from] = -1;
		
		board.piece_locations[turn][board.positions[to]] = to;
		
		board.squares[rto] = board.squares[rfrom];
		board.squares[rfrom] = Empty;
		
		board.positions[rto] = board.positions[rfrom];
		board.positions[rfrom] = -1;
		
		board.piece_locations[turn][board.positions[rto]] = rto;
		
		board.castle_rights ^= MOVE_GET_CASTLE_FLAGS(move);
		board.turn = !(board.turn);
	} 
	
	else if (MOVE_IS_PROMOTION(move)){
		
		int promo = MOVE_GET_PROMOTE_TYPE(move,turn);
		
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(from)][CONVERT_PIECE256_TO_PIECE12(board.squares[from])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(promo)];
		
		if (cap != Empty)
			board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(board.squares[to])];
		
		if (cap != Empty)
			remove_position(to,!turn);
		
		board.positions[to] = -1;
		
		board.squares[to] = promo;
		insert_position(to,turn);
		
		remove_position(from,turn);
		board.squares[from] = Empty;
		board.positions[from] = -1;
		
		board.turn = !(board.turn);		
	}
	
	else if (MOVE_IS_ENPASS(move)){
	
		int enpass = MOVE_GET_ENPASS_SQUARE(move);
		
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(from)][CONVERT_PIECE256_TO_PIECE12(board.squares[from])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(board.squares[from])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(enpass)][CONVERT_PIECE256_TO_PIECE12(board.squares[enpass])];
		
		board.squares[to] = board.squares[from];
		board.squares[from] = Empty;
		
		board.positions[to] = board.positions[from];
		board.positions[from] = -1;
		
		board.pawn_locations[turn][board.positions[to]] = to;
		
		
		
		remove_position(enpass,!turn);
		board.squares[enpass] = Empty;
		board.positions[enpass] = -1;
		
		board.turn = !(board.turn);
	}
	
	board.hash_history[board.hash_entries++] = board.hash;

	
}

void revert_move(move_t move){
	int turn = board.turn;
	int from = MOVE_GET_FROM(move);
	int to = MOVE_GET_TO(move);
	int cap = MOVE_GET_CAPTURE(move);
	
	board.ep_history[board.depth] = 0;
	board.depth--;
	
	if (MOVE_IS_NORMAL(move)){
		
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(from)][CONVERT_PIECE256_TO_PIECE12(board.squares[to])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(board.squares[to])];
		
		if (cap != Empty)
			board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(cap)];
			
		
		board.squares[from] = board.squares[to];
		board.squares[to] = cap;
		
		board.positions[from] = board.positions[to];
		board.positions[to] = -1;
		
		if (PIECE_IS_PAWN(board.squares[from]))
			board.pawn_locations[!turn][board.positions[from]] = from;
		else
			board.piece_locations[!turn][board.positions[from]] = from;
		
		if (cap != Empty)
			insert_position(to,turn);
		
		board.castle_rights ^= MOVE_GET_CASTLE_FLAGS(move);
		board.turn = !(board.turn);
	}
	
	else if (MOVE_IS_CASTLE(move)){
		
		int rfrom = from + ((to-from) == 2 ? 3 : -4);
		int rto = from + ((to-from) == 2 ? 1 : -1);
		
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(from)][CONVERT_PIECE256_TO_PIECE12(board.squares[to])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(board.squares[to])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(rfrom)][CONVERT_PIECE256_TO_PIECE12(board.squares[rto])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(rto)][CONVERT_PIECE256_TO_PIECE12(board.squares[rto])];
		
		board.squares[from] = board.squares[to];
		board.squares[to] = Empty;
		
		board.positions[from] = board.positions[to];
		board.positions[to] = -1;
		
		board.piece_locations[!turn][board.positions[from]] = from;
		
		
		
		board.squares[rfrom] = board.squares[rto];
		board.squares[rto] = Empty;
		
		board.positions[rfrom] = board.positions[rto];
		board.positions[rto] = -1;
		
		board.piece_locations[!turn][board.positions[rfrom]] = rfrom;
		
		board.castle_rights ^= MOVE_GET_CASTLE_FLAGS(move);
		board.turn = !(board.turn);
	} 
	
	else if (MOVE_IS_PROMOTION(move)){
		
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(MOVE_GET_PROMOTE_TYPE(move,!turn))];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(from)][CONVERT_PIECE256_TO_PIECE12(PawnFlag + !turn)];
		
		if (cap != Empty)
			board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(cap)];
		
		board.squares[from] = PawnFlag + !turn;
		insert_position(from,!turn);
		
		remove_position(to,!turn);
		board.squares[to] = cap;		
		board.positions[to] = -1;
		if (cap != Empty)
			insert_position(to,turn);
		
		board.turn = !(board.turn);
		
	}
	
	else if (MOVE_IS_ENPASS(move)){
		
		int enpass = MOVE_GET_ENPASS_SQUARE(move);
		
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(from)][CONVERT_PIECE256_TO_PIECE12(board.squares[to])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(to)][CONVERT_PIECE256_TO_PIECE12(board.squares[to])];
		board.hash ^= zorbist.bitstrings[CONVERT_256_TO_64(enpass)][CONVERT_PIECE256_TO_PIECE12(PawnFlag + turn)];
		
		board.squares[from] = board.squares[to];
		board.squares[to] = Empty;
		
		board.positions[from] = board.positions[to];
		board.positions[to] = -1;
		
		board.pawn_locations[!turn][board.positions[from]] = from;
		
		board.squares[enpass] = PawnFlag + turn;
		board.positions[enpass] = -1;
		insert_position(enpass,turn);
		
		board.turn = !(board.turn);
	}
	
	board.hash_entries--;
}

void insert_position(int to, int turn){
	if (PIECE_IS_PAWN(board.squares[to])){
		board.pawn_locations[turn][board.pawn_counts[turn]] = to;
		board.positions[to] = board.pawn_counts[turn];
		board.pawn_counts[turn] += 1;
	} else {
		board.piece_locations[turn][board.piece_counts[turn]] = to;
		board.positions[to] = board.piece_counts[turn];
		board.piece_counts[turn] += 1;
	}
}

void remove_position(int to, int turn){
	int i;
	if (PIECE_IS_PAWN(board.squares[to])){
		for(i = board.positions[to]; i < board.pawn_counts[turn]; i++){
			board.positions[board.pawn_locations[turn][i]] -= 1;
			board.pawn_locations[turn][i] = board.pawn_locations[turn][i+1];
		}
		board.pawn_counts[turn] -= 1;
	} else {
		for(i = board.positions[to]; i < board.piece_counts[turn]; i++){
			board.positions[board.piece_locations[turn][i]] -= 1;
			board.piece_locations[turn][i] = board.piece_locations[turn][i+1];
		}
		board.piece_counts[turn] -= 1;
	}
}

int square_is_attacked(int turn, int square){
	int i, tile, inc, *curr;
	int * sq = &(board.squares[square]);
	int * kincptr = &(king_movements[0]);
	int * nincptr = &(knight_movements[0]);
	int pawn_inc = -16 + (turn<<5);
	
	tile = *(sq+pawn_inc+1);
	if (PIECE_IS_PAWN(tile) && PIECE_COLOUR(tile) == !turn)
		return 1;
	
	tile = *(sq+pawn_inc-1);
	if (PIECE_IS_PAWN(tile) && PIECE_COLOUR(tile) == !turn)
		return 1;
	
	for(i = 0; i < 8; i++){
		tile = *(sq+*nincptr);
		if (PIECE_IS_KNIGHT(tile) && PIECE_COLOUR(tile) == !turn)
			return 1;
		nincptr++;
	}
	
	for(i = 0; i < 4; i++){
		inc = *kincptr;
		curr = sq + inc;
		
		if (PIECE_IS_KING(*curr))
			return 1;
		
		while (1){
			tile = *curr;
			curr += inc;
			if (IS_EMPTY(tile))
				continue;
			if (IS_WALL(tile))
				break;
			if (PIECE_IS_BISHOP_OR_QUEEN(tile) && PIECE_COLOUR(tile) == !turn)
				return 1;
			break;
		}
		
		kincptr++;
	}
	
	for(; i < 8; i++){
		inc = *kincptr;
		curr = sq + inc;
		
		if (PIECE_IS_KING(*curr))
			return 1;
		
		while (1){
			tile = *curr;
			curr += inc;
			if (IS_EMPTY(tile))
				continue;
			if (IS_WALL(tile))
				break;
			if (PIECE_IS_ROOK_OR_QUEEN(tile) && PIECE_COLOUR(tile) == !turn)
				return 1;
			break;
		}
		
		kincptr++;
	}
	
	return 0;
}
