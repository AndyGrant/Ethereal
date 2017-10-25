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

#ifndef _SQUARE_H
#define _SQUARE_H

#include "types.h"

extern const int SQTable[FILE_NB];

extern const int SQInv[SQUARE_NB];

#define RelativeSquare(sq, c) ((c) == WHITE ? (sq) : SQInv[(sq)])

#define RelativeSquare32(sq, c) ((c) == WHITE ? Square32(sq) : Square32(SQInv[(sq)]))

#define Square32(sq) ((((sq) >> 3) << 2) + SQTable[(sq) & 0x7])

#endif
