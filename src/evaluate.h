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

#define ROOK_OPEN_FILE_MID   (15)
#define ROOK_OPEN_FILE_END   (10)

#define ROOK_SEMI_FILE_MID   ( 9)
#define ROOK_SEMI_FILE_END   ( 9)

#define ROOK_ON_7TH_MID      (10)
#define ROOK_ON_7TH_END      (15)

#define ROOK_STACKED_MID     ( 6)
#define ROOK_STACKED_END     ( 6)

#define BISHOP_PAIR_MID      (36)
#define BISHOP_PAIR_END      (48)

#define BISHOP_HAS_WINGS_MID (14)
#define BISHOP_HAS_WINGS_END (32)

static int PawnStackedMid[8]  = { 9, 13, 16, 17, 17, 16, 13,  9};
static int PawnStackedEnd[8]  = {21, 25, 28, 29, 29, 28, 25, 21};

static int PawnIsolatedMid[8] = {19, 25, 31, 31, 31, 31, 25, 19};
static int PawnIsolatedEnd[8] = {13, 19, 21, 21, 21, 21, 19, 13};

static int PawnPassedMid[8]   = { 0,  0,  0,  5, 13, 25, 46,  0};
static int PawnPassedEnd[8]   = { 0,  0,  0,  5, 13, 25, 46,  0};

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int evaluate_board(Board * board);

#endif
