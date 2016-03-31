#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

/* Defined here to avoic circular include(s) */
#define Mate		(100000)
#define MaxDepth	(32)
#define MaxHeight	(64)

typedef struct Board {
	
	// Easy Lookups for piece types
	int squares[64];
	
	// White + Black + Empty
	uint64_t colourBitBoards[3];
	
	// Each Piece + Empty
	uint64_t pieceBitBoards[7];	
	
	// Necessary data members
	int turn;
	int castlerights;
	int fiftymoverule;
	int epsquare;
	
	// Zorbist Key
	uint64_t hash;
	
	// Material + PST values
	int opening;
	int endgame;
	
} Board;

typedef struct Undo {
	
	// Fast undo of captures
	int capture_sq;
	int capture_piece;
	
	// Previous turn and Castle Rights
	int turn;
	int castlerights;
	
	// Previous Material + PST values
	int opening;
	int endgame;
	
	// Previous EP Square
	int epsquare;
	
	// Previous Zorbist key
	uint64_t hash;
	
} Undo;

typedef struct PrincipleVariation {
	// Game Tree
	uint16_t line[MaxHeight];
	
	// Length of Tree
	int length;
	
} PrincipleVariation;

#endif