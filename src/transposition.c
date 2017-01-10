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

#include "move.h"
#include "types.h"
#include "transposition.h"

/**
 * Allocate memory for the transposition table and
 * set the data members to their inital states
 *
 * @param   table       Location to allocate the table
 * @param   megabytes   Table size in megabytes
 */
void initalizeTranspositionTable(TransTable * table, uint64_t megabytes){
    
    uint64_t keySize = 16ull;
    
    // Determine the keysize for the first power of
    // two less than than or equal to megaBytes
    for (;1ull << (keySize + 5) <= megabytes << 20 ; keySize++);
    keySize -= 1;
    
    // Setup Table's data members
    table->buckets = calloc(1 << keySize, sizeof(TransBucket));
    table->maxSize = 1 << keySize;
    table->keySize = keySize;
    table->generation = 0;
    table->used = 0;
}

void updateTranspositionTable(TransTable * table){
    
    table->generation = (table->generation + 1) % 64;
}

void destroyTranspositionTable(TransTable * table){
    
    free(table->buckets);
}

void clearTranspositionTable(TransTable * table){
    
    unsigned int i;
    int j;
    TransEntry * entry;
    
    table->generation = 0;
    table->used = 0;
    
    for (i = 0; i < table->maxSize; i++){
        for (j = 0; j < 4; j++){
            entry = &(table->buckets[i].entries[j]);
            
            entry->depth = 0;
            entry->data = 0;
            entry->value = 0;
            entry->bestMove = 0;
            entry->hash16 = 0;
        }
    }
}

/**
 * Fetch a matching entry from the table. A
 * matching entry has the same hash signature
 *
 * @param   table   TransTable pointer to table location
 * @param   hash    64-bit hash-key to be matched
 *
 * @return          Found entry or NULL
 */
TransEntry * getTranspositionEntry(TransTable * table, uint64_t hash){
    
    TransBucket * bucket = &(table->buckets[hash & (table->maxSize - 1)]);
    int i; uint16_t hash16 = hash >> 48;
    
    // SEARCH FOR MATCHING ENTRY, UPDATE GENERATION IF FOUND
    for (i = 0; i < 4; i++){
        if (EntryHash16(bucket->entries[i]) == hash16){
            EntrySetAge(&(bucket->entries[i]), table->generation);
            return &(bucket->entries[i]);
        }
    }
    
    // NO MATCHING ENTRY FOUND
    return NULL;
}

/**
 * Create and store a new entry in the Transposition Table. If
 * the bucket found with the lower N(keySize) bits of the hash
 * has an empty location, store it there. Otherwise replace the 
 * lowest depth entry that came from a previous search (has a 
 * different age/generation). Finally, if there are no old entries,
 * replace the lowest depth entry found in the bucket.
 *
 * @param   table       TransTable pointer to table location
 * @param   depth       Depth from the current search
 * @param   type        Entry type based on alpha-beta window
 * @param   value       Value to be returned by the search
 * @param   bestMove    Best move found during the search
 * @param   hash        64-bit hash-key corresponding to the board
*/
void storeTranspositionEntry(TransTable * table, uint8_t depth, uint8_t type, int16_t value, uint16_t bestMove, uint64_t hash){
    
    TransBucket * bucket = &(table->buckets[hash & (table->maxSize - 1)]);
    
    TransEntry * oldCandidate = NULL;
    TransEntry * lowDraftCandidate = NULL;
    TransEntry * toReplace = NULL;
    
    int i; uint16_t hash16 = hash >> 48;
    
    // VALIDATE PARAMETERS
    assert(depth < MaxDepth);
    assert(type >= 1 && type <= 3);
    assert(value <= 32766 && value >= -32766);
    
    for (i = 0; i < 4; i++){
        
        // FOUND AN UNUSED ENTRY
        if (EntryType(bucket->entries[i]) == 0){
            table->used += 1;
            toReplace = &(bucket->entries[i]);
            goto Replace;
        }
        
        // FOUND AN ENTRY WITH THE SAME HASH
        if (EntryHash16(bucket->entries[i]) == hash16){
            toReplace = &(bucket->entries[i]);
            goto Replace;
        }
        
        // SEARCH FOR THE LOWEST DRAFT OF AN OLD ENTRY
        if (EntryAge(bucket->entries[i]) != table->generation){
            if (oldCandidate == NULL
                || EntryDepth(*oldCandidate) >= EntryDepth(bucket->entries[i])){
                    
                oldCandidate = &(bucket->entries[i]);
            }
        }
        
        // SEARCH FOR LOWEST DRAFT IF NO OLD ENTRY FOUND YET
        if (oldCandidate == NULL){
            if (lowDraftCandidate == NULL 
                || EntryDepth(*lowDraftCandidate) >= EntryDepth(bucket->entries[i])){
                    
                lowDraftCandidate = &(bucket->entries[i]);
            }
        }
    }
    
    toReplace = oldCandidate != NULL ? oldCandidate : lowDraftCandidate;
    
    Replace:
        toReplace->depth = depth;
        toReplace->data = (table->generation << 2) | (type);
        toReplace->value = value;
        toReplace->bestMove = bestMove;
        toReplace->hash16 = hash16;
}

void initalizePawnTable(PawnTable * ptable){
    
    ptable->entries = calloc(0x10000, sizeof(PawnEntry));
}

void destoryPawnTable(PawnTable * ptable){
    
    free(ptable->entries);
}

PawnEntry * getPawnEntry(PawnTable * ptable, uint64_t phash){
    
    PawnEntry * pentry = &(ptable->entries[phash >> 48]);
    
    if (pentry->phash == phash)
        return pentry;
        
    return NULL;
}

void storePawnEntry(PawnTable * ptable, uint64_t phash, uint64_t passed, int mg, int eg){
    
    PawnEntry * pentry = &(ptable->entries[phash >> 48]);
    
    pentry->phash = phash;
    pentry->passed = passed;
    pentry->mg = mg;
    pentry->eg = eg;
}