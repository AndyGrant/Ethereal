#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

#define PawnValue   (  200)
#define KnightValue (  800)
#define BishopValue (  800)
#define RookValue   ( 1200)
#define QueenValue  ( 2400)
#define KingValue   (   50)

#define ROOK_OPEN_FILE_MID   ( 0)
#define ROOK_OPEN_FILE_END   ( 0)
#define ROOK_SEMI_FILE_MID   ( 0)
#define ROOK_SEMI_FILE_END   ( 0)
#define ROOK_ON_7TH_MID      (15)
#define ROOK_ON_7TH_END      ( 6)
#define ROOK_STACKED_MID     (19)
#define ROOK_STACKED_END     (19)

#define BISHOP_PAIR_MID      (36)
#define BISHOP_PAIR_END      (63)
#define BISHOP_HAS_WINGS_MID ( 7)
#define BISHOP_HAS_WINGS_END (32)

static int PawnStackedMid[8]  = { 5,  9, 12, 13, 13, 12,  9,  5};
static int PawnStackedEnd[8]  = {15, 19, 22, 23, 23, 22, 19, 15};

static int PawnIsolatedMid[8] = {14, 20, 26, 26, 26, 26, 20, 14};
static int PawnIsolatedEnd[8] = {10, 16, 18, 18, 18, 18, 16, 10};

static int PawnPassedMid[8]   = { 0,  0,  0, 10, 15, 24, 33,  0};
static int PawnPassedEnd[8]   = { 0,  0,  0, 10, 15, 24, 33,  0};

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int evaluate_board(Board * board);

#endif
