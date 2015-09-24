#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

#include "Search.h"
#include "Board.h"
#include "MoveGen.h"
#include "TTable.h"
#include "Evaluate.h"

TTable * TABLE;

time_t START_TIME;
time_t END_TIME;
int MAX_TIME = 3;

int START_DEPTH = 2;
int MAX_DEPTH = 16;
int DELTA_DEPTH = 2;
int END_DEPTH = 16;

int SEARCH_THREAD_DEPTH = 4;
int USE_TTABLE = 1;

int TOTAL_BOARDS_SEARCHED = 0;
int TOTAL_MOVES_FOUND = 0;

int getBestMoveIndex(Board * board, int turn){

	START_TIME = time(NULL);
	END_TIME = START_TIME + MAX_TIME;
	
	TABLE = createTTable();
	
	int size = 0;
	int * moves = getAllMoves(board,turn,&size);
	int * moves_p = moves;
	
	size = 0;
	int * unsorted = getAllMoves(board,turn,&size);
	
	if (size == 0)
		return -1;
		
	int values[size];
	
	int depth, i;
	
	for(depth = START_DEPTH; depth < MAX_DEPTH && depth != END_DEPTH; depth += DELTA_DEPTH){
		printf("==========================================\n");
		printf("Searching Depth Level : %d\n",depth);
		
		
		SEARCH_THREAD_DEPTH = depth > 5 ? 5 : 5;
		
		
		int alpha = -MATE;
		int beta = MATE;
		
		for(i = 0; i < size; i++, moves += 5){
			int preSearched = TOTAL_BOARDS_SEARCHED;
			values[i] = -alphaBetaPrune(board,!turn,moves,depth,-beta,-alpha,turn);
			printf("#%d\tValue: %d\tSearched: %d\n",i,values[i],TOTAL_BOARDS_SEARCHED-preSearched);
			
			if (values[i] > alpha)
				alpha = values[i];
				
			if (alpha == MATE)
				return endSearch(i+1,size,values,moves_p,unsorted);
			
			if (END_TIME < time(NULL))
				return endSearch(i+1,size,values,moves_p,unsorted);				
		}
		
		if (alpha == -MATE)
			return endSearch(size,size,values,moves_p,unsorted);
		
		valueSort(values,moves_p,size);
		moves = moves_p;
	}
	
	return endSearch(i+1,size,values,moves_p,unsorted);
}

int endSearch(int index, int size, int * values, int * sorted, int * unsorted){
	printf("==========================================\n");
	printf("Total Boards Searched \t: %d\n",TOTAL_BOARDS_SEARCHED);
	printf("Total Moves Found \t: %d\n",TOTAL_MOVES_FOUND);
	printf("Total Transpositions \t: %d\n",TABLE->size);
	printf("Total Nodes In TTTable \t: %d\n",TABLE->totalNodes);
	printf("Total Empty Buckets \t: %d\n",getNonEmptyBucketCount(TABLE));
	printf("Total Time Taken \t: %d\n",(int)(time(NULL)-START_TIME));
	
	int i,j;
	int best_index = 0;
	for(i = 0; i < index; i++)
		if (values[i] > values[best_index])
			best_index = i;
			
	int * best = sorted + (best_index * 5);
	
	for(i = 0; i < size; i++)
		if( memcmp(best,unsorted+(i*5),sizeof(int)*5) == 0)
			return i;
	return 0;
}

void valueSort(int * values, int * moves, int size){
	int i,j;
	int a[5], b[5];
	for(i = 0; i < size; i++){
		for(j = i + 1; j < size; j++){
			if (values[j] > values[i]){
				memcpy(a,moves+i*5,sizeof(int) * 5);
				memcpy(b,moves+j*5,sizeof(int) * 5);
				memcpy(moves+j*5,a,sizeof(int) * 5);
				memcpy(moves+i*5,b,sizeof(int) * 5);
				int x = values[j];
				values[i] = values[j];
				values[j] = x;
			}
		}
	}
}

void hueristicSort(Board * board, int * moves, int size, int turn){
	int values[size];	
	int * moves_p = moves;
	int i;
	for(i = 0; i < size; i++,moves+=5){
		ApplyMove(board,moves);
		values[i] = evaluatePosition(board,turn);
		RevertMove(board,moves);
	}
	
	valueSort(values,moves_p,size);
}

