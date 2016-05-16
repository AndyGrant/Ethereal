#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "move.h"
#include "types.h"
#include "transposition.h"

void initalizeTranspositionTable(TransTable * table, int keySize){
    
    // TABLE HAS ALREADY BEEN INITALIZED
    if (table->keySize == keySize){
        table->generation = (table->generation + 1) % 32;
        return;
    }
    
    printf("bucket sizw = %d\n",sizeof(TransBucket));
    
    // SET TABLE'S DATA MEMEBERS
    table->buckets = calloc(1 << keySize,sizeof(TransBucket));
    table->maxSize = 1 << keySize;
    table->keySize = keySize;
    table->generation = 0;
    table->used = 0;
}

TransEntry * getTranspositionEntry(TransTable * table, uint64_t hash, int turn){
    
    TransBucket * bucket = &(table->buckets[hash % table->maxSize]);
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

void storeTranspositionEntry(TransTable * table, uint8_t depth, uint8_t turn, uint8_t type, int16_t value, uint16_t bestMove, uint64_t hash){
    
    TransBucket * bucket = &(table->buckets[hash % table->maxSize]);
    
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
            
            // UPDATE MATCHING ENTRY'S GENERATION
            EntrySetAge(&(bucket->entries[i]), table->generation);
            
            // REPLACE ENTRY IF NEW ENTRY HAS GREATER DRAFT
            if (EntryDepth(bucket->entries[i]) < depth){
                toReplace = &(bucket->entries[i]);
                goto Replace;
            }
            
            // RETURN SO WE DON'T STORE TWO MATCHING HASHES
            return;
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

void dumpTranspositionTable(TransTable * table){
    
    printf("USED %d of %d\n",table->used,4*(table->maxSize));
    
    TransEntry entry;
    int i, j, depth, type, data[MaxHeight][4];
    
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