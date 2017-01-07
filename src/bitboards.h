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

#ifndef _BITBOARDS_H
#define _BITBOARDS_H

#include <stdint.h>

#define RANK_8 (0xFF00000000000000ull)
#define RANK_7 (0x00FF000000000000ull)
#define RANK_6 (0x0000FF0000000000ull)
#define RANK_5 (0x000000FF00000000ull)
#define RANK_4 (0x00000000FF000000ull)
#define RANK_3 (0x0000000000FF0000ull)
#define RANK_2 (0x000000000000FF00ull)
#define RANK_1 (0x00000000000000FFull)

#define FILE_A (0x0101010101010101ull)
#define FILE_B (0x0202020202020202ull)
#define FILE_C (0x0404040404040404ull)
#define FILE_D (0x0808080808080808ull)
#define FILE_E (0x1010101010101010ull)
#define FILE_F (0x2020202020202020ull)
#define FILE_G (0x4040404040404040ull)
#define FILE_H (0x8080808080808080ull)

#define WHITE_SQUARES (0x55AA55AA55AA55AAull)
#define BLACK_SQUARES (0xAA55AA55AA55AA55ull)

extern uint64_t Files[8];
extern uint64_t Ranks[8];

#define File(sq)            ((sq) & 7)
#define Rank(sq)            ((sq) >> 3)

#endif