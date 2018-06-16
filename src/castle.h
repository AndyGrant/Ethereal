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

#include "types.h"

#define WHITE_CASTLE_KING_SIDE_MAP  ((1ull <<  5) | (1ull <<  6))
#define BLACK_CASTLE_KING_SIDE_MAP  ((1ull << 61) | (1ull << 62))
#define WHITE_CASTLE_QUEEN_SIDE_MAP ((1ull <<  1) | (1ull <<  2) | (1ull <<  3))
#define BLACK_CASTLE_QUEEN_SIDE_MAP ((1ull << 57) | (1ull << 58) | (1ull << 59))

#define WHITE_KING_RIGHTS     (1)
#define WHITE_QUEEN_RIGHTS    (2)
#define BLACK_KING_RIGHTS     (4)
#define BLACK_QUEEN_RIGHTS    (8)

int castleGetRookFrom(int from, int to);
int castleGetRookTo(int from, int to);

extern const int CastleMask[SQUARE_NB];

#endif
