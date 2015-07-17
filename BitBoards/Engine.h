#ifndef Engine
#define Engine

#include "Types.h"
#include "Board.h"
#include "BitTools.h"

void getKnightMoves(Board * board, Move ** moves, int * index, int turn);
void getKingMoves(Board * board, Move ** moves, int * index, int turn);
void getPawnMoves(Board * board, Move ** moves, int * index, int turn);

#endif