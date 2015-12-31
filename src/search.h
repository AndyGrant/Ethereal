#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

/* Search Definitions */
#define CheckMate	(100000)

/* Piece Value Definitions */
#define PawnValue 	(300)
#define KnightValue	(3 * PawnValue)
#define BishopValue	(3 * PawnValue)
#define RookValue 	(5 * PawnValue)
#define QueenValue 	(9 * PawnValue)

#define PawnStackedValue				(-80)
#define IsolatedPawnValue				( -9)
#define DiagonallyConnectedPawnValue	(  5)
#define HorizontallyConnectedPawnValue  (  2)	

/* Function Prototypes */
move_t get_best_move(board_t * board, int time);
void init_search_tree_t(search_tree_t * tree, board_t * board);
int alpha_beta_prune(search_tree_t * tree, principle_variation_t * pv, int depth, int alpha, int beta);
int evaluate_board(board_t * board);
void order_by_value(move_t * moves, int * values, int size);
int quiescence_search(search_tree_t * tree, int alpha, int beta);
void basic_heuristic(search_tree_t * tree, move_t * moves, int size);
void update_killer_heuristic(search_tree_t * tree, move_t move);
#endif