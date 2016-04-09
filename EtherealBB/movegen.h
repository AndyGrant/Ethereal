#ifndef _MOVEGEN_H
#define _MOVEGEN_H

#include <stdint.h> // For uint16_t
#include "types.h" // For Board

void gen_all_moves(Board * board, uint16_t * moves, int * size);
void gen_all_non_quiet(Board * board, uint16_t * moves, int * size);

int is_not_in_check(Board * board, int turn);
int square_is_attacked(Board * board, int turn, int sq);

#endif