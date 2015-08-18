#ifndef Engine
#define Engine

#include "Types.h"
#include "Board.h"
#include "BitTools.h"

Move ** getAllMoves(Board * board, int * index, int turn);
Move ** validateCheck(Board * board, int * index, Move ** moves, int turn);
int validateMove(Board * board, int turn);
void getKnightMoves(Board * board, Move ** moves, int * index, int turn);
void getKingMoves(Board * board, Move ** moves, int * index, int turn);
void getPawnMoves(Board * board, Move ** moves, int * index, int turn);
void getRookMoves(Board * board, Move ** moves, int * index, int turn);
void getBishopMoves(Board * board, Move ** moves, int * index, int turn);
void getQueenMoves(Board * board, Move ** moves, int * index, int turn);


#endif