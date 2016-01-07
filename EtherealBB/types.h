#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

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
	int lastcap;
	
	// Zorbist Key
	int mainhash;
	
	// Pawn Zorbist Key
	int pawnhash;
	
	// Values depending on phase
	int opening;
	int endgame;
} Board;


#endif