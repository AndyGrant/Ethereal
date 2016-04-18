#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

#define PawnValue   (  100)
#define KnightValue (  400)
#define BishopValue (  400)
#define RookValue   (  600)
#define QueenValue  ( 1200)
#define KingValue   (   50)

#define PAWN_STACKED_MID     (15)
#define PAWN_STACKED_END     (35)
#define PAWN_ISOLATED_MID    (20)
#define PAWN_ISOLATED_END    (10)
#define PAWN_7TH_RANK_MID    (35)
#define PAWN_7TH_RANK_END    (55)

#define ROOK_OPEN_FILE_MID   (55)
#define ROOK_OPEN_FILE_END   (45)
#define ROOK_SEMI_FILE_MID   (25)
#define ROOK_SEMI_FILE_END   (25)
#define ROOK_ON_7TH_MID      (35)
#define ROOK_ON_7TH_END      (15)

#define BISHOP_PAIR_MID      (32)
#define BISHOP_PAIR_END      (64)
#define BISHOP_HAS_WINGS_MID (18)
#define BISHOP_HAS_WINGS_END (42)

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int evaluate_board(Board * board);

#endif
