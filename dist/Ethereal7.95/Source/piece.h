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

#ifndef _PIECE_H
#define _PIECE_H

#define WHITE       (0)
#define BLACK       (1)

#define PAWN    (0)
#define KNIGHT  (1)
#define BISHOP  (2)
#define ROOK    (3)
#define QUEEN   (4)
#define KING    (5)

#define WHITE_PAWN      ( 0)
#define BLACK_PAWN      ( 1)
#define WHITE_KNIGHT    ( 4)
#define BLACK_KNIGHT    ( 5)
#define WHITE_BISHOP    ( 8)
#define BLACK_BISHOP    ( 9)
#define WHITE_ROOK      (12)
#define BLACK_ROOK      (13)
#define WHITE_QUEEN     (16)
#define BLACK_QUEEN     (17)
#define WHITE_KING      (20)
#define BLACK_KING      (21)
#define EMPTY           (26)

#define PAWN_FLAG   ( 0)
#define KNIGHT_FLAG ( 4)
#define BISHOP_FLAG ( 8)
#define ROOK_FLAG   (12)
#define QUEEN_FLAG  (16)
#define KING_FLAG   (20)

#define PieceType(piece)       ((piece) >> 2)
#define PieceColour(piece)     ((piece) & 3)

#define MakePiece(flag,colour) ((flag) + (colour))

#endif