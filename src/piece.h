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