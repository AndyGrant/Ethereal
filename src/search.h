#ifndef _SEARCH_H
#define _SEARCH_H

#include <stdint.h>

#include "types.h"

#define USE_RAZOR_PRUNING                   (1)
#define USE_FUTILITY_PRUNING                (1)
#define USE_NULL_MOVE_PRUNING               (1)
#define USE_LATE_MOVE_REDUCTIONS            (1)
#define USE_INTERNAL_ITERATIVE_DEEPENING    (1)
#define USE_TRANSPOSITION_TABLE             (1)

#define MaxKillers  (3)

/* Prototypes */

uint16_t getBestMove(Board * board, int seconds, int logging);

int rootSearch(Board * board, MoveList * moveList, int depth);
int alphaBetaSearch(Board * board, int alpha, int beta, int depth, int height, int nodeType);
int quiescenceSearch(Board * board, int alpha, int beta, int height);

void evaluateMoves(Board * board, int * values, uint16_t * moves, int size, int height, uint16_t tableMove);
uint16_t getNextMove(uint16_t * moves, int * values, int index, int size);

void sortMoveList(MoveList * moveList);

int canDoNull(Board * board);

#endif