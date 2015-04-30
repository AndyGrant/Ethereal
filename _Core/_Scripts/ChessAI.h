#ifndef ChessAI
#define ChessAI

#include "Engine.h"
#include "BinTable.h"

int findBestMoveIndex(Board * board, int * last_move, int turn);
int alphaBetaPrune(int turn, int * move, int depth, int alpha, int beta, int evaluatingPlayer);

int evaluateBoard(int player, int * lastMove);
int evaluatePosition(int player);
int evaluateMaterial(int player);
int evaluateMoves(int player, int * lastMove);

int * weakHeuristic(int size, int * moves);

int endAISearch(int reached, int size, int * values, int * moves, int * unsorted);
int * sortMoves(int * values, int * moves, int size);

#endif
