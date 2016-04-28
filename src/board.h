#ifndef _BOARD_H
#define _BOARD_H

#include "types.h"

void init_board(Board * board, char * fen);
void print_board(Board * board);
int perft(Board * board, int depth);

#endif