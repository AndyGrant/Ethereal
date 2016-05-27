#ifndef _CASTLE_H
#define _CASTLE_H

#include <stdint.h>

static uint64_t WhiteCastleKingSideMap  = (1ull <<  5) + (1ull <<  6);
static uint64_t BlackCastleKingSideMap  = (1ull << 61) + (1ull << 62);
static uint64_t WhiteCastleQueenSideMap = (1ull <<  1) + (1ull <<  2) + (1ull << 3);
static uint64_t BlackCastleQueenSideMap = (1ull << 57) + (1ull << 58) + (1ull << 59);

#define WhiteKingRights     (0b0001)
#define WhiteQueenRights    (0b0010)
#define BlackKingRights     (0b0100)
#define BlackQueenRights    (0b1000)

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

static int RookFromLookupTable[5] = {-4,0,0,0,3};
static int RookToLookupTable[5] = {-1,0,0,0,1};

#define CastleGetRookFrom(from,to)   (from + RookFromLookupTable[2+to-from])
#define CastleGetRookTo(from,to)     (from + RookToLookupTable[2+to-from])

#endif