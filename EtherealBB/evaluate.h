#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

#define PawnValue   ( 100)
#define KnightValue ( 300)
#define BishopValue ( 310)
#define RookValue   ( 500)
#define QueenValue  ( 925)
#define KingValue   (  50)

#define PAWN_STACKED_PENALTY   	( 45)
#define PAWN_ISOLATED_PENALTY  	( 20)
#define PAWN_7TH_RANK_VALUE    	( 95)
	
#define ROOK_7TH_RANK_VALUE    	( 35)
#define ROOK_8TH_RANK_VALUE    	( 40)
#define ROOK_ON_SAME_FILE_VALUE	( 65)
	
#define KNIGHT_ATTACK_VALUE	   	( 13)
#define KNIGHT_DEFEND_VALUE	   	(  7)
	
#define PAWN_DEFEND_PAWN_VALUE 	(  9)

static int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int evaluate_board(Board * board);

#endif
