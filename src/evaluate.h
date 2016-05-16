#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

// PROTOTYPES
int evaluate_board(Board * board);

void evaluatePawns(int* mid, int* end, Board* board);
void evaluateKnights(int* mid, int*end, Board* board);
void evaluateBishops(int* mid, int* end, Board* board);
void evaluateRooks(int* mid, int* end, Board* board);
void evaluateKings(int* mid, int* end, Board* board);

// PIECE VALUES
#define PawnValue   (  100)
#define KnightValue (  310)
#define BishopValue (  310)
#define RookValue   (  495)
#define QueenValue  (  995)
#define KingValue   (   50)

// KING EVALUATION TERMS
#define KING_HAS_CASTLED    (10)
#define KING_CAN_CASTLE     ( 5)

// ROOK EVALUATION TERMS
#define ROOK_OPEN_FILE_MID   (32)
#define ROOK_OPEN_FILE_END   (18)
#define ROOK_SEMI_FILE_MID   ( 8)
#define ROOK_SEMI_FILE_END   ( 8)
#define ROOK_ON_7TH_MID      (28)
#define ROOK_ON_7TH_END      (32)

// BISHOP EVALUATION TERMS
#define BISHOP_PAIR_MID      ( 36)
#define BISHOP_PAIR_END      ( 54)
#define BISHOP_HAS_WINGS_MID ( 16)
#define BISHOP_HAS_WINGS_END ( 32)

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
static int PawnStackedMid[8]  = { 5,  9, 12, 13, 13, 12,  9,  5};
static int PawnStackedEnd[8]  = {15, 19, 22, 23, 23, 22, 19, 15};

static int PawnIsolatedMid[8] = {14, 20, 26, 26, 26, 26, 20, 14};
static int PawnIsolatedEnd[8] = {10, 16, 18, 18, 18, 18, 16, 10};

static int PawnPassedMid[8]   = { 0,  0,  0, 10, 15, 24, 33,  0};
static int PawnPassedEnd[8]   = { 0,  0,  0, 10, 15, 24, 33,  0};

// OTHER TERMS
#define PSQT_MULTIPLIER (1)
static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

#endif