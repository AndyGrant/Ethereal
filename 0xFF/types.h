#ifndef TYPES_H
#define TYPES_H

/* Color Definitions */
#define ColourWhite		(0)
#define ColourBlack		(1)
#define ColourNone		(2)

#define WhiteFlag		(0 << 0)
#define BlackFlag		(1 << 0)
#define NoneFlag		(1 << 1)

/* Piece Definitions */
#define PawnFlag		(1 << 2)
#define KnightFlag		(1 << 3)
#define BishopFlag		(1 << 4)
#define RookFlag		(1 << 5)
#define QueenFlag		(1 << 6)
#define KingFlag		(1 << 7)

#define Wall		((1 << 24) + 30)
#define NonePiece	(NoneFlag	| ColourNone)
#define WhitePawn	(PawnFlag	| WhiteFlag)
#define BlackPawn	(PawnFlag	| BlackFlag)
#define WhiteKnight	(KnightFlag	| WhiteFlag)
#define BlackKnight	(KnightFlag	| BlackFlag)
#define WhiteBishop	(BishopFlag	| WhiteFlag)
#define BlackBishop	(BishopFlag	| BlackFlag)
#define WhiteRook	(RookFlag	| WhiteFlag)
#define BlackRook	(RookFlag	| BlackFlag)
#define WhiteQueen	(QueenFlag	| WhiteFlag)
#define BlackQueen	(QueenFlag	| BlackFlag)
#define WhiteKing	(KingFlag	| WhiteFlag)
#define BlackKing	(KingFlag	| BlackFlag)

/* Move Type Definitions */
#define NormalFlag			(1 << 24)
#define CastleFlag			(1 << 25)
#define EnpassFlag			(1 << 26)
#define PromotionFlag		(1 << 27)
#define PromoteKnightFlag	(1 << 28)
#define PromoteishopFlag	(1 << 29)
#define PromoteRookFlag		(1 << 30)
#define PromoteQueenFlag	(1 << 31) 
#define PromoteFlags		(15<< 28)

static PromoteTypes[9] = {0, KnightFlag, BishopFlag, 0,RookFlag, 0, 0, 0, QueenFlag};

/* Search Definitions */
#define MaxDepth	(128)
#define MaxMoves	(256)

/* Colour Macro Definitions */
#define PIECE_COLOUR(piece)		(peice & 3)
#define PIECE_IS_WHITE(piece)	(!PIECE_COLOUR(piece))
#define PIECE_IS_BLACK(piece)	(PIECE_COLOUR(piece))

/* Piece Macro Definitions */
#define PIECE_IS_NONE(piece)	(piece & NonePiece)
#define PIECE_IS_PAWN(piece)	(piece & PawnFlag)
#define PIECE_IS_KNIGHT(piece)	(piece & KnightFlag)
#define PIECE_IS_BISHOP(piece)	(piece & BishopFlag)
#define PIECE_IS_ROOK(piece)	(piece & RookFlag)
#define PIECE_IS_QUEEN(piece)	(piece & QueenFlag)
#define PIECE_IS_KING(piece)	(piece & KingFlag)

#define MAKE_PIECE(type, colour)	((1 << (type + 2) + colour))

/* Move Type Macro Definitions */
#define MOVE_IS_NORMAL(move)		(move & NormalFlag)
#define MOVE_IS_CASTLE(move)		(move & CastleFlag)
#define MOVE_IS_ENPASS(move)		(move & EnpassFlag)
#define MOVE_IS_PROMOTION(move)		(move & PromotionFlag)

/* Move Decode Macro Definitions */
#define MOVE_GET_FROM(m)			((m & (0b11111111 <<  0)) >>  0)
#define MOVE_GET_TO(m)				((m & (0b11111111 <<  8)) >>  8)
#define MOVE_GET_CAPTURE_TYPE(m)	((m& (0b11111111 << 16)) >> 16)
#define MOVE_GET_CASTLE_FLAGS(m)	((m& (0b00001111 << 28)) >> 25)
#define MOVE_GET_PROMOTE_TYPE(m,c)	(PromoteTypes[((m&PromoteFlags)>>28)]+c)

/* Structure Definitions */

typedef uint32_t move_t;
//	bits	00-07: From Square
//	bits	08-15: To Square 
//	bits	16-23: Capture Type
//	bits	24-27: Move Type
//	bits	28-31: Castle Changes

typedef struct transposition_t {
	
	
} transposition_t;

typedef struct board_t {
	int positions[0xFF];
	int squares[0xFF];
	
	int piece_locations[2][32];
	int piece_counts[2];
	
	int pawn_locations[2][16];
	int pawn_counts[2];
	
	int castle_rights;
	int ep_square;
	int turn;
	
} board_t;


typedef struct principle_variation_t {
	int plys;
	move_t line[MaxDepth];
	
} principle_variation_t;


typedef struct search_tree_t {
	int ply;
	int nodes_searched;
	
	board_t board;
	
	move_t killer_moves[MaxDepth][3];
	principle_variation_t principle_variation;
	
} search_tree_t;

#endif