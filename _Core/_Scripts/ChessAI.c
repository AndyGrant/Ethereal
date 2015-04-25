#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Engine.h"
#include "Moves.h"
#include "ChessAI.h"
#include "BinTable.h"

int MATE = 99999;
int MATERIAL_VALUES[6] = {100,300,300,500,1000,10000};

int VALUE_KNIGHT_RANGE = 2;
int VALUE_BISHOP_RANGE = 2;
int VALUE_ROOK_RANGE = 2;

int VALUE_CENTER_SQUARE_ATTACKED = 4;
int VALUE_CENTRAL_KNIGHT = 5;

int VALUE_KING_SURROUNDINGS_ATTACKED = 4;

int TOTAL_BOARDS_SEARCHED = 0;


int TOTAL_MOVES_FOUND = 0;
int BOARDS_REUSED = 0;

int MIN_DEPTH = 2;
int MAX_DEPTH = 20;
int MAX_SECONDS = 7;

time_t START_TIME;

int findBestMoveIndex(Board * board, int * last_move, int turn){	
	int size = 0;
	int * moves = findAllValidMoves(board,turn,&size,last_move);
	int * moves_p = moves;
	
	int unsorted_size = 0;
	int * unsorted = findAllValidMoves(board,turn,&unsorted_size,last_move);
	
	if (size == 0)
		return -1;
	
	
	START_TIME = time(NULL);
	
	int values[size];
	int alpha, beta, i, move, cur_depth;
	
	BinaryTable * table = createTable();
	for(cur_depth = MIN_DEPTH; cur_depth < MAX_DEPTH; cur_depth += 2){
		int alpha = -MATE - 1;
		int beta = MATE + 1;
		printf("SEARCHING DEPTH LEVEL %d \n", cur_depth);
		for(i = 0; i < size; i++, moves_p += 7){
			int searched = TOTAL_BOARDS_SEARCHED;
			values[i] = alphaBetaPrune(table,board,!turn,moves_p,cur_depth,alpha,beta,turn);
			printf("#%d \t Value: %d \t Searched: %d \n",i,values[i],TOTAL_BOARDS_SEARCHED-searched);
			if (values[i] > alpha)
				alpha = values[i];
			if (alpha == MATE || START_TIME + MAX_SECONDS < time(NULL))
				return endAISearch(i+1,size,values,moves,unsorted,table);
			
			
		}
		
		printf("\n");	
		moves = sortMoves(values,moves,size);
		moves_p = moves;
	}
	
	return endAISearch(size,size,values,moves,unsorted,table);
	
}

int * sortMoves(int * values, int * moves, int size){	
	int i, j;
	
	int * sorted = malloc(28 * size);
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
			sorted[i*7+j] = moves[v[i] * 7 + j];
	
	free(moves);
	return sorted;
}

int endAISearch(int reached, int size, int * values, int * moves, int * unsorted, BinaryTable * table){

	printf("Moves Found: %d \n",TOTAL_BOARDS_SEARCHED);
	printf("Table Size: %d \n",table->elements);
	printf("Positions Reused: %d \n",BOARDS_REUSED);
	
	destroyTable(table);
	
	int i;
	int best_index = 0;
	for(i = 1; i < reached; i++)
		if (values[i] > values[best_index])
			best_index = i;
	
	int * best = malloc(28);
	for(i = 0;  i < 7; i++)
		best[i] = moves[(best_index*7) + i];
	free(moves);
	
	int j;
	int index[5] = {0,1,2,3,6};
	for(i = 0; i < size; i++){
		for(j = 0; j < 5; j++){
			if (unsorted[i*7+index[j]] != best[index[j]])
				j = 7;
			else if (j == 4){
				free(unsorted);
				return i;
			}
		}
	}
	
	
	return -1;
}

int alphaBetaPrune(BinaryTable * table, Board * board, int turn, int * move, int depth, int alpha, int beta, int evaluating_player){	
	
	if (START_TIME + MAX_SECONDS < time(NULL))
		return -MATE;
	
	applyGenericMove(board,move);

	if (depth == 0){
		int enpass = move[0] == 4 ? move[1] : 0;
		int * key = encodeBoard(board,enpass);
		Node * node = getElement(table,key);
		
		if (node != NULL){
			BOARDS_REUSED += 1;
			revertGenericMove(board,move);
			free(key);
			return node->value;
		}
		
		int value = evaluateBoard(board,evaluating_player,move);		
		revertGenericMove(board,move);
		insertElement(table,value,key);
		return value;
	}
	
	
	int size = 0;
	int * moves = findAllValidMoves(board,turn,&size,move);	
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
			v = alphaBetaPrune(table, board,!turn,moves,depth-1,alpha,beta,evaluating_player);
			value = value > v ? value : v;
			alpha = alpha > value? alpha : value;
			if (beta <= alpha)
				break;
			
		}
	}
	
	else {
		value = MATE + 1;		
		for(i = 0; i < size; i++, moves += 7){
			v = alphaBetaPrune(table, board,!turn,moves,depth-1,alpha,beta,evaluating_player);
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

int evaluateBoard(Board *board, int player, int * lastMove){
	TOTAL_BOARDS_SEARCHED += 1;
	int value = 	evaluateMaterial(board,player) + 
				evaluateMoves(board,player,lastMove) - 
				evaluateMoves(board,!player,lastMove);
	return value;
}

int evaluateMaterial(Board *board, int player){
	int value = 0;
	int x,y;
	for(x = 0; x < 8; x++){
		for(y = 0; y < 8; y++){
			if (board->types[x][y] != EMPTY){
				if (board->colors[x][y] == player)
					value += MATERIAL_VALUES[board->types[x][y]];
				else
					value -= MATERIAL_VALUES[board->types[x][y]];
			}
		}
	}
	
	return value;
}

int evaluateMoves(Board *board, int player, int * lastMove){
	int size = 0;
	int * moves = findAllValidMoves(board,player,&size,lastMove);
	int * moves_pointer = moves;
	
	int value = 0;

	int i,x,y;

	int * t = *(board->types);
	for(i = 0; i < size; i++, moves+=7){
		if (t[moves[1]] == KNIGHT)
			value += VALUE_KNIGHT_RANGE;
		else if (t[moves[1]] == BISHOP)
			value += VALUE_BISHOP_RANGE;
		else if (t[moves[1]] == ROOK)
			value += VALUE_ROOK_RANGE;
		if (moves[2] / 8 > 2 && moves[2] / 8 < 5 && moves[2] % 8 > 2 && moves[2] % 8 < 5)
			value += VALUE_CENTER_SQUARE_ATTACKED;		
	}
	
	
	int pawn_end = player * 7;
	int pawn_start = 6 - (player * 5);
	for(x = 1; pawn_start < pawn_end; pawn_start++, x++)
		for(y = 0; y < 8; y++)
			if (board->types[pawn_start][y] == PAWN && board->colors[pawn_start][y] == player)
				value += x;
	
	
	for(x = 2; x < 6; x++)
		for(y = 2; y < 6; y++)
			if (board->types[x][y] == KNIGHT && board->colors[x][y] == player)
				value += VALUE_CENTRAL_KNIGHT;

	
	for(x = 3; x < 5; x++)
		for(y = 3; y < 5; y++)
			if (board->types[x][y] != EMPTY && board->colors[x][y] == player)
				value += VALUE_CENTER_SQUARE_ATTACKED;
	
	free(moves_pointer);
	return value;
}

int * weakHeuristic(Board * board, int size, int * moves, int turn){	
	int * sorted = malloc(7 * sizeof(int) * size);
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
}
