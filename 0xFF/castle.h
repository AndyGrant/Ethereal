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

/* Castle Macro Definitions */
#define CREATE_CASTLE_RIGHTS(a,b,c,d) ((a<<0)+(b<<1)+(c<<2)+(d<<3))

#endif