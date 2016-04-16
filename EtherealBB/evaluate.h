#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

#define PawnValue   ( 100)
#define KnightValue ( 300)
#define BishopValue ( 300)
#define RookValue   ( 500)
#define QueenValue  ( 975)
#define KingValue   (  50)

#define PAWN_STACKED_MID     (30)
#define PAWN_STACKED_END     (45)
#define PAWN_ISOLATED_MID    (20)
#define PAWN_ISOLATED_END    (15)
#define PAWN_7TH_RANK_MID    (35)
#define PAWN_7TH_RANK_END    (95)

#define ROOK_OPEN_FILE_MID   (35)
#define ROOK_OPEN_FILE_END   (45)
#define ROOK_SEMI_FILE_MID   (10)
#define ROOK_SEMI_FILE_END   (10)
#define ROOK_ON_7TH_MID      (17)
#define ROOK_ON_7TH_END      (17)

#define BISHOP_PAIR_MID      ( 7)
#define BISHOP_PAIR_END      (14)
#define BISHOP_HAS_WINGS_MID ( 5)
#define BISHOP_HAS_WINGS_END ( 7)

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int evaluate_board(Board * board);

#endif
