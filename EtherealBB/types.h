#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

#define Mate		(100000)
#define MaxDepth	(64)
#define MaxHeight	(128)

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

typedef struct TranspositionEntry {
	int8_t depth;
	int8_t turn;
	int8_t type;
	int value;
	uint16_t best_move;
	uint64_t hash;	
	
} TranspositionEntry;

typedef struct TranspositionTable {
	TranspositionEntry * entries;
	int max_size;
	int key_size;
	int num_entries;
	int hits;
	int misses;
	int key_collisions;
	
} TranspositionTable;

#endif