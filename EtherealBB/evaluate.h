#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

#define PawnValue   (  100)
#define KnightValue (  400)
#define BishopValue (  400)
#define RookValue   (  600)
#define QueenValue  ( 1200)
#define KingValue   (   50)

#define ROOK_OPEN_FILE_MID   (25)
#define ROOK_OPEN_FILE_END   (15)
#define ROOK_SEMI_FILE_MID   ( 8)
#define ROOK_SEMI_FILE_END   ( 8)
#define ROOK_ON_7TH_MID      (15)
#define ROOK_ON_7TH_END      ( 6)

#define BISHOP_PAIR_MID      ( 9)
#define BISHOP_PAIR_END      (17)
#define BISHOP_HAS_WINGS_MID ( 7)
#define BISHOP_HAS_WINGS_END (32)

static int PawnStackedMid[8]  = { 5,  9, 12, 13, 13, 12,  9,  5};
static int PawnStackedEnd[8]  = {15, 19, 22, 23, 23, 22, 19, 15};

static int PawnIsolatedMid[8] = {14, 20, 26, 26, 26, 26, 20, 14};
static int PawnIsolatedEnd[8] = {10, 16, 18, 18, 18, 18, 16, 10};

static int PawnPassedMid[8]   = { 0,  0,  0,  8, 12, 20, 28,  0};
static int PawnPassedEnd[8]   = { 0,  0,  0,  8, 12, 20, 28,  0};

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int evaluate_board(Board * board);

#endif
