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

void initalizeMagics();
void generateKnightMap();
void generateKingMap();
void generateOccupancyMaskRook();
void generateOccupancyMaskBishop();
void generateOccupancyVariationsRook();
void generateOccupancyVariationsBishop();
void generateMoveDatabaseRook();
void generateMoveDatabaseBishop();

extern uint64_t KnightMap[64];
extern uint64_t KingMap[64];

extern uint64_t OccupancyMaskRook[64];
extern uint64_t OccupancyMaskBishop[64];

extern uint64_t ** OccupancyVariationsRook;
extern uint64_t ** OccupancyVariationsBishop;

extern uint64_t * MoveDatabaseRook;
extern uint64_t * MoveDatabaseBishop;

extern int MagicRookIndexes[64];
extern int MagicBishopIndexes[64];

extern int MagicShiftsRook[64];
extern int MagicShiftsBishop[64];

extern uint64_t MagicNumberRook[64];
extern uint64_t MagicNumberBishop[64];

#endif