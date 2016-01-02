#include <stdlib.h>
#include <time.h>

#include "types.h"
#include "zorbist.h"
#include "piece.h"
#include "util.h"

extern zorbist_t zorbist;
extern board_t board;

void init_zorbist_t(){
	srand(time(NULL));
	int i, j;
	for(i = 0; i < 64; i++)
		for(j = 0; j < 12; j++)
			zorbist.bitstrings[i][j] = gen_bitstring();
}

int gen_bitstring(){
	return (rand() | (rand() << 8) | (rand() << 16) | (rand() << 24));
}

void init_zorbist_hash(){
	board.hash = 0;
	
	int i, j, p;
	for(i = 0; i < 64; i++){
		p = board.squares[CONVERT_64_TO_256(i)];
		if (!IS_EMPTY(p))
			board.hash ^= zorbist.bitstrings[i][CONVERT_PIECE256_TO_PIECE12(p)];
	}
}