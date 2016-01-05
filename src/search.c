#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#include "board.h"
#include "colour.h"
#include "evaluate.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "ttable.h"
#include "types.h"
#include "util.h"

extern board_t board;
extern search_tree_t tree;
extern zorbist_t zorbist;
extern ttable_t table;

time_t StartTime, EndTime;
int EvaluatingPlayer;

move_t get_best_move(int alloted_time){
	int depth, i, size = 0; 	
	alloted_time = 20;
	
	StartTime = time(NULL);
	EndTime = StartTime + alloted_time;
	clock_t start = clock();
	
	EvaluatingPlayer = board.turn;
	
	init_search_tree_t();
	print_board_t();
	
	int old_hash = board.hash;
	
	for(depth = 0; depth < MaxDepth; depth++){
		
		int value = alpha_beta_prune(&(tree.principle_variation),depth,-CheckMate,CheckMate);		
		printf("info depth %d score cp %d time %d nodes %d pv ",depth,(100*value)/PawnValue,1000 * (time(NULL) - StartTime), tree.raw_nodes);
		for(i = 0; i < tree.principle_variation.length; i++){
			print_move_t(tree.principle_variation.line[i]);
			printf(" ");
		}
		printf("\n");
		
 		if (EndTime - ((float)(alloted_time) * .8) < time(NULL))
 			break;	
	}
	
	printf("TIME TAKEN %d\n",((int)clock()-(int)start)/CLOCKS_PER_SEC);
	return tree.principle_variation.line[0];	
}

void init_search_tree_t(){
	tree.ply = 0;
	tree.raw_nodes = 0;
	tree.alpha_beta_nodes = 0;
	tree.quiescence_nodes = 0;
	tree.principle_variation.length = 0;
	memset(&(tree.principle_variation.line[0]),0,sizeof(move_t) * MaxDepth);
	memset(&(tree.killer_moves[0][0]),0,sizeof(move_t) * 3 * MaxDepth);
}

int alpha_beta_prune(principle_variation_t * pv, int depth, int alpha, int beta){
	tree.raw_nodes++;
	tree.alpha_beta_nodes++;
	
	principle_variation_t lpv;
	lpv.length = 0;
	
	if (EndTime < time(NULL)){
		pv->length = -1;
		return board.turn == EvaluatingPlayer ? -CheckMate : CheckMate;
	}
	
	if (depth == 0){
		tree.raw_nodes--;
		tree.alpha_beta_nodes--;
		return quiescence_search(alpha,beta);
	}	

	tree.ply++;
	
	int valid_size = 0, size = 0;
	move_t moves[MaxMoves];
	gen_all_moves(&(moves[0]),&size);
	int values[size];
	basic_heuristic(&(values[0]),&(moves[0]),size);
	
	int i, index, value, best = -CheckMate - depth;
	
	for(index = 0; index < size; index++){
		i = get_best_next_index(&(values[0]),size);
		apply_move(moves[i]);
		
		if(is_not_in_check(!board.turn)){
			valid_size++;
			// Determine three-fold repitition
			int z, repititions = 0;
			for(z = board.hash_entries-1; z >= 0; z-=2)
				if (board.hash_history[z] == board.hash)
					repititions++;
			
			// Determine Value Of Node 
			if (repititions >= 3){
				value = 0;
			} else if (i == 0){
				value = -alpha_beta_prune(&lpv,depth-1,-beta,-alpha);
			} else if (i >= 2 && depth >= 2 &&  IS_EMPTY(MOVE_GET_CAPTURE(moves[i]))){
				if (depth >= 6) value = -alpha_beta_prune(&lpv,depth-3,-beta,-alpha);
				else			value = -alpha_beta_prune(&lpv,depth-2,-beta,-alpha);
				
				if (value > alpha)
					value = -alpha_beta_prune(&lpv,depth-1,-beta,-alpha);
			} else {
				value = -alpha_beta_prune(&lpv,depth-1,-alpha-1,-alpha);
				if (value > alpha && value < beta)
					value = -alpha_beta_prune(&lpv,depth-1,-beta,-alpha);
			}
			
			if (value > best){
				best = value;
				if (best > alpha){
					alpha = best;
					if (lpv.length != -1){
						pv->line[0] = moves[i];
						memcpy(pv->line + 1, lpv.line, sizeof(move_t) * lpv.length);
						pv->length = lpv.length + 1;
					}
				}
			}
			
			if (alpha > beta){
				update_killer_heuristic(moves[i]);
				revert_move(moves[i]); 
				break;
			}
		}
		
		revert_move(moves[i]);
	}
	
	// If Legal Moves Were Found And In Check Return DRAW
	if (valid_size == 0 && is_not_in_check(board.turn))
		best = 0;
	
	tree.ply--;
	return best;
}

