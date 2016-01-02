#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

/* Function Prototypes */
move_t get_best_move(int time);
void init_search_tree_t();
int alpha_beta_prune(principle_variation_t * pv, int depth, int alpha, int beta);
void order_by_value(move_t * moves, int * values, int size);
int quiescence_search(int alpha, int beta);
void basic_heuristic(move_t * moves, int size);
void update_killer_heuristic(move_t move);
#endif