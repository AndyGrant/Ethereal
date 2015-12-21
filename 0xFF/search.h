#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

/* Search Definitions */
#define CheckMate	(100000)

/* Piece Value Definitions */
#define PawnValue 	(100)
#define KnightValue	(300)
#define BishopValue	(300)
#define RookValue 	(500)
#define QueenValue 	(900)

/* Function Prototypes */
move_t get_best_move(board_t * board, int time);
void init_search_tree_t(search_tree_t * tree, board_t * board);
int alpha_beta_prune(search_tree_t * tree, int depth, int alpha, int beta);
int evaluate_board(board_t * board);
void order_by_value(move_t * moves, int * values, int size);
int quiescence_search(search_tree_t * tree, int alpha, int beta);
void basic_heuristic(board_t * board, move_t * moves, int size);
#endif