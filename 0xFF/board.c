#include <stdio.h>
#include <stdlib.h>

#include "types.h" 

char BaseBoard[64] = "rbnqknbrppppppppeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRBNQKNBR"

void init_board_t(board_t board, char setup[137]){
	int i, s;
	
	
	/* Define Wall, Center will be overwritten */
	for(s = 0; s < 256; s++)
		board->squares[s] = WALL;
	
	/* Define Pieces in Squares */
	for(s = 0, i = 0; s < 64; s++){
		int type = setup[i++]-'0';
		int colour = setup[i++] < 
		board->squares[CONVERT_64_TO_256(s)] = MAKE_PIECE(type,colour)
	}
	
	/* Define Castle Rights */
	board->castle_rights = 0;
	for(s = 0; s < 4; s++)
		board->castle_rights |= (setup[i++]-'0') << s;
	
	
	
}