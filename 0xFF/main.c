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
	init_board_t(&board,"rbnqknbrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRBNQKNBR11110000");
	return 0;
	
}