int alphaBetaPrune(Board * board, int turn, int * move, int depth, int alpha, int beta, int eval){
	
	if (END_TIME < time(NULL))
		return eval == turn ? -MATE : MATE;
	
	ApplyMove(board,move);
	int * lastmove = board->LastMove;
	board->LastMove = move;
	
	int * key;
	int hash;
	Node * node;
  
	if (USE_TTABLE){
		key = createKey(board);
		hash = createHash(key);
		node = getNode(TABLE,hash,key);
		
		if (node != NULL && node->depth >= depth){
			int value = node->turn == turn ? node->value : -node->value;
			
			if (node->type == EXACT){
				free(key);
				RevertMove(board,move);
				return value;
			}
			
			if (node->type == LOWERBOUND && node->value > alpha)
				alpha = value;
			else if (node->type == UPPERBOUND && node->value < beta)
				beta = value;
				
			if (alpha >= beta){
				free(key);
				RevertMove(board,move);
				return value;
			}
		}
	}
	
	if (depth == 0){		
		int value = evaluateBoard(board, turn);
		RevertMove(board,move);
		if (USE_TTABLE){
			if (node == NULL)
				storeNode(TABLE,hash,createNode(key,value,depth,getNodeType(alpha,beta,value),turn));
			else
				free(key);
		}
		board->LastMove = lastmove;
		return value;
	}
	
	int best = 0, size = 0;
	int * moves = getAllMoves(board,turn,&size);
	hueristicSort(board,moves,size,turn);
	int * moves_p = moves;
	
	TOTAL_MOVES_FOUND += size;
	
	if (size == 0)
		best = validateMove(board,turn) == 1 ? 0 : -MATE;
  else if (depth == SEARCH_THREAD_DEPTH)
    best = threadedAlphaBetaPrune(board,turn,moves,size,depth,alpha,beta,eval);
  else {
		best = -MATE - 1;
		int i;
		for(i = 0; i < size; i++, moves += 5){
			int value = -alphaBetaPrune(board,!turn,moves,depth-1,-beta,-alpha,eval);
			if (value > best)
				best = value;	
			if (best > alpha)
				alpha = best;	
			if (alpha >= beta)
				break;
			
		}
	}
	
	if (USE_TTABLE){
		if (node == NULL)
			storeNode(TABLE,hash,createNode(key,best,depth,getNodeType(alpha,beta,best),turn));
		else	
			free(key);
		if (node != NULL && node->depth < depth){
			node->depth = depth;
			node->turn = turn;
			node->value = best;
			node->type = getNodeType(alpha,beta,best);
		}
	}
	
	RevertMove(board,move);
	board->LastMove = lastmove;
	free(moves_p);
	return best;
}

int threadedAlphaBetaPrune(Board * board, int turn, int * moves, int size, int depth, int alpha, int beta, int eval){  

  int best = -MATE-1;
  
  pthread_t threads[4];
  SearchThreadData data[4];
  Board * boards[4];
  
  int i,j;		
  
  for(i = 0 ; i < 4; i++)
    boards[i] = copyBoard(board);
  
  for(i = 0; i < size; i += 4){
    for(j = 0; j < 4 && j + i < size; j++){
      data[j].board = boards[j];
      data[j].board->LastMove = board->LastMove;
      data[j].turn = !turn;
      data[j].depth = depth;				
      data[j].move = moves + (i+j)*5;
      data[j].alpha = alpha;
      data[j].beta = beta;
      data[j].eval = eval;
    }			
    
    for(j = 0; j < 4 && j + i < size; j++)
      pthread_create(&(threads[j]),NULL,createAlphaBetaPruneThread,&(data[j]));
      
    for(j = 0; j < 4 && j + i < size; j++)
      pthread_join(threads[j],NULL);
    
  
    for(j = 0; j < 4 && j + i < size; j++){
      if (data[j].alpha > best)
        best = data[j].alpha;	
      if (best > alpha)
        alpha = best;
      if (best >= beta){
        i = size;
      }
    }
  }
  
  for(i = 0 ; i < 4; i++)
    free(boards[i]);
    
  return best;	
}

void * createAlphaBetaPruneThread(void * ptr){
	SearchThreadData * data = (SearchThreadData *)(ptr);
	int x = -alphaBetaPrune(data->board,data->turn,data->move,data->depth-1,-data->beta,-data->alpha,data->eval);
	data->alpha = x;
	return NULL;
}