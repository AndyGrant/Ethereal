#ifndef Moves
#define Moves

#include "board.h"
#include "types.h"
#include "engine.h"

extern int NormalMove;
extern int PawnDoublePushMove;
extern int BreaksLeftCastleMove;
extern int BreaksRightCastleMove;
extern int BreaksBothCastlesMove;
extern int LeftCastleMove;
extern int RightCastleMove;
extern int PromoteBishopMove;
extern int PromoteKnightMove;
extern int PromoteRookMove;
extern int PromoteQueenMove;
extern int EnpassLeftMove;
extern int EnpassRightMove;

void ApplyNormalMove(Board * board, Move * move, int turn);
void RevertNormalMove(Board * board, Move * move, int turn);

void ApplyPawnDoublePushMove(Board * board, Move * move, int turn);
void RevertPawnDoublePushMove(Board * board, Move * move, int turn);

#endif