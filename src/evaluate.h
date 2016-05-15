#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

// PROTOTYPES
int evaluate_board(Board * board);
void evaluateWhiteOutpost(int* mid, int* end, int isKnight, int sq, uint64_t empty, uint64_t wpawns, uint64_t bpawns);
void evaluateBlackOutpost(int* mid, int* end, int isKnight, int sq, uint64_t empty, uint64_t wpawns, uint64_t bpawns);

// PIECE VALUES
#define PawnValue   (  200)
#define KnightValue (  650)
#define BishopValue (  650)
#define RookValue   ( 1025)
#define QueenValue  ( 1950)
#define KingValue   (  100)

// KING EVALUATION TERMS
#define KING_HAS_CASTLED    (25)
#define KING_CAN_CASTLE     (10)

// ROOK EVALUATION TERMS
#define ROOK_OPEN_FILE_MID   (25)
#define ROOK_OPEN_FILE_END   (30)
#define ROOK_SEMI_FILE_MID   (10)
#define ROOK_SEMI_FILE_END   (10)
#define ROOK_ON_7TH_MID      (15)
#define ROOK_ON_7TH_END      (25)
#define ROOK_STACKED_MID     ( 7)
#define ROOK_STACKED_END     ( 7)

// BISHOP EVALUATION TERMS
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
      0, 0, 1, 1, 1, 1, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, },
      
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 1, 1, 1, 1, 0, 0, 
      0, 6, 8, 8, 8, 8, 6, 0, 
      0, 7, 9, 9, 9, 9, 7, 0, 
      0, 2, 4, 4, 4, 4, 2, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, }
};

// KNIGHT EVALUATION TERMS
static int KnightOutpost[2][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 2, 6, 6, 6, 6, 2, 0,
      0, 4, 7, 9, 9, 7, 4, 0,
      0, 2, 6, 6, 6, 6, 2, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, },
      
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 2, 6, 6, 6, 6, 2, 0,
      0, 4, 7, 9, 9, 7, 4, 0, 
      0, 2, 6, 6, 6, 6, 2, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, }
};

// PAWN EVALUATION TERMS
static int PawnStackedMid[8]  = {10, 18, 24, 26, 26, 24, 18, 10};
static int PawnStackedEnd[8]  = {30, 38, 44, 46, 46, 46, 38, 30};

static int PawnIsolatedMid[8] = {28, 40, 52, 52, 52, 52, 40, 28};
static int PawnIsolatedEnd[8] = {20, 32, 36, 36, 36, 36, 32, 20};

static int PawnPassedMid[8]   = { 0,  0,  0, 10, 20, 40, 80,  0};
static int PawnPassedEnd[8]   = { 0,  0,  0, 10, 20, 40, 80,  0};

// OTHER TERMS
#define PSQT_MULTIPLIER (2.5)
static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

#endif
