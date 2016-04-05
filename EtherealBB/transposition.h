#ifndef _TRANSPOSITON_H
#define _TRANSPOSITON_H

#include <stdint.h>

/* Node Types */
#define PVNODE	(1)
#define CUTNODE	(2)
#define ALLNODE	(3)

/* Prototypes */
void init_transposition_table(TranspositionTable * table, int key_size);
TranspositionEntry * get_transposition_entry(TranspositionTable * table, uint64_t hash);
void store_transposition_entry(TranspositionTable * table, int8_t depth, int8_t turn, int8_t type, int value, uint16_t best_move, uint64_t hash);
void dump_transposition_table(TranspositionTable * table);
void extract_pv_from_transposition_table(TranspositionTable * table, Board * board, int depth, uint16_t * dest);
#endif 
