/*
  Ethereal is a UCI chess playing engine authored by Andrew Grant.
  <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>
  
  Ethereal is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  Ethereal is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _TRANSPOSITON_H
#define _TRANSPOSITON_H

#include <stdint.h>

#include "types.h"

typedef struct TransEntry {
    int16_t value;
    uint8_t depth;
    uint8_t age: 6, type: 2;
    uint16_t bestMove, hash16;
} TransEntry;

typedef struct TransBucket {
    TransEntry entries[BUCKET_SIZE];
} TransBucket;

typedef struct TransTable {
    TransBucket * buckets;
    uint64_t numBuckets;
    uint64_t keySize;
    uint8_t generation;
} TransTable;

typedef struct PawnKingEntry {
    uint64_t pkhash;
    uint64_t passed;
    int eval;
} PawnKingEntry;

typedef struct PawnKingTable {
    PawnKingEntry entries[0x10000];
} PawnKingTable;

void initializeTranspositionTable(TransTable* table, uint64_t megabytes);
void destroyTranspositionTable(TransTable* table);
void updateTranspositionTable(TransTable* table);
void clearTranspositionTable(TransTable* table);
int estimateHashfull(TransTable* table);

int getTranspositionEntry(TransTable* table, uint64_t hash, TransEntry* ttEntry);
void storeTranspositionEntry(TransTable* table, int depth, int type, int value, int bestMove, uint64_t hash);

PawnKingEntry * getPawnKingEntry(PawnKingTable* pktable, uint64_t pkhash);
void storePawnKingEntry(PawnKingTable* pktable, uint64_t pkhash, uint64_t passed, int eval);

#endif 
