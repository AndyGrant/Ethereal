#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

#define PawnValue   (  200)
#define KnightValue (  800)
#define BishopValue (  800)
#define RookValue   ( 1200)
#define QueenValue  ( 2400)
#define KingValue   (   50)

#define TEMPO_MID   (14)
#define TEMPO_END   (10)

#define ROOK_OPEN_FILE_MID   ( 9)
#define ROOK_OPEN_FILE_END   ( 7)

#define ROOK_SEMI_FILE_MID   ( 5)
#define ROOK_SEMI_FILE_END   ( 5)

#define ROOK_ON_7TH_MID      ( 8)
#define ROOK_ON_7TH_END      (12)

#define ROOK_STACKED_MID     ( 6)
#define ROOK_STACKED_END     ( 4)

#define BISHOP_PAIR_MID      (36)
#define BISHOP_PAIR_END      (63)

#define BISHOP_HAS_WINGS_MID ( 7)
#define BISHOP_HAS_WINGS_END (32)

static int PawnStackedMid[8]  = {10, 18, 24, 26, 26, 24, 18, 10};
static int PawnStackedEnd[8]  = {30, 38, 44, 46, 46, 44, 38, 30};

static int PawnIsolatedMid[8] = {28, 40, 52, 52, 52, 52, 40, 28};
static int PawnIsolatedEnd[8] = {20, 32, 36, 36, 36, 36, 32, 20};

static int PawnPassedMid[8]   = { 0,  0,  0, 14, 25, 31, 44,  0};
static int PawnPassedEnd[8]   = { 0,  0,  0, 14, 25, 31, 44,  0};

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int evaluate_board(Board * board);

#endif
