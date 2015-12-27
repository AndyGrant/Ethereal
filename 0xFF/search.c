#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "board.h"
#include "colour.h"
#include "evaluate.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "types.h"
#include "util.h"

time_t StartTime, EndTime;
int EvaluatingPlayer;

move_t get_best_move(board_t * board, int t){
	int depth, i, size = 0; 	
	
	StartTime = time(NULL);
	EndTime = StartTime + 5;
	EvaluatingPlayer = board->turn;
	
	clock_t start = clock();
	
	move_t moves[MaxMoves];
	gen_all_legal_moves(board,moves,&size);
	
	search_tree_t tree;
	init_search_tree_t(&tree,board);
	
	for(depth = 2; depth <= 5; depth++){
		
		int rnodes = tree.raw_nodes;
		int anodes = tree.alpha_beta_nodes;
		int qnodes = tree.quiescence_nodes;
		
		int value = alpha_beta_prune(&tree,&(tree.principle_variation),depth,-CheckMate,CheckMate);
		
		printf("Search Depth        : %d\n",depth);
		printf("MicroPawns          : %c%.2f",value >= 0, value/100.0);
		printf("Raw Nodes           : %d\n",tree.raw_nodes - rnodes);
		printf("Alpha Nodes         : %d\n",tree.alpha_beta_nodes - anodes);
		printf("Quiescence Nodes    : %d\n",tree.quiescence_nodes - qnodes);
		
		printf("Principle Variation : ");
		for(i = 0; i < tree.principle_variation.length; i++){
			print_move_t(tree.principle_variation.line[i]);
			printf(" -> ");
		}
		printf("\n\n");
	}
	
	printf("TIME TAKEN %d\n",((int)clock()-(int)start)/CLOCKS_PER_SEC);
	
	return tree.principle_variation.line[0];
	
	
}

void init_search_tree_t(search_tree_t * tree, board_t * board){
	tree->ply = 0;
	tree->raw_nodes = 0;
	tree->alpha_beta_nodes = 0;
	tree->quiescence_nodes = 0;
	memcpy(&(tree->board),board,sizeof(*board));
	tree->principle_variation.length = 0;
}

int alpha_beta_prune(search_tree_t * tree, principle_variation_t * pv, int depth, int alpha, int beta){
	
	tree->raw_nodes++;
	tree->alpha_beta_nodes++;
	
	principle_variation_t lpv;
	board_t * board = &(tree->board);
	
	if (EndTime < time(NULL)){
		pv->length = -1;
		return tree->board.turn == EvaluatingPlayer ? -CheckMate : CheckMate;
	}
	
	if (depth == 0){
		pv->length = 0;
		return quiescence_search(tree,alpha,beta);
	}
	
	tree->ply++;
	
	int size = 0;
	move_t moves[MaxMoves];
	gen_all_moves(&(tree->board),&(moves[0]),&size);
	basic_heuristic(board,&(moves[0]),size);
	
	int i, value, best = -99999;
	for (i = 0; i < size; i++){
		apply_move(board,moves[i]);
		if (is_not_in_check(board,!board->turn)){
			
			value = -alpha_beta_prune(tree,&lpv,depth-1,-beta,-alpha);

			if (value > best)	
				best = value;
			
			if (best > alpha){
				alpha = best;
				
				if (lpv.length != -1){
					pv->line[0] = moves[i];
					memcpy(pv->line + 1, lpv.line, sizeof(move_t) * lpv.length);
					pv->length = lpv.length + 1;
				}
			}
			
			if (alpha > beta)	{revert_move(board,moves[i]); break;}
		}
		revert_move(board,moves[i]);
	}
	
	tree->ply--;
	return best;
}

int quiescence_search(search_tree_t * tree, int alpha, int beta){
	
	tree->raw_nodes++;
	tree->quiescence_nodes++;
	
	if (EndTime < time(NULL))
		return tree->board.turn == EvaluatingPlayer ? -CheckMate : CheckMate;
	
	board_t * board = &(tree->board);
	
	tree->ply++;
	
	int size = 0;
	move_t moves[MaxMoves];
	gen_all_captures(&(tree->board),&(moves[0]),&size);
	
	basic_heuristic(board,&(moves[0]),size);
	
	int i, value = -100000, best = evaluate_board(board);
	for (i = size-1; i >= 0; i--){
		apply_move(board,moves[i]);
		if (is_not_in_check(board,!board->turn)){
			value = -quiescence_search(tree,-beta,-alpha);
			
			if (value > best)	best = value;
			if (best > alpha)	alpha = best;
			if (alpha > beta)	{revert_move(board,moves[i]); break;}
		}
		revert_move(board,moves[i]);
	}
	
	tree->ply--;
	return best;
}

int evaluate_board(board_t * board){
	int value = 0;
	int turn = board->turn;
	int * location;
	
	for(location = &(board->piece_locations[turn][1]); *location != -1; location++){
		switch(PIECE_TYPE(board->squares[*location])){
			case QueenFlag: 	value += (QueenValue 	+ 3 * QUEEN_POSITION_VALUE(*location)); 	break;
			case RookFlag: 		value += (RookValue 	+ 3 * ROOK_POSITION_VALUE(*location)); 		break;
			case BishopFlag: 	value += (BishopValue 	+ 3 * BISHOP_POSITION_VALUE(*location)); 	break;
			case KnightFlag: 	value += (KnightValue 	+ 3 * KNIGHT_POSITION_VALUE(*location)); 	break;
		}
	}
	
	for(location = &(board->piece_locations[!turn][1]); *location != -1; location++){
		switch(PIECE_TYPE(board->squares[*location])){
			case QueenFlag: 	value -= (QueenValue 	+ 3 * QUEEN_POSITION_VALUE(*location)); 	break;
			case RookFlag: 		value -= (RookValue 	+ 3 * ROOK_POSITION_VALUE(*location)); 		break;
			case BishopFlag: 	value -= (BishopValue 	+ 3 * BISHOP_POSITION_VALUE(*location)); 	break;
			case KnightFlag: 	value -= (KnightValue 	+ 3 * KNIGHT_POSITION_VALUE(*location)); 	break;
		}
	}
	
	value += PawnValue * (board->pawn_counts[turn] - board->pawn_counts[!turn]);
	
	int temp[3] = {0, 3, 7};
	for(location = &(board->pawn_locations[turn][0]); *location != -1; location++){
		if (turn == ColourWhite){
			int supports = board->squares[*location+17] == WhitePawn; 
				supports += board->squares[*location+15] == WhitePawn;
			value += temp[supports];
		} else if (turn == ColourBlack){
			int supports = board->squares[*location-17] == BlackPawn; 
				supports += board->squares[*location-15] == BlackPawn;
			value += temp[supports];
		}
	}
	
	for(location = &(board->pawn_locations[!turn][0]); *location != -1; location++){
		if (turn == ColourWhite){
			int supports = board->squares[*location+17] == WhitePawn; 
				supports += board->squares[*location+15] == WhitePawn;
			value -= temp[supports];
		} else if (turn == ColourBlack){
			int supports = board->squares[*location-17] == BlackPawn; 
				supports += board->squares[*location-15] == BlackPawn;
			value -= temp[supports];
		}
	}
	
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