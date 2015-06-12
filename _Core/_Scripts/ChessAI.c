#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Engine.h"
#include "Moves.h"
#include "ChessAI.h"
#include "BinTable.h"

int MATE = 99999;
int MATERIAL_VALUES[6] = {100,300,300,500,1000,10000};
int CAPTURE_VALUES[6] = {1,3,3,6,9,1};

int VALUE_KNIGHT_RANGE = 2;
int VALUE_BISHOP_RANGE = 2;
int VALUE_ROOK_RANGE = 0;

int VALUE_CENTER_SQUARE_ATTACKED = 4;
int VALUE_CENTRAL_KNIGHT = 5;
int VALUE_KING_SURROUNDINGS_ATTACKED = 10;

int TOTAL_BOARDS_SEARCHED = 0;
int TOTAL_MOVES_FOUND = 0;
int BOARDS_REUSED = 0;

int MIN_DEPTH = 2;
int MAX_DEPTH = 20;
int MAX_SECONDS = 10;

time_t START_TIME;

Board * BOARD;
BinaryTable * TABLE;

int findBestMoveIndex(Board * board, int * last_move, int turn){
	
	// Initalize globals to reduce argument passing
	START_TIME = time(NULL);
	BOARD = board;
	TABLE = createTable();
	
	// Create array of all valid moves that may be manipulated
	int size = 0;
	int * moves = findAllValidMoves(BOARD,turn,&size,last_move);
	int * moves_p = moves;
	
	// Create array of all valid moves to reference after sorting
	int unsorted_size = 0;
	int * unsorted = findAllValidMoves(BOARD,turn,&unsorted_size,last_move);
	
	// AI is in checkmate return -1 to indicate this
	if (size == 0)
		return -1;
	
	// Create array for moves values and all needed loop variables
	// and updated alpha beta pruning value holders
	int values[size];
	int alpha, beta, i, move, cur_depth;
	
	
	// Use iterative deepening with sorted moves until time expires
	// or until a max depth has been reached
	for(cur_depth = MIN_DEPTH; cur_depth < MAX_DEPTH; cur_depth += 2){
		printf("SEARCHING DEPTH LEVEL %d \n", cur_depth);
		
		// Initalize alpha and beta to largest window range
		int alpha = -MATE - 1;
		int beta = MATE + 1;
		
		// Evaluate all moves and fill the values array
		for(i = 0; i < size; i++, moves_p += 7){
			int searched = TOTAL_BOARDS_SEARCHED;
			values[i] = -alphaBetaPrune(!turn,moves_p,cur_depth,-beta,-alpha,turn);
			printf("#%d \t Value: %d \t Searched: %d \n",i,values[i],TOTAL_BOARDS_SEARCHED-searched);
			
			// Update alpha value
			if (values[i] > alpha)
				alpha = values[i];
			
			// If checkmate has been found or time has expired, terminate
			if (alpha == MATE || START_TIME + MAX_SECONDS < time(NULL))
				return endAISearch(i+1,size,values,moves,unsorted);
			
			
		}
		
		printf("\n");	
		moves = sortMoves(values,moves,size);
		moves_p = moves;
	}
	
	return endAISearch(size,size,values,moves,unsorted);
	
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

int endAISearch(int reached, int size, int * values, int * moves, int * unsorted){

	printf("Moves Found: %d \n",TOTAL_MOVES_FOUND);
	printf("Boards Evaluated: %d \n",TOTAL_BOARDS_SEARCHED);
	printf("Table Size: %d \n",TABLE->elements);
	
	destroyTable(TABLE);
	
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
				free(best);
				free(unsorted);
				return i;
			}
		}
	}
	
	free(best);
	free(unsorted);
	return -1;
}
int alphaBetaPrune(int turn, int * move, int depth, int alpha, int beta, int evaluating_player){
	
	// Return worst value if time has expired
	if (START_TIME + MAX_SECONDS < time(NULL))
		return -MATE;
		
	// Create the new board
	applyGenericMove(BOARD,move);
	
	// Needed Storage
	int value, best, i, size = 0;
	int * moves, * moves_pointer;
	
	// Search Transposition Table
	int enpass = move[0] == 4 ? move[1] : 0;
	int * key = encodeBoard(BOARD,enpass,turn);
	Node * node = getElement(TABLE,key);
	
	// Use the Node to speed up algorithm
	if (node != NULL && node->depth >= depth){
	
		int node_rel_value = node->key[9] == turn ? node->value : -node->value;

		// If node is exact return it's value
		if (node->type == EXACT){
			revertGenericMove(BOARD,move);
			return node_rel_value;
		}
		
		// Adjust bounds based on node type and value
		if (node->type == LOWERBOUND && node->value > alpha)		
			alpha = node_rel_value;
		else if (node->type == UPPERBOUND && node->value < beta)
			beta = node_rel_value;
		
		// Check new bounds for pruning
		if (alpha >= beta){
			revertGenericMove(BOARD,move);
			// Free the key if it is not needed
			free(key);
			return node_rel_value;
		}		
	}
	
	// Max search depth has been reached
	if (depth == 0){
		
		// Determine board value
		value = evaluateBoard(turn,move);
		
		// Reset board state
		revertGenericMove(BOARD,move);
		
		// Determine type of transposition
		if (node == NULL){
			if (value <= alpha)
				insertElement(TABLE,value,key,depth,LOWERBOUND);
			else if (value >= beta)
				insertElement(TABLE,value,key,depth,UPPERBOUND);
			else
				insertElement(TABLE,value,key,depth,EXACT);
		}
		else
			free(key);
		return value;
	}
	
	// Find moves if not already found
	moves = findAllValidMoves(BOARD,turn,&size,move);
	moves = weakHeuristic(size,moves);
	
	// Create pointer to start of moves
	moves_pointer = moves;
	
	TOTAL_MOVES_FOUND += size;
	
	if (size == 0){
		int is_not_in_check = 0;
		checkMove(BOARD,&is_not_in_check,turn);
		
		if(is_not_in_check)
			best = 0;
		else
			best = -MATE;
	}
	
	// For the AI's turn
	else{
		best = -MATE - 1;
		for(i = 0; i < size; i++, moves += 7){
			value = -alphaBetaPrune(!turn,moves,depth-1,-beta,-alpha,evaluating_player);
			if(value > best)
				best = value;
			if(best > alpha)
				alpha = best;
			if(best >= beta)
				break;

		}
	}
	
	
	// Reset board state
	revertGenericMove(BOARD,move);
	
	// Free malloc'ed memory
	free(moves_pointer);
	
	// Determine type of transposition
	if (node == NULL){
		if (best <= alpha)
			insertElement(TABLE,best,key,depth,LOWERBOUND);
		else if (best >= beta)
			insertElement(TABLE,best,key,depth,UPPERBOUND);
		else
			insertElement(TABLE,best,key,depth,EXACT);
	}
	else if (depth > node->depth){
		free(key);
		node->value = node->key[9] == turn ? best : -best;
		if (best <= alpha)
			node->type = LOWERBOUND;
		else if (best >= beta)
			node->type = UPPERBOUND;
		else
			node->type = EXACT;
	}
	return best;
}

