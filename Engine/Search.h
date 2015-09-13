#ifndef SEARCH_HEADER
#define SEARCH_HEADER

#include "Engine.h"
#include "TTable.h"

#define MATE 99999

#define VALUE_KNIGHT_RANGE 2
#define VALUE_BISHOP_RANGE 2

#define VALUE_CENTER_ATTACKED 10
#define VALUE_CENTRAL_KNIGHT 20

#define VALUE_KING_SURROUNDINGS_ATTACKED 5

// CASTLE EARLY AND OFTEN -- Justin
#define VALUE_CASTLED 100
#define VALUE_ABLE_TO_CASTLE 25

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

int evaluateBoard(Board * board, int turn);
int evaluateMaterial(Board * board, int turn);
int evaluatePosition(Board * board, int turn);
int evaluateMoves(Board * board, int turn);

#endif
