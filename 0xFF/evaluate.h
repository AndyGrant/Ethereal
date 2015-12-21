#ifndef EVALUATE_H
#define EVALUATE_H

#include "util.h"

static int KnightValueMap[64] = {
	 -5, -5, -4, -5, -5, -4, -5, -5,
	 -2,  0,  0,  1,  1,  0,  0, -2,
	 -2,  3,  2,  5,  5,  2,  3, -2,
	 -1,  1,  7,  4,  4,  7,  1, -1,
	 -1,  1,  7,  4,  4,  7,  1, -1,
	 -2,  3,  2,  5,  5,  2,  3, -2,
	 -2,  0,  0,  1,  1,  0,  0, -2,
	 -5, -5, -4, -5, -5, -4, -5, -5
};

static int BishopValueMap[64] = {
	 -5, -1, -1, -1, -1, -1, -1, -5,	
      0,  4,  1, -1, -1,  1,  4,  0,
	  1,  3,  2,  3,  3,  2,  3,  1,
	  1,  4,  5,  1,  1,  5,  4,  1,
	  1,  4,  5,  1,  1,  5,  4,  1,
	  1,  3,  2,  3,  3,  2,  3,  1,
	  0,  4,  1, -1, -1,  1,  4,  0,
	 -5, -1, -1, -1, -1, -1, -1, -5	
};

static int RookValueMap[64] = {
	 -1, -1,  1,  3,  3, -1,  1, -1,
	 -1,  0,  0,  2,  2,  0,  0, -1, 
	  0,  2,  3,  1,  1,  3,  2,  0, 
	  1,  1,  2, -2, -2,  2,  1,  1, 
	  1,  1,  2, -2, -2,  2,  1,  1, 
	  0,  2,  3,  1,  1,  3,  2,  0, 
	 -1,  0,  0,  2,  2,  0,  0, -1, 
	 -1, -1,  1,  3,  3, -1,  1, -1
};

static int QueenValueMap[64] = {
	 -2, -1, -1, -1, -1, -1, -1, -2,
	  1,  2,  3,  2,  2,  3,  2,  1,
	  4,  4,  5,  3,  3,  5,  4,  4,
	  0,  6,  5,  7,  7,  5,  6,  0,
	  0,  6,  5,  7,  7,  5,  6,  0,
	  4,  4,  5,  3,  3,  5,  4,  4,
	  1,  2,  3,  2,  2,  3,  2,  1,
	 -2, -1, -1, -1, -1, -1, -1, -2,
};

#define QUEEN_POSITION_VALUE(p)		(QueenValueMap[CONVERT_256_TO_64((p))])
#define ROOK_POSITION_VALUE(p)		(RookValueMap[CONVERT_256_TO_64((p))])
#define BISHOP_POSITION_VALUE(p)	(BishopValueMap[CONVERT_256_TO_64((p))])
#define KNIGHT_POSITION_VALUE(p)	(KnightValueMap[CONVERT_256_TO_64((p))])

#endif