#ifndef _CASTLE_H
#define _CASTLE_H

#include <stdint.h> // For uint64_t

/* To determine if empty squares between king and castling rook */
static uint64_t WhiteCastleKingSideMap  = (1ull <<  5) + (1ull <<  6);
static uint64_t BlackCastleKingSideMap  = (1ull << 61) + (1ull << 62);
static uint64_t WhiteCastleQueenSideMap = (1ull <<  1) + (1ull <<  2) + (1ull << 3);
static uint64_t BlackCastleQueenSideMap = (1ull << 57) + (1ull << 58) + (1ull << 59);

#define WhiteKingRights		(1)
#define WhiteQueenRights	(2)
#define BlackKingRights		(4)
#define BlackQueenRights	(8)

static int CastleMask[64] = {
	13, 15, 15, 15, 12, 15, 15, 14,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	 7, 15, 15, 15,  3, 15, 15, 11 
};

#endif