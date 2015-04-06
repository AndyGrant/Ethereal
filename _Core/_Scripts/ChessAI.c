#include <stdio.h>
#include <stdlib.h>
#include "Engine.h"
#include "Moves.h"
#include "ChessAI.h"
#include "TranspositionTable.h"

int MATE = 99999;
int MATERIAL_VALUES[6] = {100,300,300,600,1000,10000};

int VALUE_KNIGHT_RANGE = 3;
int VALUE_BISHOP_RANGE = 3;
int VALUE_CENTER_SQUARE_ATTACKED = 8;
int VALUE_KING_SURROUNDINGS_ATTACKED = 4;

int TOTAL_BOARDS_SEARCHED = 0;

int DEPTH = 4;
int HEURISTIC_DEPTH = 2;
int ORIGINAL_PLAYER;

int TOTAL_MOVES_FOUND = 0;

int USE_GOOD_HEURISTIC = 1;
int USE_BAD_HEURISTIC = 1;


int findBestMoveIndex(struct Board * board, int * last_move, int turn){
	int size = 0;
	int * moves = findAllValidMoves(board,turn,&size,last_move);
	
	if (size == 0)
		return -1;
	
	if (USE_GOOD_HEURISTIC == 1)
		moves = goodHeuristic(board,size,moves,turn);
		
	int * moves_pointer = moves;
	
	int values[size];
	int alpha = -MATE - 1;
	int beta = MATE + 1;
	int i, temp;
	
	TOTAL_MOVES_FOUND += size;
	for(i = 0; i < size; i++){
		temp = TOTAL_BOARDS_SEARCHED;
		values[i] = alphaBetaPrune(board,!turn,moves,DEPTH,alpha,beta,turn);
		if (values[i] > alpha)
			alpha = values[i];
		moves += 7;
		printf("#%d \t Value: %d \t Alpha: %d \t Searched: %d \n",i,values[i],alpha,TOTAL_BOARDS_SEARCHED-temp);
		//printf("Alpha %d Evaluated Move #%d to %d \t %d Board(s) \n",alpha,i,values[i],TOTAL_BOARDS_SEARCHED-temp);
	}
	
	printf("Total Evals %d \n",TOTAL_BOARDS_SEARCHED);
	printf("Total Moves Found %d \n \n", TOTAL_MOVES_FOUND);
	
	int best_index = 0;
	for(i = 1; i < size; i++)
		if (values[i] > values[best_index])
			best_index = i;
	
	int * best = malloc(28);
	for(i = 0;  i < 7; i++)
		best[i] = moves_pointer[(best_index*7) + i];
	free(moves_pointer);
	
	int size_unsorted = 0;
	int * unsorted = findAllValidMoves(board,turn,&size_unsorted,last_move);
	
	int j;
	int index[4] = {0,1,2,6};
	for(i = 0; i < size; i++){
		for(j = 0; j < 4; j++){
			if (unsorted[i*7+index[j]] != best[index[j]])
				j = 7;
			else if (j == 3){
				free(unsorted);
				return i;
			}
		}
	}
	
	return -1;
}

int alphaBetaPrune(struct Board * board, int turn, int * move, int depth, int alpha, int beta, int evaluating_player){	

	applyGenericMove(board,move);	
	if (depth == 0){
		int value = evaluateBoard(board,evaluating_player,move);
		revertGenericMove(board,move);
		return value;
	}
	
	int size = 0;
	int * moves = findAllValidMoves(board,turn,&size,move);
	
	if (USE_BAD_HEURISTIC == 1)
		moves = weakHeuristic(board,size,moves,turn);
		
	int * moves_pointer = moves;
	TOTAL_MOVES_FOUND += size;
	
	if (size == 0){
		int is_Check_Mate = 0;
		checkMove(board,&is_Check_Mate,turn);
		revertGenericMove(board,move);
		free(moves_pointer);
		if (is_Check_Mate == 0){
			if (turn == evaluating_player)
				return -MATE;
			else
				return MATE;
		}
		else 
			return 0;
	}
	
	int i, v, value;
	
	if (turn == evaluating_player){
		value = -MATE - 1;
		for(i = 0; i < size; i++, moves += 7){
			v = alphaBetaPrune(board,!turn,moves,depth-1,alpha,beta,evaluating_player);
			value = value > v ? value : v;
			alpha = alpha > value? alpha : value;
			if (beta <= alpha)
				break;
		}
	}
	
	else {
		value = MATE + 1;		
		for(i = 0; i < size; i++, moves += 7){
			v = alphaBetaPrune(board,!turn,moves,depth-1,alpha,beta,evaluating_player);
			value = value < v ? value : v;
			beta = beta < value? beta : value;
			if (beta <= alpha)
				break;
		}
	}	
	
	revertGenericMove(board,move);
	free(moves_pointer);
	return value;
}

