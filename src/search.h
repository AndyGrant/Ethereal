#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

/* Function Prototypes */
move_t get_best_move(board_t * board, int time);
void init_search_tree_t(search_tree_t * tree, board_t * board);
int alpha_beta_prune(search_tree_t * tree, principle_variation_t * pv, int depth, int alpha, int beta);
void order_by_value(move_t * moves, int * values, int size);
int quiescence_search(search_tree_t * tree, int alpha, int beta);
void basic_heuristic(search_tree_t * tree, move_t * moves, int size);
void update_killer_heuristic(search_tree_t * tree, move_t move);
#endif