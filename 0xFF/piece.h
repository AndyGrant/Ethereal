#ifndef PIECE_H
#define PIECE_H

#include "colour.h"

/* Piece Flag Definitions */
#define PawnFlag		(1 << 2)
#define KnightFlag		(1 << 3)
#define BishopFlag		(1 << 4)
#define RookFlag		(1 << 5)
#define QueenFlag		(1 << 6)
#define KingFlag		(1 << 7)

/* Piece Definitions */
#define Wall			((1 << 8))
#define NonePiece		(NoneFlag	| ColourNone)
#define WhitePawn		(PawnFlag	| WhiteFlag)
#define BlackPawn		(PawnFlag	| BlackFlag)
#define WhiteKnight		(KnightFlag	| WhiteFlag)
#define BlackKnight		(KnightFlag	| BlackFlag)
#define WhiteBishop		(BishopFlag	| WhiteFlag)
#define BlackBishop		(BishopFlag	| BlackFlag)
#define WhiteRook		(RookFlag	| WhiteFlag)
#define BlackRook		(RookFlag	| BlackFlag)
#define WhiteQueen		(QueenFlag	| WhiteFlag)
#define BlackQueen		(QueenFlag	| BlackFlag)
#define WhiteKing		(KingFlag	| WhiteFlag)
#define BlackKing		(KingFlag	| BlackFlag)

/* Piece Macro Definitions */
#define PIECE_IS_NONE(piece)		(piece & NonePiece)
#define PIECE_IS_PAWN(piece)		(piece & PawnFlag)
#define PIECE_IS_KNIGHT(piece)		(piece & KnightFlag)
#define PIECE_IS_BISHOP(piece)		(piece & BishopFlag)
#define PIECE_IS_ROOK(piece)		(piece & RookFlag)
#define PIECE_IS_QUEEN(piece)		(piece & QueenFlag)
#define PIECE_IS_KING(piece)		(piece & KingFlag)
#define PIECE_IS_WALL(piece)		(piece & Wall)
#define MAKE_PIECE(type, colour)	((1 << (type + 2) + colour))

#endif