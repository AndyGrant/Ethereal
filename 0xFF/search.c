#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "colour.h"
#include "evaluate.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "types.h"
#include "util.h"

int main2(){
	board_t board;
	init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	get_best_move(&board,1000);
}

move_t get_best_move(board_t * board, int time){
	
	int depth, i, size = 0; 
	int alpha, beta;
	
	clock_t start = clock();
	
	move_t moves[MaxMoves];
	gen_all_legal_moves(board,moves,&size);
	
	int values[size];
	
	search_tree_t tree;
	init_search_tree_t(&tree,board);
	
	for(depth = 2; depth <= 4; depth++){
		alpha = -99999; 
		beta = 99999;
		int last_depth_alpha = tree.nodes_searched;
		int last_depth_quiescence = tree.quiescence_nodes;
		int last_move_alpha_nodes = tree.nodes_searched;
		int last_move_quiescence_nodes = tree.quiescence_nodes;
		
		printf("SEARCHING DEPTH %d \n",depth);
		for(i = 0; i < size; i++){			
			apply_move(&(tree.board),moves[i]);
			values[i] = -alpha_beta_prune(&tree, depth, -beta, -alpha);
			revert_move(&(tree.board),moves[i]);			
			
			printf("  Move #%d\t Value=%d\t AlphaN=%d\t QuiescenceN=%d\n",i,values[i],tree.nodes_searched-last_move_alpha_nodes,tree.quiescence_nodes-last_move_quiescence_nodes);
			last_move_alpha_nodes = tree.nodes_searched;
			last_move_quiescence_nodes = tree.quiescence_nodes;
			
			if (values[i] > alpha)
				alpha = values[i];
			
			if (alpha == CheckMate)
				return moves[i];
		}
		
		order_by_value(&(moves[0]),&(values[0]),size);
		
		printf("ALPHABETA NODES %d QUIESCENCE NODES %d \n\n",tree.nodes_searched-last_depth_alpha,tree.quiescence_nodes-last_depth_quiescence);
	}
	
	return moves[0];
	
	printf("TIME TAKEN %d\n",((int)clock()-(int)start)/CLOCKS_PER_SEC);
}

void init_search_tree_t(search_tree_t * tree, board_t * board){
	tree->ply = 0;
	tree->nodes_searched = 0;
	tree->quiescence_nodes = 0;
	memcpy(&(tree->board),board,sizeof(*board));
}

int alpha_beta_prune(search_tree_t * tree, int depth, int alpha, int beta){
	tree->nodes_searched++;
	board_t * board = &(tree->board);
	
	if (depth == 0)
		return quiescence_search(tree,alpha,beta);
	
	tree->ply++;
	
	int size = 0;
	move_t moves[MaxMoves];
	gen_all_moves(&(tree->board),&(moves[0]),&size);
	
	basic_heuristic(board,&(moves[0]),size);
	
	int value, best = -99999;
	for(size -= 1; size >= 0; size--){
		apply_move(board,moves[size]);
		if (is_not_in_check(board,!board->turn)){
			value = -alpha_beta_prune(tree,depth-1,-beta,-alpha);
			
			if (value > best)	best = value;
			if (best > alpha)	alpha = best;
			if (alpha > beta)	{revert_move(board,moves[size]); break;}
		}
		revert_move(board,moves[size]);
	}
	
	tree->ply--;
	return best;
}

int quiescence_search(search_tree_t * tree, int alpha, int beta){
	tree->quiescence_nodes++;
	board_t * board = &(tree->board);
	
	tree->ply++;
	
	int size = 0;
	move_t moves[MaxMoves];
	gen_all_moves(&(tree->board),&(moves[0]),&size);
	
	int value = -100000, best = -99999;
	for(size -= 1; size >= 0; size--){
		if (MOVE_IS_NORMAL(moves[size]) && !IS_EMPTY(MOVE_GET_CAPTURE(moves[size]))){
			apply_move(board,moves[size]);
			if (is_not_in_check(board,!board->turn)){
				value = -quiescence_search(tree,-beta,-alpha);
				
				if (value > best)	best = value;
				if (best > alpha)	alpha = best;
				if (alpha > beta)	{revert_move(board,moves[size]); break;}
			}
			revert_move(board,moves[size]);
		}
	}
	
	tree->ply--;
	return value == -100000 ? evaluate_board(board) : best;
}

int evaluate_board(board_t * board){
	int value = 0;
	int turn = board->turn;
	int * location;
	
	for(location = &(board->piece_locations[turn][1]); *location != -1; location++){
		switch(PIECE_TYPE(board->squares[*location])){
			case QueenFlag: 	value += (QueenValue 	+ QUEEN_POSITION_VALUE(*location)); 	break;
			case RookFlag: 		value += (RookValue 	+ ROOK_POSITION_VALUE(*location)); 		break;
			case BishopFlag: 	value += (BishopValue 	+ BISHOP_POSITION_VALUE(*location)); 	break;
			case KnightFlag: 	value += (KnightValue 	+ KNIGHT_POSITION_VALUE(*location)); 	break;
		}
	}
	
	for(location = &(board->piece_locations[!turn][1]); *location != -1; location++){
		switch(PIECE_TYPE(board->squares[*location])){
			case QueenFlag: 	value -= (QueenValue 	+ QUEEN_POSITION_VALUE(*location)); 	break;
			case RookFlag: 		value -= (RookValue 	+ ROOK_POSITION_VALUE(*location)); 		break;
			case BishopFlag: 	value -= (BishopValue 	+ BISHOP_POSITION_VALUE(*location)); 	break;
			case KnightFlag: 	value -= (KnightValue 	+ KNIGHT_POSITION_VALUE(*location)); 	break;
		}
	}
	
	value += PawnValue * (board->pawn_counts[turn] - board->pawn_counts[!turn]);
	
	return value;
}

void order_by_value(move_t * moves, int * values, int size){
	int i, j, temp_value;
	move_t temp_move;
	
	for(i = 0; i < size; i++){
		for(j = i+1; j < size; j++){
			if (values[j] > values[i]){
				temp_value = values[j];
				values[j] = values[i];
				values[i] = temp_value;
				
				temp_move = moves[j];
				moves[i] = moves[j];
				moves[j] = temp_move;
			}
		}
	}
}

void basic_heuristic(board_t * board, move_t * moves, int size){	
	int values[size];
	
	int i;
	for(i = 0; i < size; i++){
		int cap = MOVE_GET_CAPTURE(moves[i]);
		values[i] = !IS_EMPTY(cap) * cap;
	}
	
	order_by_value(moves,values,size);
}