int evaluateBoard(struct Board *board, int player, int * lastMove){
	
	TOTAL_BOARDS_SEARCHED += 1;
	int value = 	evaluateMaterial(board,player) + 
					evaluateMoves(board,player,lastMove) - 
					evaluateMoves(board,!player,lastMove);
	return value;
}

int evaluateMaterial(struct Board *board, int player){
	int value = 0;
	int x,y;
	for(x = 0; x < 8; x++){
		for(y = 0; y < 8; y++){
			if (board->types[x][y] != 9){
				if (board->colors[x][y] == player)
					value += MATERIAL_VALUES[board->types[x][y]];
				else
					value -= MATERIAL_VALUES[board->types[x][y]];
			}
		}
	}
	
	return value;
}

int evaluateMoves(struct Board *board, int player, int * lastMove){
	int size = 0;
	int * moves = findAllValidMoves(board,player,&size,lastMove);
	int * moves_pointer = moves;
	
	int value = 0;
	value += size;
	int i;

	
	
	
	
	int * t = *(board->types);
	for(i = 0; i < size; i++, moves+=7){
		if (t[moves[1]] == 1)
			value += VALUE_KNIGHT_RANGE;
		if (t[moves[1]] == 2)
			value += VALUE_BISHOP_RANGE;
		if ((t[moves[1]] != 0 || moves[1] % 8 != moves[2] % 8) && moves[2] / 8 > 2 && moves[2] / 8 < 5 && moves[2] % 8 > 2 && moves[2] % 8 < 5)
			value += VALUE_CENTER_SQUARE_ATTACKED;
	}
	
	int x,y;
	for(x = 3; x < 5; x++)
		for(y = 3; y < 5; y++)
			if (board->types[x][y] != 9 && board->colors[x][y] == player)
				value += VALUE_CENTER_SQUARE_ATTACKED;
	
	free(moves_pointer);
	return value;
}

int * goodHeuristic(struct Board *board, int size, int * moves, int turn){
	int * sorted = malloc(28 * size);
	moves = weakHeuristic(board,size,moves,turn);
	int * moves_pointer = moves;
	
	int values[size];
	
	int alpha = -MATE - 1;
	int beta = MATE + 1;
	
	int i,j;
	for(i = 0; i < size; i++, moves += 7){
		values[i] = alphaBetaPrune(board,!turn,moves,HEURISTIC_DEPTH,alpha,beta,turn);	
		if (values[i] > alpha)
			alpha = values[i];
	}
	
	int v[size];
	for(i = 0; i < size; i++)
		v[i] = i;
	
	for(i = 0; i < size; i++)
		for(j = i + 1; j < size; j++)
			if (values[i] < values[j]){
				int t = values[i];
				values[i] = values[j];
				values[j] = t;
				
				t = v[i];
				v[i] = v[j];
				v[j] = t;
			}
		
	for(i = 0; i < size; i++)
		for(j = 0; j < 7; j++)
			sorted[i*7+j] = moves_pointer[v[i] * 7 + j];
			
	free(moves_pointer);
	TOTAL_BOARDS_SEARCHED = 0;
	TOTAL_MOVES_FOUND = 0;
	
	return sorted;
}

int * weakHeuristic(struct Board *board, int size, int * moves, int turn){	

	
	int * sorted = malloc(28 * size);
	int * moves_pointer = moves;
	int * sorted_pointer = sorted;
	int * types = *(board->types);
	
	int i,j;
	for(i = 0; i < size; i++){
		if (types[moves[2]] != 9)
			for(j = 0; j < 7; j++, sorted++,moves++)
				*sorted = *moves;
		else
			moves += 7;
	}
	
	moves = moves_pointer;
	
	for(i = 0; i < size; i++){
		if (types[moves[2]] == 9)
			for(j = 0; j < 7; j++, sorted++,moves++)
				*sorted = *moves;
		else
			moves += 7;
	}
	
	free(moves_pointer);
	return sorted_pointer;
		
	

	for(i = 0; i < size; i++)
		if (types[moves[i*7 + 2]] != 9)			
			for(j = 0; j < 7; j++)
				sorted[i * 7 + j] = moves[i * 7 + j];
	
	
	sorted = sorted_pointer;
	moves = moves_pointer;
	
	for(i = 0; i < size; i++)
		if (types[moves[i*7 + 2]] == 9)			
			for(j = 0; j < 7; j++)
				sorted[i * 7 + j] = moves[i * 7 + j];
	
	
	free(moves_pointer);
	return sorted_pointer;	
}