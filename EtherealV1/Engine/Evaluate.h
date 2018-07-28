#ifndef __EVALUATE_H
#define __EVALUATE_H

#include "Board.h"
#include "MoveGen.h"

extern int VALUE_KNIGHT_RANGE;
extern int VALUE_BISHOP_RANGE;

extern int VALUE_CENTER_ATTACKED;
extern int VALUE_CENTRAL_KNIGHT;

extern int VALUE_KING_SURROUNDINGS_ATTACKED;

// CASTLE EARLY AND OFTEN -- Justin
extern int VALUE_CASTLED;
extern int VALUE_ABLE_TO_CASTLE;


static int MATERIAL_VALUES[6] = {200,600,600,1000,1800,2000};
static int CAPTURE_VALUES[6] = {1,3,3,6,9,1};

static int PAWN_SCORE_MAP[2][8] = {
	{0,128,64,16,4,0,0,0},
	{0,0,0,4,16,64,128,0}
};

int evaluateBoard(Board * board, int turn);
int evaluateMaterial(Board * board, int turn);
int evaluatePosition(Board * board, int turn);
int evaluateMoves(Board * board, int turn);

#endif
