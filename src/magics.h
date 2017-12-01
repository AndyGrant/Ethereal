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

#ifndef _MAGICS_H
#define _MAGICS_H

#include <stdio.h>

#include "types.h"

void initalizeMagics();
void generateKnightMap();
void generateKingMap();
void generateRookIndexes();
void generateBishopIndexes();
void generateOccupancyMaskRook();
void generateOccupancyMaskBishop();
void generateOccupancyVariationsRook();
void generateOccupancyVariationsBishop();
void generateMoveDatabaseRook();
void generateMoveDatabaseBishop();

extern uint64_t KnightMap[SQUARE_NB];
extern uint64_t KingMap[SQUARE_NB];

extern uint64_t OccupancyMaskRook[SQUARE_NB];
extern uint64_t OccupancyMaskBishop[SQUARE_NB];

extern uint64_t * MoveDatabaseRook;
extern uint64_t * MoveDatabaseBishop;

extern int MagicRookIndexes[SQUARE_NB];
extern int MagicBishopIndexes[SQUARE_NB];

extern const int MagicShiftsRook[SQUARE_NB];
extern const int MagicShiftsBishop[SQUARE_NB];

extern const uint64_t MagicNumberRook[SQUARE_NB];
extern const uint64_t MagicNumberBishop[SQUARE_NB];

#endif
