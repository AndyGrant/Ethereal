#ifndef _TRANSPOSITON_H
#define _TRANSPOSITON_H

#include <stdint.h>

#include "types.h"

void initalizeTranspositionTable(TransTable * table, unsigned int keySize);
void destroyTranspositionTable(TransTable * table);
void clearTranspositionTable(TransTable * table);
TransEntry * getTranspositionEntry(TransTable * table, uint64_t hash, int turn);
void storeTranspositionEntry(TransTable * table, uint8_t depth, uint8_t turn, uint8_t type, int16_t value, uint16_t bestMove, uint64_t hash);
void dumpTranspositionTable(TransTable * table);

void initalizePawnTable(PawnTable * ptable);
void destoryPawnTable(PawnTable * ptable);
PawnEntry * getPawnEntry(PawnTable * ptable, uint64_t phash);
void storePawnEntry(PawnTable * ptable, uint64_t phash, int mg, int eg);

extern TransTable Table;
extern PawnTable PTable;

#define PVNODE  (1)
#define CUTNODE (2)
#define ALLNODE (3)

#define EntrySetAge(e,a)    ((e)->data = ((a) << 3) | ((e)->data & 7))
#define EntryDepth(e)       ((e).depth)
#define EntryHash16(e)      ((e).hash16)
#define EntryAge(e)         ((e).data >> 3)
#define EntryType(e)        (((e).data & 6) >> 1)
#define EntryTurn(e)        ((e).data & 1)
#define EntryMove(e)        ((e).bestMove)
#define EntryValue(e)       ((e).value)

#endif 