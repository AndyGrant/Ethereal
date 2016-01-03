#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

//#include "search.h"
#define MaxDepth	(64)
#define MaxMoves	(256)

/* Structure Definitions */

typedef uint32_t move_t;
//	bits	00-07: From Square
//	bits	08-15: To Square 
//	bits	16-23: Capture Type
//	bits	24-27: Move Type
//	bits	28-31: Castle Changes

typedef struct zorbist_t{
	int bitstrings[64][16];
	
} zorbist_t;

typedef struct board_t {
	int positions[256];
	int squares[256];
	
	int piece_locations[2][32];
	int piece_counts[2];
	
	int pawn_locations[2][16];
	int pawn_counts[2];
	
	int castle_rights;
	
	int depth;
	move_t ep_history[MaxDepth];
	
	int hash;
	int hash_entries;
	int hash_history[1024];
	
	int turn;
	
} board_t;

typedef struct principle_variation_t {
	int length;
	move_t line[MaxDepth];
	
} principle_variation_t;


typedef struct search_tree_t {
	int ply;
	int raw_nodes;
	int alpha_beta_nodes;
	int quiescence_nodes;
	
	move_t killer_moves[MaxDepth][5];
	principle_variation_t principle_variation;
	
} search_tree_t;

typedef uint32_t ttentry_t;
// bits		00-15: Value + 32768
// bits		16-16: Turn
// bits		17-23: Depth
// bits		24-25: Bound Type

typedef struct ttable_t {
	ttentry_t * entries;	
	
} ttable_t;

#endif