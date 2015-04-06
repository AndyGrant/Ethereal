#ifndef ChessAI
#define ChessAI

#include "Engine.h"
#include "TranspositionTable.h"

int * findBestMove(struct Board * board, int * last_move, int turn);
int findBestMoveIndex(struct Board * board, int * last_move, int turn);
int alphaBetaPrune(struct TTable *, struct Board *b, int turn, int * move, int depth, int alpha, int beta, int evaluatingPlayer);
int evaluateBoard(struct Board *b, int player, int * lastMove);
int evaluateMaterial(struct Board *b, int player);
int evaluateMoves(struct Board *b, int player, int * lastMove);
int * goodHeuristic(struct TTable *, struct Board *,int,int *, int);
int * weakHeuristic(struct Board *, int, int *, int);
int evaluate(struct Board *b, int * move, int turn, int value, int depth);

#endif