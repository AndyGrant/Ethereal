#ifndef _BOARD_H
#define _BOARD_H

#include "types.h"

void initalizeBoard(Board * board, char * fen);
void printBoard(Board * board);
int perft(Board * board, int depth);

#endif