int quiescence_search(int alpha, int beta){
	int index, i, size = 0, value, best;
	
	tree.ply++;
	tree.raw_nodes++;
	tree.quiescence_nodes++;
	
	best = evaluate_board();
	if (best > alpha) alpha = best;
	if (alpha > beta) {tree.ply--; return best;}
	
	move_t moves[MaxMoves];
	gen_all_captures(&(moves[0]),&size);
	int values[size];
	quiescence_heuristic(&(values[0]),&(moves[0]),size);	
	
	for(index = 0; index < size; index++){
		i = get_best_next_index(&(values[0]),size);
		apply_move(moves[i]);	
		if (is_not_in_check(!board.turn)){
			value = -quiescence_search(-beta,-alpha);
			if (value > best){	best = value;}
			if (best > alpha)	alpha = best;
			if (alpha > beta){
				update_killer_heuristic(moves[i]);
				revert_move(moves[i]); 
				break;
			}
		}
		revert_move(moves[i]);
	}
	
	tree.ply--;
	return best;
}

void basic_heuristic(int * values, move_t * moves, int size){	
	move_t * arr = &(tree.killer_moves[tree.ply][0]);
	int i;
	
	for(i = 0; i < size; i++){
		int cap = MOVE_GET_CAPTURE(moves[i]);
		
		if (!IS_EMPTY(cap))		values[i] = cap - board.squares[MOVE_GET_FROM(moves[i])];
		else					values[i] = -board.squares[MOVE_GET_FROM(moves[i])];
		
		int sq64 = CONVERT_256_TO_64(MOVE_GET_TO(moves[i]));
		int truesq = board.turn == ColourWhite ? sq64 : inv[sq64];
		int is_endgame  = (board.piece_counts[0] + board.piece_counts[1] <= 7);
		
		switch(board.squares[MOVE_GET_FROM(moves[i])] & ~1){
			case WhitePawn:
				if (is_endgame) values[i] += PawnEndValueMap[truesq];
				else			values[i] += PawnEarlyValueMap[truesq];
			
			case WhiteKnight:
				values[i] += KnightValueMap[truesq];
			
			case WhiteBishop:
				values[i] += BishopValueMap[truesq];
		}
				
		if (moves[i] == tree.principle_variation.line[tree.ply-1])
			values[i] += 300000;
		else if (arr[0] == moves[i])
			values[i] += 25000;
		else if (arr[1] == moves[i])
			values[i] += 25000;
		else if (arr[2] == moves[i])
			values[i] += 25000;
	}
}

void quiescence_heuristic(int * values, move_t * moves, int size){
	move_t * arr = &(tree.killer_moves[tree.ply][0]);
	int i;
	for(i = 0; i < size; i++){
		values[i] =  40 * MOVE_GET_CAPTURE(moves[i]) - board.squares[MOVE_GET_FROM(moves[i])];
		if (arr[0] == moves[i])
			values[i] += 5000;
		else if (arr[1] == moves[i])
			values[i] += 5000;
		else if (arr[2] == moves[i])
			values[i] += 5000;
	}
}

int get_best_next_index(int * values, int size){
	int index, bestindex;
	for(index = 0, bestindex = 0; index < size; index++){
		if (values[bestindex] < values[index])
				bestindex = index;
	}
	values[bestindex] = -CheckMate;
	return bestindex;
}

void update_killer_heuristic(move_t move){
	move_t * arr = &(tree.killer_moves[tree.ply][0]);
	
	if (move != arr[2] && move != arr[1] && move != arr[0]){
		arr[2] = arr[1];
		arr[1] = arr[0];
		arr[0] = move;
	}
}