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

#define ColourWhite (0)
#define ColourBlack (1)
#define ColourNone  (2)

#define WhitePawn   (0)
#define BlackPawn   (1)
#define WhiteKnight (4)
#define BlackKnight (5)
#define WhiteBishop (8)
#define BlackBishop (9)
#define WhiteRook   (12)
#define BlackRook   (13)
#define WhiteQueen  (16)
#define BlackQueen  (17)
#define WhiteKing   (20)
#define BlackKing   (21)
#define Empty       (26)

#define PawnFlag    (0)
#define KnightFlag  (4)
#define BishopFlag  (8)
#define RookFlag    (12)
#define QueenFlag   (16)
#define KingFlag    (20)

#define PieceType(piece)       ((piece) >> 2)
#define PieceColour(piece)     ((piece) & 3)

#define MakePiece(flag,colour) ((flag) + (colour))

#endif