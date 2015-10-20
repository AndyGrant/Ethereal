#ifndef BOARD_H
#define BOARD_H

#include "types.h"

#define MaxMoves	(256)

/* Board Macro Definitions */

/* Function Protoypes */
void init_board_t(board_t * board, char setup[73]);
void print_board_t(board_t * board);

#endif