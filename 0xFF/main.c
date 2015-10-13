#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "types.h"
#include "board.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "search.h"

int main(){
	struct board_t board;
	init_board_t(&board,"rbnqknbrppppppppeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRBNQKNBR11110000");
	
	int i;
	for(i = 0; i < board.pawn_counts[ColourBlack]; i++)
		printf("Index %d Type %d\n",board.pawn_locations[ColourBlack][i],
			board.squares[board.pawn_locations[ColourBlack][i]]);
}