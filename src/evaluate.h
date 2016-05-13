#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

#define PawnValue   (  100)
#define KnightValue (  325)
#define BishopValue (  325)
#define RookValue   (  500)
#define QueenValue  (  975)
#define KingValue   (   50)

#define TEMPO_MID   (8)
#define TEMPO_END   (6)

#define PSQT_MULTIPLIER (1)

#define ROOK_OPEN_FILE_MID         (35)
#define ROOK_OPEN_FILE_END         (20)
#define ROOK_SEMI_FILE_MID         (10)
#define ROOK_SEMI_FILE_END         (10)
#define ROOK_ON_7TH_MID            (20)
#define ROOK_ON_7TH_END            (30)
#define ROOK_STACKED_MID           ( 6)
#define ROOK_STACKED_END           ( 8)

#define BISHOP_PAIR_MID            (36)
#define BISHOP_PAIR_END            (56)
#define BISHOP_HAS_WINGS_MID       (14)
#define BISHOP_HAS_WINGS_END       (28)

#define KING_HAS_FORWARD_PAWN_MID  ( 5)
#define KING_HAS_FORWARD_PAWN_END  ( 3)

static int PawnStackedMid[8]  = { 5,  7,  9,  9,  9,  9,  7,  5};
static int PawnStackedEnd[8]  = {15, 17, 19, 19, 19, 19, 17, 15};

static int PawnIsolatedMid[8] = {14, 20, 22, 23, 23, 22, 20, 14};
static int PawnIsolatedEnd[8] = {10, 12, 14, 14, 14, 14, 12, 10};

static int PawnPassedMid[8]   = { 0,  0,  0,  3,  7, 14, 23,  0};
static int PawnPassedEnd[8]   = { 0,  0,  0,  3,  7, 14, 23,  0};

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int evaluate_board(Board * board);

#endif
