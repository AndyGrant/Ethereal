#ifndef BOARD_H
#define BOARD_H

#include "types.h"

#define MaxMoves	(256)

/* Board Macro Definitions */
#define CONVERT_64_TO_256(n)	((16*(4+(n/8)))+(4+(n%8)))

void init_board_t(board_t * board, char setup[73]);

#endif