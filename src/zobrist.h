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

#ifndef _ZORBIST_H
#define _ZORBIST_H

#include <stdint.h>

#include "types.h"

#define CASTLE (2)
#define ENPASS (3)
#define TURN   (6)

void initializeZorbist();
uint64_t rand64();

extern uint64_t ZobristKeys[32][SQUARE_NB];
extern uint64_t PawnKingKeys[32][SQUARE_NB];

#endif
