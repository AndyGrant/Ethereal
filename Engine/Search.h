#ifndef __SEARCH__H
#define __SEARCH__H

#include "Board.h"

#define MATE 99999

extern int TOTAL_BOARDS_SEARCHED;
extern int TOTAL_MOVES_FOUND;


typedef struct SearchThreadData{
	Board * board;
	int turn;
	int * move;
	int depth;
	int alpha;
	int beta;
	int eval;
} SearchThreadData;

int getBestMoveIndex(Board * board, int turn);
int endSearch(int index, int size, int * values, int * sorted, int * unsorted);
int alphaBetaPrune(Board * board, int turn, int * moves , int depth, int alpha, int beta, int eval);

void valueSort(int * values, int * moves, int size);
void hueristicSort(Board * board, int * moves, int size, int turn);

#endif
