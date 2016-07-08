#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

int evaluateBoard(Board * board, PawnTable * ptable);
void evaluatePawns(int * mid, int * end, Board* board, int * pawnCount);
void evaluateKnights(int * mid, int *end, Board* board, int * knightCount);
void evaluateBishops(int * mid, int * end, Board* board, int * bishopCount);
void evaluateRooks(int * mid, int * end, Board* board, int * rookCount);
void evaluateKings(int * mid, int * end, Board* board);

#define PawnValue   ( 100)
#define KnightValue ( 325)
#define BishopValue ( 325)
#define RookValue   ( 505)
#define QueenValue  (1000)
#define KingValue   ( 100)

#define KING_HAS_CASTLED     (10)
#define KING_CAN_CASTLE      ( 5)

#define ROOK_OPEN_FILE_MID   (35)
#define ROOK_OPEN_FILE_END   (20)
#define ROOK_SEMI_FILE_MID   (12)
#define ROOK_SEMI_FILE_END   (12)
#define ROOK_STACKED_MID     ( 8)
#define ROOK_STACKED_END     ( 8)
#define ROOK_ON_7TH_MID      (10)
#define ROOK_ON_7TH_END      (15)

#define BISHOP_PAIR_MID      (46)
#define BISHOP_PAIR_END      (64)
#define BISHOP_HAS_WINGS_MID (13)
#define BISHOP_HAS_WINGS_END (36)

#define PAWN_STACKED_MID     (10)
#define PAWN_STACKED_END     (20)
#define PAWN_ISOLATED_MID    (10)
#define PAWN_ISOLATED_END    (20)

#define PSQT_MULTIPLIER      (1)

extern unsigned int BishopOutpost[2][64];
extern unsigned int KnightOutpost[2][64];
extern int PawnConnected[2][64];
extern int PawnPassedMid[8];
extern int PawnPassedEnd[8];
extern int PieceValues[8];
extern int BishopMobility[13];
extern int RookMobility[14];

#endif