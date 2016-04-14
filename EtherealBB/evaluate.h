#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

#define PawnValue   ( 100)
#define KnightValue ( 300)
#define BishopValue ( 300)
#define RookValue   ( 500)
#define QueenValue  ( 925)
#define KingValue   (  50)

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

static int PAWN_PASSED_VALUES[8] = {0, 0, 0, 2, 7, 15, 23, 0};

int evaluate_board(Board * board);
int evaluate_board_midgame(Board * board);
int evaluate_board_endgame(Board * board);

#endif
