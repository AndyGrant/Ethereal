/*
  Ethereal is a UCI chess playing engine authored by Andrew Grant.
  <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>
  
  Ethereal is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  Ethereal is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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