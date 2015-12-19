#ifndef BOARD_H
#define BOARD_H

#include "types.h"

#define MaxMoves	(256)

/* Function Protoypes */
void print_board_t(board_t * board);
void print_board_t_locations(board_t * board);
void encode_board_t(board_t * board, char str[73]);
void validate_board_t(board_t * board);

#endif