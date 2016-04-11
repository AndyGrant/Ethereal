#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

#define PawnValue   ( 100)
#define KnightValue ( 300)
#define BishopValue ( 300)
#define RookValue   ( 500)
#define QueenValue  ( 900)
#define KingValue   (1000)

#define PAWN_STACKED_PENALTY  ( 30)
#define PAWN_ISOLATED_PENALTY ( 20)
#define ROOK_7TH_RANK_VALUE   ( 35)
#define ROOK_8TH_RANK_VALUE   ( 45)

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int evaluate_board(Board * board);

#endif