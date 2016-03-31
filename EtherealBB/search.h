#ifndef _SEARCH_H
#define _SEARCH_H

#include <stdint.h>

#include "types.h"

/* Prototypes */
uint16_t get_best_move(Board * board, int seconds);
int alpha_beta_prune(Board * board, int alpha, int beta, int depth, int height, PrincipleVariation * PV);


#endif