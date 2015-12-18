#ifndef MOVE_H
#define MOVE_H

#include "piece.h"
#include "types.h"

/* Move Type Definitions */
#define NormalFlag			(1 << 24)
#define CastleFlag			(1 << 25)
#define EnpassFlag			(1 << 26)
#define PromotionFlag		(1 << 27)
#define PromoteKnightFlag	(1 << 28)
#define PromoteBishopFlag	(1 << 29)
#define PromoteRookFlag		(1 << 30)
#define PromoteQueenFlag	(1 << 31) 
#define PromoteFlags		(15<< 28)

/* Move Type Macro Definitions */
#define MOVE_IS_NORMAL(move)		((move) & NormalFlag)
#define MOVE_IS_CASTLE(move)		((move) & CastleFlag)
#define MOVE_IS_ENPASS(move)		((move) & EnpassFlag)
#define MOVE_IS_PROMOTION(move)		((move) & PromoteFlags)

/* Move Creation Macro Definitions */
#define MAKE_NORMAL_MOVE(f,t,c,p)	((f)|((t)<<8)|((c)<<16)|(NormalFlag)|((p)<<28))
#define MAKE_CASTLE_MOVE(f,t,p)		((f)|((t)<<8)|(CastleFlag)|((p)<<28))
#define MAKE_ENPASS_MOVE(f,t,l)		((f)|((t)<<8)|((l)<<16)|(EnpassFlag))
#define MAKE_PROMOTION_MOVE(f,t,c,p)((f)|((t)<<8)|((c)<<16)|(PromotionFlag)|(p))

/* Needed for Macro Definition */
static int PromoteTypes[9] = {0, KnightFlag, BishopFlag, 0, RookFlag, 0, 0, 0, QueenFlag};

/* Needed for check validation */
static int knight_movements[8] = {33,31,18,14,-14,-18,-31,-33};
static int king_movements[8] = {-17,-15,15,17,-16,-1,1,16};





/* Move Decode Macro Definitions */
#define MOVE_GET_FROM(m)			((m & (0b11111111 <<  0)) >>  0)
#define MOVE_GET_TO(m)				((m & (0b11111111 <<  8)) >>  8)
#define MOVE_GET_CAPTURE(m)			((m & (0b11111111 << 16)) >> 16)
#define MOVE_GET_ENPASS_SQUARE(m)	((m & (0b11111111 << 16)) >> 16)
#define MOVE_GET_CASTLE_FLAGS(m)	((m & (0b00001111 << 28)) >> 28)
#define MOVE_GET_PROMOTE_TYPE(m,c)	(PromoteTypes[((m&PromoteFlags)>>28)]+c)

/* Function Prototypes */
void gen_all_moves(board_t * board, move_t * list, int * size);
void apply_move(board_t * board, move_t move);
void revert_move(board_t * board, move_t move);

void insert_position(board_t * board, int to);
void remove_position(board_t * board, int to);

int is_not_in_check(board_t * board, int turn);
int square_is_attacked(board_t * board, int turn, int square);
	
#endif