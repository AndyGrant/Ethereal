#ifndef BOARD_H
#define BOARD_H

#include "types.h"

#define MaxMoves	(256)

/* Function Protoypes */
void init_board_t(board_t * board, char setup[73]);
void encode_board_t(board_t * board, char str[73]);

#endif