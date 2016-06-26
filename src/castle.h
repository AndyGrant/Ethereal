#ifndef _CASTLE_H
#define _CASTLE_H

#include <stdint.h>

#define WhiteCastleKingSideMap  ((1ull <<  5) + (1ull <<  6))
#define BlackCastleKingSideMap  ((1ull << 61) + (1ull << 62))
#define WhiteCastleQueenSideMap ((1ull <<  1) + (1ull <<  2) + (1ull <<  3))
#define BlackCastleQueenSideMap ((1ull << 57) + (1ull << 58) + (1ull << 59))

#define WhiteKingRights     (1)
#define WhiteQueenRights    (2)
#define BlackKingRights     (4)
#define BlackQueenRights    (8)

extern int CastleMask[64];

extern int RookFromLookupTable[5];
extern int RookToLookupTable[5];

#define CastleGetRookFrom(from,to)   (from + RookFromLookupTable[2+to-from])
#define CastleGetRookTo(from,to)     (from + RookToLookupTable[2+to-from])

#endif