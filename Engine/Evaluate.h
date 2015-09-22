#ifndef __EVALUATE_H
#define __EVALUATE_H

#include "Board.h"
#include "MoveGen.h"

#define VALUE_KNIGHT_RANGE 2
#define VALUE_BISHOP_RANGE 2

#define VALUE_CENTER_ATTACKED 8
#define VALUE_CENTRAL_KNIGHT 10

#define VALUE_KING_SURROUNDINGS_ATTACKED 5

// CASTLE EARLY AND OFTEN -- Justin
#define VALUE_CASTLED 100
#define VALUE_ABLE_TO_CASTLE 25


static int MATERIAL_VALUES[6] = {200,700,650,1000,2000,10000};
static int CAPTURE_VALUES[6] = {1,3,3,6,9,1};

static int PAWN_SCORE_MAP[2][8] = {
	{0,120,70,35,15,0,0,0},
	{0,0,0,15,35,70,120,0}
};

int evaluateBoard(Board * board, int turn);
int evaluateMaterial(Board * board, int turn);
int evaluatePosition(Board * board, int turn);
int evaluateMoves(Board * board, int turn);

#endif