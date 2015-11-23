#ifndef PIECE_H
#define PIECE_H

#include "colour.h"

/* Piece Flag Definitions */
#define PawnFlag		(1 << 1)
#define KnightFlag		(1 << 2)
#define BishopFlag		(1 << 3)
#define RookFlag		(1 << 4)
#define QueenFlag		(1 << 5)
#define KingFlag		(1 << 6)

/* Piece Definitions */
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

/* Non Piece Flag Definitions */
#define Empty			(1 << 7)
#define Wall			(1 << 8)
#define NonPiece		(Empty | Wall)

/* Non Piece Macro Definitions */
#define IS_PIECE(square)			(square & (~NonPiece))
#define IS_NOT_PIECE(square)		(piece & NonPiece)
#define IS_EMPTY(square)			((square) & Empty)
#define IS_WALL(square)				((square) & Wall)

/* Piece Macro Definitions */
#define PIECE_IS_PAWN(piece)		(piece & PawnFlag)
#define PIECE_IS_KNIGHT(piece)		(piece & KnightFlag)
#define PIECE_IS_BISHOP(piece)		(piece & BishopFlag)
#define PIECE_IS_ROOK(piece)		(piece & RookFlag)
#define PIECE_IS_QUEEN(piece)		(piece & QueenFlag)
#define PIECE_IS_KING(piece)		(piece & KingFlag)
#define PIECE_TYPE(piece)			(piece & ~BlackFlag)

#define IS_EMPTY_OR_ENEMY(s,t) 		((s) != Wall && ((s) == Empty || ((s)%2) != (t)))

#define MAKE_PIECE(type, colour)	((1 << (type + 1)) + colour)

#endif