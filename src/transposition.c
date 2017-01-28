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
 * @param   megabytes   Table size in megabytes (upperbound)
 */
void initalizeTranspositionTable(TransTable * table, uint64_t megabytes){
    
    // Default table size uses a 16bit key, which
    // results in a table using 16MB of memory.
    uint64_t keySize = 16ull;
    
    // Determine the keysize for the first power of
    // two less than than or equal to megaBytes. We
    // assume here that every bucket is 256 bits
    for (;1ull << (keySize + 5) <= megabytes << 20 ; keySize++);
    keySize -= 1;
    
    // Setup Table's data members
    table->buckets = calloc(1 << keySize, sizeof(TransBucket));
    table->numBuckets = 1 << keySize;
    table->keySize = keySize;
    table->generation = 0;
    table->used = 0;
}

/**
 * Free the memory allocated for the transposition table
 *
 * @param   table   Location of table to free
 */
void destroyTranspositionTable(TransTable * table){
    
    free(table->buckets);
}

/**
 * Fetch a matching entry from the table. A
 * matching entry has the same hash signature
 *
 * @param   table   TransTable pointer to table location
 * @param   hash    64-bit zorbist key to be matched
 *
 * @return          Found entry or NULL
 */
TransEntry * getTranspositionEntry(TransTable * table, uint64_t hash){
    
    TransBucket * bucket = &(table->buckets[hash & (table->numBuckets - 1)]);
    int i; uint16_t hash16 = hash >> 48;
    
    // Search for a matching entry. Update the generation if found.
    for (i = 0; i < BUCKET_SIZE; i++){
        if (EntryHash16(bucket->entries[i]) == hash16){
            EntrySetAge(&(bucket->entries[i]), table->generation);
            return &(bucket->entries[i]);
        }
    }
    
    // No entry found
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
 * @param   hash        64bit zorbist key corresponding to the board
*/
void storeTranspositionEntry(TransTable * table, int depth, int type, int value, int bestMove, uint64_t hash){
    
    // Validate Parameters
    assert(depth < MaxDepth && depth >= 0);
    assert(type == PVNODE || type == CUTNODE || type == ALLNODE);
    assert(value <= MATE && value >= -MATE);
    
    TransBucket * bucket = &(table->buckets[hash & (table->numBuckets - 1)]);
    TransEntry * oldCandidate = NULL;
    TransEntry * lowDraftCandidate = NULL;
    TransEntry * toReplace = NULL;
    
    int i; uint16_t hash16 = hash >> 48;
    
    for (i = 0; i < BUCKET_SIZE; i++){
        
        // Found an unused entry
        if (EntryType(bucket->entries[i]) == 0){
            table->used += 1;
            toReplace = &(bucket->entries[i]);
            goto Replace;
        }
        
        // Found an entry with the same hash key
        if (EntryHash16(bucket->entries[i]) == hash16){
            toReplace = &(bucket->entries[i]);
            goto Replace;
        }
        
        // Search for the lowest draft of an old entry
        if (EntryAge(bucket->entries[i]) != table->generation){
            if (oldCandidate == NULL
                || EntryDepth(*oldCandidate) >= EntryDepth(bucket->entries[i])){
                    
                oldCandidate = &(bucket->entries[i]);
            }
        }
        
        // Search for the lowest draft if no old entry has been found yet
        if (oldCandidate == NULL){
            if (lowDraftCandidate == NULL 
                || EntryDepth(*lowDraftCandidate) >= EntryDepth(bucket->entries[i])){
                    
                lowDraftCandidate = &(bucket->entries[i]);
            }
        }
    }
    
    // If no old candidate, use the lowest draft
    toReplace = oldCandidate != NULL ? oldCandidate : lowDraftCandidate;
    
    Replace:
        toReplace->depth = depth;
        toReplace->data = (table->generation << 2) | (type);
        toReplace->value = value;
        toReplace->bestMove = bestMove;
        toReplace->hash16 = hash16;
}

/**
 * Update the age / generation of the transposition table
 *
 * @param   table   TransTable pointer to table location
 */
void updateTranspositionTable(TransTable * table){
    
    table->generation = (table->generation + 1) % 64;
}

/**
 * Zero out all entries in the transposition table
 *
 * @param   table   TransTable pointer to table location
 */
void clearTranspositionTable(TransTable * table){
    
    unsigned int i; int j;
    TransEntry * entry;
    
    table->generation = 0;
    table->used = 0;
    
    for (i = 0; i < table->numBuckets; i++){
        for (j = 0; j < BUCKET_SIZE; j++){
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
 * Allocate memory for the pawn structure hash table
 *
 * @param   ptable  Location to allocate table
 */
void initalizePawnTable(PawnTable * ptable){
    
    ptable->entries = calloc(0x10000, sizeof(PawnEntry));
}

/**
 * Delete memory used by the pawn structure hash table
 *
 * @param   ptable  Location of table to free
 */
void destoryPawnTable(PawnTable * ptable){
    
    free(ptable->entries);
}

/**
 * Fetch a matching entry from the pawn table.
 * Matching entries share the same pawn key.
 *
 * @param   ptable  Location of pawn table
 * @param   phash   Pawn hash to match
 */
PawnEntry * getPawnEntry(PawnTable * ptable, uint64_t phash){
    
    PawnEntry * pentry = &(ptable->entries[phash >> 48]);
    
    // Check for a matching hash signature
    if (pentry->phash == phash)
        return pentry;
        
    // No entry found
    return NULL;
}

/**
 * Store a pawn entry into the table
 *
 * @param   ptable  Location of pawn table
 * @param   phash   Pawn hash of the current board
 * @param   passed  Bitboard of the passed pawns
 * @param   mg      Evaluation for the mid game
 * @param   eg      Evaluation for the end game
 */
void storePawnEntry(PawnTable * ptable, uint64_t phash, uint64_t passed, int mg, int eg){
    
    PawnEntry * pentry = &(ptable->entries[phash >> 48]);
    pentry->phash = phash;
    pentry->passed = passed;
    pentry->mg = mg;
    pentry->eg = eg;
}