int evaluateBoard(int player, int * lastMove){
	TOTAL_BOARDS_SEARCHED += 1;
	int value = evaluateMaterial(player) + 
				evaluatePosition(player) + 
				evaluateMoves(player,lastMove) - 
				evaluateMoves(!player,lastMove);
	return value;
}

int evaluatePosition(int player){

	int x, y, value = 0;
	int pawn_start, pawn_end;
	
	pawn_end = player * 7;
	pawn_start = 6 - (player * 5);
	for(x = 1; pawn_start < pawn_end; pawn_start++, x++)
		for(y = 0; y < 8; y++)
			if (BOARD->types[pawn_start][y] == PAWN && BOARD->colors[pawn_start][y] == player)
				value += x;
			
	pawn_end = !player * 7;
	pawn_start = 6 - (!player * 5);
	for(x = 1; pawn_start < pawn_end; pawn_start++, x++)
		for(y = 0; y < 8; y++)
			if (BOARD->types[pawn_start][y] == PAWN && BOARD->colors[pawn_start][y] == !player)
				value -= x;
	
	
	for(x = 2; x < 6; x++){
		for(y = 2; y < 6; y++){
			if (BOARD->types[x][y] == KNIGHT){
				if (BOARD->colors[x][y] == player)
					value += VALUE_CENTRAL_KNIGHT;
				else
					value -= VALUE_CENTRAL_KNIGHT;
			}
		}
	}
			
	for(x = 3; x < 5; x++){
		for(y = 3; y < 5; y++){
			if (BOARD->types[x][y] != EMPTY){
				if(BOARD->colors[x][y] == player)
					value += VALUE_CENTER_SQUARE_ATTACKED;
				else
					value -= VALUE_CENTER_SQUARE_ATTACKED;
			}
		}
	}
	
	return value;
}

int evaluateMaterial(int player){
	int value = 0;
	int x,y;
	for(x = 0; x < 8; x++){
		for(y = 0; y < 8; y++){
			if (BOARD->types[x][y] != EMPTY){
				if (BOARD->colors[x][y] == player)
					value += MATERIAL_VALUES[BOARD->types[x][y]];
				else
					value -= MATERIAL_VALUES[BOARD->types[x][y]];
			}
		}
	}
	
	return value;
}

int evaluateMoves(int player, int * lastMove){
	int size = 0;
	int * moves = findAllValidMoves(BOARD,player,&size,lastMove);
	int * moves_pointer = moves;
	
	TOTAL_MOVES_FOUND += size;
		
	int value = 0;
	float nv = 0;
	
	int kcords = BOARD->kingLocations[!player + 1];
	int kx = kcords / 8;
	int ky = kcords % 8;

	int i;

	int * t = *(BOARD->types);
	for(i = 0; i < size; i++, moves+=7){
		if (t[moves[1]] == KNIGHT)
			value += VALUE_KNIGHT_RANGE;
		else if (t[moves[1]] == BISHOP)
			value += VALUE_BISHOP_RANGE;
		else if (t[moves[1]] == ROOK)
			value += VALUE_ROOK_RANGE;
		if (moves[2] / 8 > 2 && moves[2] / 8 < 5 && moves[2] % 8 > 2 && moves[2] % 8 < 5)
			value += VALUE_CENTER_SQUARE_ATTACKED;
		if (moves[0] == 0 && moves[3] != 9)
			nv += CAPTURE_VALUES[moves[3]] / CAPTURE_VALUES[t[moves[1]]];
		//if (abs((moves[2] / 8) - kx) <= 1 && abs((moves[2] % 8) - ky) <= 1)
			//value += VALUE_KING_SURROUNDINGS_ATTACKED;
			
		
	}
	
	free(moves_pointer);
	return value + (int)(nv / 2);
}

int * weakHeuristic(int size, int * moves){	
	int * sorted = malloc(7 * sizeof(int) * size);
	int * moves_pointer = moves;
	int * sorted_pointer = sorted;
	int * types = *(BOARD->types);
	
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
