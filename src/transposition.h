#ifndef _TRANSPOSITON_H
#define _TRANSPOSITON_H

#include <stdint.h>

#define PVNODE  (1)
#define CUTNODE (2)
#define ALLNODE (3)

void initalizeTranspositionTable(TranspositionTable * table, int keySize);

TranspositionEntry * getTranspositionEntry(TranspositionTable * table, uint64_t hash);

void storeTranspositionEntry(TranspositionTable * table, int8_t depth, int8_t turn, int8_t type, int value, uint16_t bestMove, uint64_t hash);

void dumpTranspositionTable(TranspositionTable * table);

void updateTranspositionTable(TranspositionTable * table);

#endif 
