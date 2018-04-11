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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "move.h"
#include "types.h"
#include "transposition.h"

TransTable Table;

void initializeTranspositionTable(TransTable* table, uint64_t megabytes){
    
    // Minimum table size is 1MB. This maps to a key of size 15.
    // We start at 16, because the loop to adjust the memory
    // size to a power of two ends with a decrement of keySize
    uint64_t keySize = 16ull;
    
    // Every bucket must be 256 bits for the following scaling
    assert(sizeof(TransBucket) == 32);

    // Scale down the table to the closest power of 2, at or below megabytes
    for (;1ull << (keySize + 5) <= megabytes << 20 ; keySize++);
    keySize -= 1;
    
    // Setup Table's data members
    table->buckets      = malloc((1ull << keySize) * sizeof(TransBucket));
    table->numBuckets   = 1ull << keySize;
    table->keySize      = keySize;

    clearTranspositionTable(table);
}

void destroyTranspositionTable(TransTable* table){
    free(table->buckets);
}

void updateTranspositionTable(TransTable* table){
    table->generation = (table->generation + 1) % 64;
}

void clearTranspositionTable(TransTable* table){

    table->generation = 0u;

    memset(table->buckets, 0, sizeof(TransBucket) * table->numBuckets);
    
}

int estimateHashfull(TransTable* table){
    
    int i, used = 0;
    
    for (i = 0; i < 250 && i < (int64_t)table->numBuckets; i++)
        used += (table->buckets[i].entries[0].type != 0)
             +  (table->buckets[i].entries[1].type != 0)
             +  (table->buckets[i].entries[2].type != 0)
             +  (table->buckets[i].entries[3].type != 0);
             
    return 1000 * used / (i * 4);
}

int getTranspositionEntry(TransTable* table, uint64_t hash, TransEntry* ttEntry){
    
    TransBucket* bucket = &(table->buckets[hash & (table->numBuckets - 1)]);
    int i; uint16_t hash16 = hash >> 48;
    
    #ifdef TEXEL
    return NULL;
    #endif
    
    // Search for a matching entry. Update the generation if found.
    for (i = 0; i < BUCKET_SIZE; i++){
        if (bucket->entries[i].hash16 == hash16){
            bucket->entries[i].age = table->generation;
            memcpy(ttEntry, &bucket->entries[i], sizeof(TransEntry));
            return 1;
        }
    }
    
    return 0;
}

void storeTranspositionEntry(TransTable* table, int depth, int type, int value, int bestMove, uint64_t hash){
    
    // Validate Parameters
    assert(depth < MAX_DEPTH && depth >= 0);
    assert(type == PVNODE || type == CUTNODE || type == ALLNODE);
    assert(value <= MATE && value >= -MATE);
    
    TransBucket* bucket = &(table->buckets[hash & (table->numBuckets - 1)]);
    TransEntry* entries = bucket->entries;
    TransEntry* oldOption = NULL;
    TransEntry* lowDraftOption = NULL;
    TransEntry* toReplace = NULL;
    
    int i; uint16_t hash16 = hash >> 48;
    
    for (i = 0; i < BUCKET_SIZE; i++){
        
        // Found an unused entry
        if (entries[i].type == 0){
            toReplace = &(entries[i]);
            goto Replace;
        }
        
        // Found an entry with the same hash key
        if (entries[i].hash16 == hash16){
            toReplace = &(entries[i]);
            goto Replace;
        }
        
        // Search for the lowest draft of an old entry
        if (entries[i].age != table->generation)
            if (oldOption == NULL || oldOption->depth >= entries[i].depth)
                oldOption = &(entries[i]);
        
        // Search for the lowest draft if no old entry has been found yet
        if (oldOption == NULL)
            if (lowDraftOption == NULL || lowDraftOption->depth >= entries[i].depth)
                lowDraftOption = &(entries[i]);
    }
    
    // If no old option, use the lowest draft
    toReplace = oldOption != NULL ? oldOption : lowDraftOption;
    
    Replace:
        toReplace->value    = value;
        toReplace->depth    = depth;
        toReplace->age      = table->generation;
        toReplace->type     = type;
        toReplace->bestMove = bestMove;
        toReplace->hash16   = hash16;
}

PawnKingEntry * getPawnKingEntry(PawnKingTable* pktable, uint64_t pkhash){
    PawnKingEntry* pkentry = &(pktable->entries[pkhash >> 48]);
    return pkentry->pkhash == pkhash ? pkentry : NULL;
}

void storePawnKingEntry(PawnKingTable* pktable, uint64_t pkhash, uint64_t passed, int eval){
    PawnKingEntry* pkentry = &(pktable->entries[pkhash >> 48]);
    pkentry->pkhash = pkhash;
    pkentry->passed = passed;
    pkentry->eval   = eval;
}
