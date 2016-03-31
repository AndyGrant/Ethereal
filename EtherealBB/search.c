#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "castle.h"
#include "magics.h"
#include "piece.h"
#include "search.h"
#include "types.h"
#include "move.h"
#include "movegen.h"
#include "movegentest.h"
#include "zorbist.h"


time_t StartTime;
time_t EndTime;

long NodesSearched;
int EvaluatingPlayer;

uint16_t get_best_move(Board * board, int seconds){
	
	int value, depth, i, j, size=0;
	
	
	StartTime = time(NULL);
	EndTime = StartTime + seconds;
	
	NodesSearched = 0;
	EvaluatingPlayer = board->turn;
	
	PrincipleVariation PV;
	PV.length = 0;
	
	clock_t start = clock();
	
	printf("Starting Search, alloted_time=%d\n",seconds);
	print_board(board);
	
	for (depth = 0; depth < MaxDepth; depth++){
		value = alpha_beta_prune(board,-Mate,Mate,depth,0,&PV);
		
		printf("depth %d score %d Nodes %d Seconds %d\n",depth,value,NodesSearched,time(NULL)-StartTime);
		
		printf("PV = ");
		for (j = 0; j < PV.length; j++){
			printf(" -> ");
			print_move(PV.line[j]);
		}
		printf("\n\n");
		
		if (time(NULL) - StartTime > seconds)
			break;
	}
	
	return PV.line[0];	
}

int alpha_beta_prune(Board * board, int alpha, int beta, int depth, int height, PrincipleVariation * PV){
	
	// Alloted Time has Expired
	if (EndTime < time(NULL))
		return board->turn == EvaluatingPlayer ? -Mate : Mate;
	
	// Max Depth Reached
	if (depth == 0)
		return EvaluatingPlayer == ColourWhite ? board->opening : -board->opening;
	
	// Max Height Reached
	if (height >= MaxHeight)
		return EvaluatingPlayer == ColourWhite ? board->opening : -board->opening;
	
	// Updated Node Counter
	NodesSearched += 1;
	
	// Create Local Principle Variation
	PrincipleVariation LPV;
	LPV.length = 0;

	// Storage to use moves
	Undo undo[1];
	int i, j, size = 0;
	uint16_t moves[256];
	
	// Storage for search outputs
	int best = -Mate, value;
	
	gen_all_moves(board,moves,&size);
	
	for (i = 0; i < size; i++){
		apply_move(board,moves[i],undo);
		
		// Ensure move is Legal
		if (!is_not_in_check(board,!board->turn)){
			revert_move(board,moves[i],undo);
			continue;
		}
		
		value = -alpha_beta_prune(board,-beta,-alpha,depth-1,height+1,&LPV);
		
		revert_move(board,moves[i],undo);
		
		
		// Update Search Bounds
		if (value > best){				
			best = value;
			
			if (best > alpha){
				alpha = best;
				
				// Improvement Found, Update Principle Variation
				PV->line[0] = moves[i];
				memcpy(PV->line + 1, LPV.line, sizeof(uint16_t) * LPV.length);
				PV->length = LPV.length + 1;	
			}
		}
		
		if (alpha > beta)
			return value;
	}
	
	return best;	
}