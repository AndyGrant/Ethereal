#ifndef SEARCH_HEADER
#define SEARCH_HEADER

#include "Engine.h"
#include "TTable.h"

#define MATE 99999

int getBestMoveIndex(Board * board, int turn);
int endSearch(int index, int size, int * values, int * sorted, int * unsorted);
int alphaBetaPrune(Board * board, int turn, int * moves , int depth, int alpha, int beta, int eval);

void valueSort(int * values, int * moves, int size);
void hueristicSort(int * moves, int size);

int evaluateBoard(Board * board, int turn);
int evaluateMaterial(Board * board, int turn);
int evalutePosition(Board * board, int turn);
int evaluateMoves(Board * board, int turn);

#endif