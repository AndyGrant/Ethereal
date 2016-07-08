#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "move.h"
#include "types.h"
#include "transposition.h"

/**
 * Perform one of two operations:
 *  1)  Allocate memory for the transposition table and
 *      set the data members to their inital states
 *  2)  Update the generation of the table because it has
 *      already been created and can be used between searches
 *
 * @param   table   TransTable pointer to allocation destination
 * @param   keySize Number of bits per key, also the size of the table
 */
void initalizeTranspositionTable(TransTable * table, unsigned int keySize){
    
    // TABLE HAS ALREADY BEEN INITALIZED
    if (table->keySize == keySize){
        table->generation = (table->generation + 1) % 32;
        return;
    }
    
    // SET TABLE'S DATA MEMEBERS
    table->buckets = calloc(1 << keySize,sizeof(TransBucket));
    table->maxSize = 1 << keySize;
    table->keySize = keySize;
    table->generation = 0;
    table->used = 0;
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
 * Fetch a matching entry from the table. A matching entry has
 * the same hash signature and the same turn.
 *
 * @param   table   TransTable pointer to table location
 * @param   hash    64-bit hash-key to be matched
 * @param   turn    Game turn to be matched
 *
 * @return          Found entry or NULL
 */
TransEntry * getTranspositionEntry(TransTable * table, uint64_t hash, int turn){
    
    TransBucket * bucket = &(table->buckets[hash & (table->maxSize - 1)]);
    int i; uint16_t hash16 = hash >> 48;
    
    // SEARCH FOR MATCHING ENTRY, UPDATE GENERATION IF FOUND
    for (i = 0; i < 4; i++){
        if (EntryHash16(bucket->entries[i]) == hash16
            && EntryTurn(bucket->entries[i]) == turn){
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
 * @param   turn        Turn from current search
 * @param   type        Entry type based on alpha-beta window
 * @param   value       Value to be returned by the search
 * @param   bestMove    Best move found during the search
 * @param   hash        64-bit hash-key corresponding to the board
*/
void storeTranspositionEntry(TransTable * table, uint8_t depth, uint8_t turn, uint8_t type, int16_t value, uint16_t bestMove, uint64_t hash){
    
    TransBucket * bucket = &(table->buckets[hash & (table->maxSize - 1)]);
    
    TransEntry * oldCandidate = NULL;
    TransEntry * lowDraftCandidate = NULL;
    TransEntry * toReplace = NULL;
    
    int i; uint16_t hash16 = hash >> 48;
    
    // VALIDATE PARAMETERS
    assert(depth < MaxDepth);
    assert(turn == 0 || turn == 1);
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
        if (EntryHash16(bucket->entries[i]) == hash16
            && EntryTurn(bucket->entries[i]) == turn){
            
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
        toReplace->data = (table->generation << 3) | (type << 1) | (turn);
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

void storePawnEntry(PawnTable * ptable, uint64_t phash, int mg, int eg){
    
    PawnEntry * pentry = &(ptable->entries[phash >> 48]);
    
    pentry->phash = phash;
    pentry->mg = mg;
    pentry->eg = eg;
}

/**
 * Print out information about the Transposition Table. Print
 * the percentage of the table that has been used, as well as
 * the number of each type of entry {ALLNODE, CUTNODE, PVNODE}
 * at each depth that can be found in the table
 *
 * @param   table   TransTable pointer to table location
 */
void dumpTranspositionTable(TransTable * table){
    
    printf("USED %d of %d\n",table->used,4*(table->maxSize));
    
    TransEntry entry;
    int j, depth, type, data[MaxHeight][4];
    unsigned int i;
    
    for (i = 0; i < MaxHeight; i++)
        for (j = 0; j < 4; j++)
            data[i][j] = 0;
        
    for (i = 0; i < table->maxSize; i++){
        for (j = 0; j < 4; j++){
            entry = table->buckets[i].entries[j];
            depth = EntryDepth(entry);
            type = EntryType(entry);
            data[depth][type]++;
        }
    }
    
    printf("\n\n");
    printf("<-----------TRANSPOSITION TABLE---------->\n");
    printf("| Depth  |   PV    |    CUT    |   ALL   |\n");
    for (i = 0; i < MaxHeight; i++)
        if (data[i][1] || data[i][2] || data[i][3])
            printf("|%8d|%9d|%11d|%9d|\n",i,data[i][1],data[i][2],data[i][3]);
    printf("\n\n");
}