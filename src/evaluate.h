#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"


// PROTOTYPES
int evaluate_board(Board * board);

#define PSQT_MULTIPLIER (2.5)

// PIECE VALUES
#define PawnValue   (  200)
#define KnightValue (  700)
#define BishopValue (  700)
#define RookValue   ( 1000)
#define QueenValue  ( 1975)
#define KingValue   (  100)

// PIECE VALUES LOOKUP
static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

// ROOK EVALUATION BONUSES
#define ROOK_OPEN_FILE_MID   (25)
#define ROOK_OPEN_FILE_END   (30)
#define ROOK_SEMI_FILE_MID   (10)
#define ROOK_SEMI_FILE_END   (10)
#define ROOK_ON_7TH_MID      (15)
#define ROOK_ON_7TH_END      (25)
#define ROOK_STACKED_MID     ( 7)
#define ROOK_STACKED_END     ( 7)

// BISHOP EVALUATION BONUSES
#define BISHOP_PAIR_MID      ( 72)
#define BISHOP_PAIR_END      (136)
#define BISHOP_HAS_WINGS_MID ( 32)
#define BISHOP_HAS_WINGS_END ( 64)

static int BishopOutpost[2][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 2, 4, 4, 4, 4, 2, 0, 
      0, 7, 9, 9, 9, 9, 7, 0, 
      0, 6, 8, 8, 8, 8, 6, 0, 
      0, 0, 2, 2, 2, 2, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, },
      
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 2, 2, 2, 2, 0, 0, 
      0, 6, 8, 8, 8, 8, 6, 0, 
      0, 7, 9, 9, 9, 9, 7, 0, 
      0, 2, 4, 4, 4, 4, 2, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, }
};

// KNIGHT EVALUATION BONUSES
static int KnightOutpost[2][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 2, 6, 6, 6, 6, 2, 0,
      0, 4, 7, 9, 9, 7, 4, 0,
      0, 2, 6, 6, 6, 6, 2, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, },
      
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 2, 6, 6, 6, 6, 2, 0,
      0, 4, 7, 9, 9, 7, 4, 0, 
      0, 2, 6, 6, 6, 6, 2, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, }
};

// PAWN EVALUATION BONUSES
static int PawnStackedMid[8]  = {10, 18, 24, 26, 26, 24, 18, 10};
static int PawnStackedEnd[8]  = {30, 38, 44, 46, 46, 46, 38, 30};

static int PawnIsolatedMid[8] = {28, 40, 52, 52, 52, 52, 40, 28};
static int PawnIsolatedEnd[8] = {20, 32, 36, 36, 36, 36, 32, 20};

static int PawnPassedMid[8]   = { 0,  0,  0, 10, 20, 40, 80,  0};
static int PawnPassedEnd[8]   = { 0,  0,  0, 10, 20, 40, 80,  0};

#endif
