#ifndef _SEARCH_H
#define _SEARCH_H

#include <stdint.h>

#include "types.h"


#define MaxKillers	(3)

/* Prototypes */
uint16_t get_best_move(Board * board, int seconds);
int alpha_beta_prune(Board * board, int alpha, int beta, int depth, int height);
void sort_moves(uint16_t * moves, int size, int depth, int height, TranspositionEntry * entry);

#endif