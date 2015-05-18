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
int MAX_SECONDS = 10;

time_t START_TIME;

Board * BOARD;
BinaryTable * TABLE;

/*
 * Function : findBestMoveIndex
 * ----------------------------
 * 	Return the index corresponding to Java's Engine of the move
 * 		With the greatest value
 * 	
 * 	Arguments:
 * 		game_board : game board
 * 		last_move : boolean of last_move enables enpassant
 * 		turn : turn of player
 */
int findBestMoveIndex(Board * board, int * last_move, int turn){

	START_TIME = time(NULL);
	BOARD = board;
	TABLE = createTable();
	
	int size = 0;
	int * moves = findAllValidMoves(BOARD,turn,&size,last_move);
	int * moves_p = moves;
	
	int unsorted_size = 0;
	int * unsorted = findAllValidMoves(BOARD,turn,&unsorted_size,last_move);
	
	if (size == 0)
		return -1;
	
	int values[size];
	int alpha, beta, i, move, cur_depth;
	
	for(cur_depth = MIN_DEPTH; cur_depth < MAX_DEPTH; cur_depth += 2){
		int alpha = -MATE - 1;
		int beta = MATE + 1;
		printf("SEARCHING DEPTH LEVEL %d \n", cur_depth);
		for(i = 0; i < size; i++, moves_p += 7){
			int searched = TOTAL_BOARDS_SEARCHED;
			values[i] = alphaBetaPrune(!turn,moves_p,cur_depth,alpha,beta,turn);
			printf("#%d \t Value: %d \t Searched: %d \n",i,values[i],TOTAL_BOARDS_SEARCHED-searched);
			if (values[i] > alpha)
				alpha = values[i];
			if (alpha == MATE || START_TIME + MAX_SECONDS < time(NULL))
				return endAISearch(i+1,size,values,moves,unsorted);
			
			
		}
		
		printf("\n");	
		moves = sortMoves(values,moves,size);
		moves_p = moves;
	}
	
	return endAISearch(size,size,values,moves,unsorted);
	
}


/*
 * Function : sortMoves
 * --------------------
 * 	Return a sorted version of passed in moves corresponding
 * 		to the array of value(s) given
 * 
 * 	Arguments:
 * 		values : value of each move 
 * 		moves : moves on the board
 * 		size : number of moves
 */
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


/*
 * Function : endAISerach
 * ----------------------
 * 	Terminate search and return index of best move relative
 * 		to the original unsorted array of moves which pairs
 * 		with the ArrayList of moves that Java finds
 * 
 * 	Arguments:
 * 		reached : cut off for search
 * 		size : number of moves
 * 		values : value of each move
 * 		moves : sorted by depth N-2 of moves
 * 		unsorted : original array of moves
 */
int endAISearch(int reached, int size, int * values, int * moves, int * unsorted){

	printf("Moves Found: %d \n",TOTAL_BOARDS_SEARCHED);
	printf("Table Size: %d \n",TABLE->elements);
	printf("Positions Reused: %d \n",BOARDS_REUSED);
	
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


/*
 * Function : alphaBetaPrune
 * -------------------------
 * 	Perform an alpha beta prune search from depth N to zero with the
 * 		aid of a transposition tables
 * 
 * 	Arguments: 
 * 		turn : player of the current search depth
 * 		move : last move made
 * 		depth : counter to zero for cut-off
 * 		alpha : best found
 * 		beta: worst found
 * 		evauluating_player: original player to find move for
 */
int alphaBetaPrune(int turn, int * move, int depth, int alpha, int beta, int evaluating_player){	
	
	if (START_TIME + MAX_SECONDS < time(NULL))
		return -MATE;
	
	applyGenericMove(BOARD,move);

	if (depth == 0){
		int enpass = move[0] == 4 ? move[1] : 0;
		int * key = encodeBoard(BOARD,enpass);
		Node * node = getElement(TABLE,key);
		
		if (node != NULL){
			BOARDS_REUSED += 1;
			revertGenericMove(BOARD,move);
			free(key);
			return node->value;
		}
		
		int value = evaluateBoard(evaluating_player,move);		
		revertGenericMove(BOARD,move);
		insertElement(TABLE,value,key);
		return value;
	}
	
	
	int size = 0;
	int * moves = findAllValidMoves(BOARD,turn,&size,move);	
	moves = weakHeuristic(size,moves);
		
	int * moves_pointer = moves;
	TOTAL_MOVES_FOUND += size;
	
	if (size == 0){
		int is_Check_Mate = 0;
		checkMove(BOARD,&is_Check_Mate,turn);
		revertGenericMove(BOARD,move);
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
			v = alphaBetaPrune(!turn,moves,depth-1,alpha,beta,evaluating_player);
			value = value > v ? value : v;
			alpha = alpha > value? alpha : value;
			if (beta <= alpha)
				break;
			
		}
	}
	
	else {
		value = MATE + 1;		
		for(i = 0; i < size; i++, moves += 7){
			v = alphaBetaPrune(!turn,moves,depth-1,alpha,beta,evaluating_player);
			value = value < v ? value : v;
			beta = beta < value? beta : value;
			if (beta <= alpha)
				break;
			
		}
	}
	
	revertGenericMove(BOARD,move);
	free(moves_pointer);	
	return value;
}


/*
 * Function : evaluateBoard
 * ------------------------
 * 	Take a given board and determine an integer value for the
 * 		material on the board, the placement of the pieces, and
 * 		child moves of that board
 * 	
 * 	Arguments:
 * 		player : player to evaluate relative to
 * 		lastMove: lastMove applied to get here
 */
int evaluateBoard(int player, int * lastMove){
	TOTAL_BOARDS_SEARCHED += 1;
	int value = evaluateMaterial(player) + 
				evaluatePosition(player) + 
				evaluateMoves(player,lastMove) - 
				evaluateMoves(!player,lastMove);
	return value;
}


/*
 * Function : evaluatePosition
 * ---------------------------
 * 	Assign an integer value to the set-up of the pieces
 * 		on the game board.
 * 	
 * 	Arguments:
 * 		player : player to evaluate for
 */
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


/*
 * Function : evaluateMaterial
 * ---------------------------
 *	Sum up the material on the board as an integer
 * 		relative to the player 
 * 	
 * 	Arguments:
 * 		player : player to evaluate for
 */
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


/*
 * Function : evaluateMoves
 * ------------------------
 * 	Evaluate the moves each player has in a very
 * 		shallow way based off of values defined globally
 * 
 * 	Arguments:
 * 		player: player to evaluate for
 * 		lastMove: move made to get here
 */
int evaluateMoves(int player, int * lastMove){
	int size = 0;
	int * moves = findAllValidMoves(BOARD,player,&size,lastMove);
	int * moves_pointer = moves;
	
	int value = 0;

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
	}
	
	free(moves_pointer);
	return value;
}


/*
 * Function : weakHeuristic
 * ------------------------
 * 	Sort moves so that all moves involving a capture of a piece
 * 		are ordered at the front, and the non-captures at the end
 * 	
 * 	Arguments:
 * 		size : number of moves
 * 		moves : moves to be sorted
 */
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
