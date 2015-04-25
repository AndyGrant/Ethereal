#ifndef ChessAI
#define ChessAI

#include "Engine.h"
#include "BinTable.h"

int findBestMoveIndex(Board * board, int * last_move, int turn);
int alphaBetaPrune(BinaryTable * table , Board *b, int turn, int * move, int depth, int alpha, int beta, int evaluatingPlayer);

int evaluateBoard(Board *b, int player, int * lastMove);
int evaluateMaterial(Board *b, int player);
int evaluateMoves(Board *b, int player, int * lastMove);

int * weakHeuristic(Board *board, int size, int * moves, int turn);

int endAISearch(int reached, int size, int * values, int * moves, int * unsorted, BinaryTable * table);
int * sortMoves(int * values, int * moves, int size);

#endif
