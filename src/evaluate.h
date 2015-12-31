#ifndef EVALUATE_H
#define EVALUATE_H

#include "util.h"

/* Search Definitions */
#define CheckMate	(100000)

/* Piece Value Definitions */
#define PawnValue 	(300)
#define KnightValue	(3 * PawnValue)
#define BishopValue	(3 * PawnValue)
#define RookValue 	(5 * PawnValue)
#define QueenValue 	(9 * PawnValue)

#define PawnStackedValue				(-99)
#define IsolatedPawnValue				(-10)
#define DiagonallyConnectedPawnValue	(  5)
#define HorizontallyConnectedPawnValue  (  2)	

static int WhitePawnValueMap[64] = {
	  0,  0,  0,  0,  0,  0,  0,  0,
	  5, 10, 15, 24, 24, 15, 10,  5,
	  4,  8, 12, 20, 20, 12,  8,  4,
	  3,  6,  9, 16, 16,  9,  6,  3,
	  2,  4,  6, 12, 12,  6,  4,  2,
	  1,  2,  3,  3,  3,  3,  2,  1,
	  0,  0,  0, -9, -9,  0,  0,  0,	  
	  0,  0,  0,  0,  0,  0,  0,  0
};

static int BlackPawnValueMap[64] = {
	  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0, -9, -9,  0,  0,  0,
	  1,  2,  3,  3,  3,  3,  2,  1,
	  2,  4,  6, 12, 12,  6,  4,  2,
	  3,  6,  9, 16, 16,  9,  6,  3,
	  4,  8, 12, 20, 20, 12,  8,  4,
	  5, 10, 15, 24, 24, 15, 10,  5,
	  0,  0,  0,  0,  0,  0,  0,  0
};

static int KnightValueMap[64] = {
	 -9, -5, -5, -5, -5, -5, -5, -9,
	 -5,  0,  0,  3,  3,  0,  0, -5,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	 -5,  0,  5,  9,  9,  5,  0, -5,
	 -5,  0,  5,  9,  9,  5,  0, -5,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	 -5,  0,  0,  3,  3,  0,  0, -5,
	 -9, -5, -5, -5, -5, -5, -5, -9,
};

static int BishopValueMap[64] = {
	 -5, -5, -5, -5, -5, -5, -5, -5,
	 -4,  5,  5, 10, 10,  5,  5, -4,
	 -3,  5,  8, 12, 12,  8,  5, -3,
	 -3,  5, 12,  3,  3, 12,  5, -3,
	 -3,  5, 12,  3,  3, 12,  5, -3,
	 -3,  5,  8, 12, 12,  8,  5, -3,
	 -4,  5,  5, 10, 10,  5,  5, -4,
	 -5, -5, -5, -5, -5, -5, -5, -5,
};


#define WHITE_PAWN_POSITION_VALUE(p)	(WhitePawnValueMap[CONVERT_256_TO_64((p))])
#define BLACK_PAWN_POSITION_VALUE(p)	(BlackPawnValueMap[CONVERT_256_TO_64((p))])
#define KNIGHT_POSITION_VALUE(p)		(KnightValueMap[CONVERT_256_TO_64((p))])
#define BISHOP_POSITION_VALUE(p)		(BishopValueMap[CONVERT_256_TO_64((p))])
#define ROOK_POSITION_VALUE(p)			(0)
#define QUEEN_POSITION_VALUE(p)			(0)

int evaluate_board(board_t * board);

#endif