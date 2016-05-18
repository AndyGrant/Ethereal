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
#define RookValue   (  505)
#define QueenValue  ( 1000)
#define KingValue   (  100)

// KING EVALUATION TERMS
#define KING_HAS_CASTLED    (10)
#define KING_CAN_CASTLE     ( 5)

// ROOK EVALUATION TERMS
#define ROOK_OPEN_FILE_MID   (25)
#define ROOK_OPEN_FILE_END   (30)
#define ROOK_SEMI_FILE_MID   (12)
#define ROOK_SEMI_FILE_END   (12)

// BISHOP EVALUATION TERMS
#define BISHOP_PAIR_MID      ( 36)
#define BISHOP_PAIR_END      ( 46)
#define BISHOP_HAS_WINGS_MID (  9)
#define BISHOP_HAS_WINGS_END ( 36)

static int BishopOutpost[2][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 1, 2, 4, 4, 2, 1, 0, 
      0, 2, 4, 8, 8, 4, 2, 0, 
      0, 1, 6, 9, 9, 6, 1, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, },
      
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 1, 6, 9, 9, 6, 1, 0, 
      0, 2, 4, 8, 8, 4, 2, 0, 
      0, 1, 2, 4, 4, 2, 1, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, }
};

// KNIGHT EVALUATION TERMS
static int KnightOutpost[2][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 2, 4, 6, 6, 4, 2, 0, 
      0, 3, 6, 8, 8, 6, 3, 0, 
      0, 4, 8,10,10, 8, 4, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, },
      
    { 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 4, 8,10,10, 8, 4, 0, 
      0, 3, 6, 8, 8, 6, 3, 0, 
      0, 2, 4, 6, 6, 4, 2, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, }
};

// PAWN EVALUATION TERMS
#define PAWN_STACKED_MID    (10)
#define PAWN_STACKED_END    (20)
#define PAWN_ISOLATED_MID   (10)
#define PAWN_ISOLATED_END   (20)

static int PawnPassedMid[8]   = { 0, 10, 10, 15, 21, 28, 40, 0};
static int PawnPassedEnd[8]   = { 0, 10, 10, 15, 21, 28, 40, 0};

static int PawnConnected[2][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0,
      2, 2, 2, 3, 3, 2, 2, 2,
      4, 4, 5, 6, 6, 5, 4, 4,
      7, 8,10,12,12,10, 8, 7,
     11,14,17,21,21,17,14,11,
     16,21,25,33,33,25,12,16,
     32,42,50,55,55,50,42,32,
      0, 0, 0, 0, 0, 0, 0, 0, },
      
    { 0, 0, 0, 0, 0, 0, 0, 0,
     32,42,50,55,55,50,42,32,
     16,21,25,33,33,25,12,16,
     11,14,17,21,21,17,14,11,
      7, 8,10,12,12,10, 8, 7,
      4, 4, 5, 6, 6, 5, 4, 4,
      2, 2, 2, 3, 3, 2, 2, 2,
      0, 0, 0, 0, 0, 0, 0, 0, }
};

// OTHER TERMS
#define PSQT_MULTIPLIER (1)
static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

#endif