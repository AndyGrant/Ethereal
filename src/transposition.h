#ifndef _TRANSPOSITON_H
#define _TRANSPOSITON_H

#include <stdint.h>

#define PVNODE  (1)
#define CUTNODE (2)
#define ALLNODE (3)

#define EntryTurn(e) (e->info  & 0b1)
#define EntryType(e) ((e->info & 0b110) >> 1)
#define EntryAge(e)  (e->info >> 3)

void initalizeTranspositionTable(TransTable * table, int keySize);

TransEntry * getTranspositionEntry(TransTable * table, uint64_t hash);

void storeTranspositionEntry(TransTable * table, int8_t depth, int8_t turn, int8_t type, int value, uint16_t bestMove, uint64_t hash);

void dumpTranspositionTable(TransTable * table);

#endif 