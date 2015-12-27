#ifndef CASTLE_H
#define CASTLE_H

#include "board.h"
#include "castle.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "types.h"
#include "util.h"

/* Castle Right Definitions */
#define WhiteKingCastle		(1 << 0)
#define WhiteQueenCastle	(1 << 1)
#define BlackKingCastle		(1 << 2)
#define BlackQueenCastle	(1 << 3)

#define KING_HAS_RIGHTS(t,r)	((r) & (1 << ((t)*2)))
#define QUEEN_HAS_RIGHTS(t,r)	((r) & (2 << ((t)*2)))
#define GET_RIGHTS(t,b)			((b)->castle_rights & (3 << (t*2)))

#define CREATE_KING_RIGHTS(t)	(1 << ((t)*2))
#define CREATE_QUEEN_RIGHTS(t)	(2 << ((t)*2))

#define IS_LEFT_ROOK(t,s)		((s)==(180-((t)*(180-68))))
#define IS_RIGHT_ROOK(t,s)		((s)==(187-((t)*(187-75))))

/* Castle Macro Definitions */
#define CREATE_CASTLE_RIGHTS(a,b,c,d) (((a)<<0)+((b)<<1)+((c)<<2)+((d)<<3))

#endif