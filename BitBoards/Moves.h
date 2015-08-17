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

extern void (*ApplyTypes[7])(Board *, Move *, int);
extern void (*RevertTypes[7])(Board *, Move *, int);

#define ApplyMove(b,m,t) ApplyTypes[m->Type](b,m,t);
#define RevertMove(b,m,t) RevertTypes[m->Type](b,m,t);

void ApplyNormalMove(Board * board, Move * move, int turn);
void RevertNormalMove(Board * board, Move * move, int turn);

void ApplyPawnDoublePushMove(Board * board, Move * move, int turn);
void RevertPawnDoublePushMove(Board * board, Move * move, int turn);

void ApplyBreaksLeftCastleMove(Board * board, Move * move, int turn);
void RevertBreaksLeftCastleMove(Board * board, Move * move, int turn);

void ApplyBreaksRightCastleMove(Board * board, Move * move, int turn);
void RevertBreaksRightCastleMove(Board * board, Move * move, int turn);

void ApplyBreaksBothCastlesMove(Board * board, Move * move, int turn);
void RevertBreaksBothCastlesMove(Board * board, Move * move, int turn);

void ApplyLeftCastleMove(Board * board, Move * move, int turn);
void RevertLeftCastleMove(Board * board, Move * move, int turn);

void ApplyRightCastleMove(Board * board, Move * move, int turn);
void RevertRightCastleMove(Board * board, Move * move, int turn);

#endif