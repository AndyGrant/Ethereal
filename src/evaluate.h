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
#define KnightValue (  325)
#define BishopValue (  325)
#define RookValue   (  500)
#define QueenValue  (  975)
#define KingValue   (  100)

// KING EVALUATION TERMS
#define KING_HAS_CASTLED    (25)
#define KING_CAN_CASTLE     (10)

// ROOK EVALUATION TERMS
#define ROOK_OPEN_FILE_MID   (20)
#define ROOK_OPEN_FILE_END   (25)
#define ROOK_SEMI_FILE_MID   ( 5)
#define ROOK_SEMI_FILE_END   ( 5)
#define ROOK_ON_7TH_MID      ( 6)
#define ROOK_ON_7TH_END      ( 6)
#define ROOK_STACKED_MID     ( 7)
#define ROOK_STACKED_END     ( 7)

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
static int PawnStackedMid[8]  = { 5,  6,  8,  8,  8,  8,  6,  5};
static int PawnStackedEnd[8]  = {15, 18, 21, 21, 21, 21, 18, 15};

static int PawnIsolatedMid[8] = {13, 17, 21, 21, 21, 21, 17, 13};
static int PawnIsolatedEnd[8] = {10, 13, 16, 16, 16, 16, 13, 10};

static int PawnPassedMid[8]   = { 0,  0,  0,  6, 12, 20, 32,  0};
static int PawnPassedEnd[8]   = { 0,  0,  0,  6, 12, 20, 32,  0};

// OTHER TERMS
#define PSQT_MULTIPLIER (1)
static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

#endif
