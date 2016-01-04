#ifndef EVALUATE_H
#define EVALUATE_H

#include "util.h"

/* Search Definitions */
#define CheckMate	(32768 - 128)

/* Piece Value Definitions */
#define PawnValue 	(105)
#define KnightValue	(325)
#define BishopValue	(300)
#define RookValue 	(515)
#define QueenValue 	(985)

#define StackedPawnValue				(-10)
#define IsolatedPawnValue				(-15)
#define PassedPawnValue					(  8)
#define PawnInfrontOfKingValue			(  5)

#define OpenFileRookValue				( 25)
#define SemiOpenFileRookValue			( 10)
#define SeventhRankRookValue			( 20)
#define EightRankRookValue				(  8)
#define RooksOnSameOpenFileValue		( 40)
#define RooksOnSameSemiOpenFileValue	( 15)

#define HasBothBishopsValue				( 25)

static int PawnEarlyValueMap[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	  5,  10,  15,  20,  20,  15,  10,   5,
	  4,   8,  12,  16,  16,  12,   8,   4,
	  3,   6,   9,  12,  12,   9,   6,   3,
	  2,   4,   6,   8,   8,   6,   4,   2,
	  1,   2,   3, -10, -10,   3,   2,   1,
	  0,   0,   0, -35, -35,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0
};

static int PawnEndValueMap[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	 68,  89,  99,  99,  99,  99,  89,   68,
	 46,  56,  61,  66,  66,  61,  56,   46,
	 26,  34,  39,  44,  44,  39,  34,   26,
	 12,  14,  26,  33,  33,  26,  14,   12,
	  6,   8,  13, -10, -10,  13,   8,   6,
	  0,   0,   0, -35, -35,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0
};

static int KnightValueMap[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -30, -10, -10, -10, -10, -30, -10
};

static int BishopValueMap[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -10, -20, -10, -10, -20, -10, -10
};

static int KingEarlyValueMap[64] = {
	-50, -50, -50, -50, -50, -50, -50, -50,
	-50, -50, -50, -50, -50, -50, -50, -50,
	-50, -50, -50, -50, -50, -50, -50, -50,
	-50, -50, -50, -50, -50, -50, -50, -50,
	-50, -50, -50, -50, -50, -50, -50, -50,
	-50, -50, -50, -50, -50, -50, -50, -50,
	-30, -30, -30, -60, -60, -60, -30, -30,
	  0,  20,  40, -60,   0, -60,  40,  20
};

static int KingEndValueMap[64] = {
	  0,  10,  20,  30,  30,  20,  10,   0,
	 10,  20,  30,  40,  40,  30,  20,  10,
	 20,  30,  40,  50,  50,  40,  30,  20,
	 30,  40,  50,  60,  60,  50,  40,  30,
	 30,  40,  50,  60,  60,  50,  40,  30,
	 20,  30,  40,  50,  50,  40,  30,  20,
	 10,  20,  30,  40,  40,  30,  20,  10,
	  0,  10,  20,  30,  30,  20,  10,   0
};

static int inv[64] = {
	 56,  57,  58,  59,  60,  61,  62,  63,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 16,  17,  18,  19,  20,  21,  22,  23,
	  8,   9,  10,  11,  12,  13,  14,  15,
	  0,   1,   2,   3,   4,   5,   6,   7
};

int evaluate_board();
int evaluate_player(int turn, int pawn_info[2][10]);

#endif