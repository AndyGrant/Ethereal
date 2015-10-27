#ifndef BOARD_H
#define BOARD_H

#include "types.h"

#define MaxMoves	(256)

/* Board Macro Definitions */

/* Function Protoypes */
void print_board_t(board_t * board);
void encode_board_t(board_t * board, char str[73]);

#endif