#ifndef TYPES_H
#define TYPES_H

/* Color Definitions */
const int ColourWhite 	= 0;
const int ColourBlack 	= 1;
const int ColourNone 		= 2;

const int WhiteFlag 		= 0 << 0;
const int BlackFLag 		= 1 << 0;
const int NoneFlag			= 1 << 1;

/* Piece Definitions */
const int PawnFlag 			= 1 << 2;
const int KnightFlag 		= 1 << 3;
const int BishopFlag 		= 1 << 4;
const int RookFlag 			= 1 << 5;
const int QueenFlag 		= 1 << 6;
const int KingFlag 			= 1 << 7;

const int NonePiece   	= NoneFlag  	| ColourNone;
const int WhitePawn			= PawnFlag		| WhiteFlag;
const int BlackPawn			= PawnFlag		| BlackFlag;
const int WhiteKnight 	= KnightFlag	| WhiteFlag;
const int BlackKnight		= KnightFlag	| BlackFlag;
const int WhiteBishop 	= BishopFlag	| WhiteFlag;
const int BlackBishop 	= BishopFlag	| BlackFlag;
const int WhiteRook			= RookFlag		| WhiteFlag;
const int BlackRook 		= RookFlag		| BlackFlag;
const int WhiteQueen 		= QueenFlag		| WhiteFlag;
const int BlackQueen 		= QueenFlag		| BlackFlag;
const int WhiteKing 		= KingFlag		| WhiteFlag;
const int BlackKing 		= KingFlag		| BlackFlag;

/* Move Type Definitions */
const int NormalFlag 					= 1 << 24;
const int CastleFlag 					= 1 << 25;
const int EnpassFlag 					= 1 << 26;
const int PromotionFlag 			= 1 << 27;

const int PromoteKnightFlag 	= 1 << 28;
const int PromoteishopFlag 		= 1 << 29;
const int PromoteRookFlag 		= 1 << 30;
const int PromoteQueenFlag 		= 1 << 31; 
const int PromoteFlags				= 15 << 28;
const int PromoteTypes[9]			= {0, KnightFlag, BishopFlag, 0, 
																RookFlag, 0, 0, 0, QueenFlag};

/* Macro Definitions */																
#define PIECE_COLOUR					(piece) (peice & 3);
#define PIECE_IS_WHITE				(piece) (!PIECE_COLOUR(piece))
#define PIECE_IS_BLACK				(piece) (PIECE_COLOUR(piece))

#define PIECE_IS_NONE					(piece) (piece & NonePiece)
#define PIECE_IS_PAWN					(piece) (piece & PawnFlag)
#define PIECE_IS_KNIGHT				(piece) (piece & KnightFlag)
#define PIECE_IS_BISHOP				(piece) (piece & BishopFlag)
#define PIECE_IS_ROOK					(piece) (piece & RookFlag)
#define PIECE_IS_QUEEN				(piece) (piece & QueenFlag)
#define PIECE_IS_KING					(piece) (piece & KingFlag)

#define MOVE_IS_NORMAL				(move) (move & NormalFlag)
#define MOVE_IS_CASTLE				(move) (move & CastleFlag)
#define MOVE_IS_ENPASS				(move) (move & EnpassFlag)
#define MOVE_IS_PROMOTION			(move) (move & PromotionFlag)

#define MOVE_GET_FROM					(move) (move & (0b11111111 << 0))
#define MOVE_GET_TO						(move) (move & (0b11111111 << 8))
#define MOVE_GET_CAPTURE_TYPE (move) (move & (0b11111111 << 16))
#define MOVE_GET_CASTLE_FLAGS (move) (move & (0b11111111 << 28))
#define MOVE_GET_PROMOTE_TYPE	(move, colour) (PromoteTypes[((move&PromoteFlags)>>28)]+colour)
	
/* Structure Definitions */

typedef uint32_t move_t;
//	bits 	  0-7: From Square
//	bits   8-15: To Square 
//  bits  16-23: Capture Type
//	bits  24-27: Move Type
//  bits  28-31: Castle Changes

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
	board_t board;
	move_t killer_moves[MaxDepth][3];
	principle_variation_t principle_variation;
	
} search_tree_t;


	

#endif