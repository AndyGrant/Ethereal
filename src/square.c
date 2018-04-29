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

#include "piece.h"
#include "square.h"
#include "types.h"

const int SQInv[SQUARE_NB] = {
    56,  57,  58,  59,  60,  61,  62,  63,
    48,  49,  50,  51,  52,  53,  54,  55,
    40,  41,  42,  43,  44,  45,  46,  47,
    32,  33,  34,  35,  36,  37,  38,  39,
    24,  25,  26,  27,  28,  29,  30,  31,
    16,  17,  18,  19,  20,  21,  22,  23,
     8,   9,  10,  11,  12,  13,  14,  15,
     0,   1,   2,   3,   4,   5,   6,   7,
};

int relativeSquare(int sq, int colour){
    return colour == WHITE ? sq : SQInv[sq];
}

int relativeSquare32(int sq, int colour){
    return colour == WHITE ? square32(sq) : square32(SQInv[sq]);
}

int square32(int sq){
    static const int table[FILE_NB] = {0, 1, 2, 3, 3, 2, 1, 0};
    return ((sq >> 3) << 2) + table[sq & 0x7];
}
