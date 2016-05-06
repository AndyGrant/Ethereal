#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "move.h"
#include "types.h"
#include "transposition.h"

void initalizeTranspositionTable(TransTable * table, int keySize){
    
    if (table->keySize == keySize){
        table->age = (table->age + 1) % (0b11111);
        return;
    }
    
    table->buckets = calloc(1 << keySize,sizeof(TransBucket));
    table->maxSize = 1 << keySize;
    table->keySize = keySize;
    table->age = 0;
}

TransEntry * getTranspositionEntry(TransTable * table, uint64_t hash){
    TransBucket * bucket = &(table->buckets[hash % table->maxSize]);
    TransEntry * toReturn = NULL;
    int i;
    
    for (i = 0; i < 4; i++){
        if (bucket->entries[i].hash == hash){
            toReturn = &(bucket->entries[i]);
            toReturn->info = (table->age << 3) | (toReturn->info & 0b111);
            break;
        }
    }
    
    return toReturn;
}

void storeTranspositionEntry(TransTable * table, int depth, int turn, int type, int value, uint16_t bestMove, uint64_t hash){
    
    TransBucket * bucket = &(table->buckets[hash % table->maxSize]);
    TransEntry * toReplace = NULL;
    TransEntry * candidate;
    int i, j;
    
    // SEARCH FOR ENTRY WITH MATCHING HASH
    for (i = 0; i < 4; i++){
        if (bucket->entries[i].hash == hash){
            if (depth >= bucket->entries[i].depth){
                toReplace = &(bucket->entries[i]);
                goto Replace;
            }
            
            candidate = &(bucket->entries[i]);
            candidate->info = (table->age << 3) | (candidate->info & 0b111);
            return;
        }
    }
    
    // FIND AND COMPARE LOWEST ENTRY TO NEW ONE
    candidate = &(bucket->entries[0]);
    for (i = 1; i < 4; i++){
        if (bucket->entries[i].depth < candidate->depth)
            candidate = &(bucket->entries[i]);
    }
    if (depth > candidate->depth){
        toReplace = candidate;
        goto Replace;
    }
    
    // FIND AND COMPARE LOWEST OLD ENTRY TO NEW ONE
    // IF NO OLD ENTRIES TO REPLACE WE WILL BE
    // LEFT WITH THE ALWAYS REPLACE ENTRY
    candidate = &(bucket->entries[3]);
    for (i = 2; i >= 0; i--){
        if (EntryAge(&(bucket->entries[i])) != table->age
            && bucket->entries[i].depth <= candidate->depth)
            candidate = &(bucket->entries[i]);
    } toReplace = candidate;
    
    // PERFORM THE REPLACEMENT
    Replace:    
        toReplace->depth = (uint8_t)depth;
        toReplace->info  = (uint8_t)((table->age << 3) | (type << 1) | (turn));
        toReplace->value = (int16_t)value;
        toReplace->best  = bestMove;
        toReplace->hash  = hash;
}

void dumpTranspositionTable(TransTable * table){
    /*int i, j, data[MaxHeight][4];
    TransBucket bucket;
    
    for (i = 0; i < MaxHeight; i++)
        for (j = 0; j < 4; j++)
            data[i][j] = 0;
        
    for (i = 0; i < table->maxSize; i++){
        bucket = table->buckets[i];
        data[bucket.slot1[0].depth][EntryType(&(bucket.slot1[0]))]++;
        data[bucket.slot1[1].depth][EntryType(&(bucket.slot1[1]))]++;
        data[bucket.slot2[0].depth][EntryType(&(bucket.slot2[0]))]++;
        data[bucket.slot2[1].depth][EntryType(&(bucket.slot2[1]))]++;
        data[bucket.always  .depth][EntryType(&(bucket.always  ))]++;
    }
    
    printf("\n\n");
    printf("<-----------TRANSPOSITION TABLE---------->\n");
    printf("| Depth  |   PV    |    CUT    |   ALL   |\n");
    for (i = 0; i < MaxHeight; i++)
        if (data[i][1] || data[i][2] || data[i][3])
            printf("|%8d|%9d|%11d|%9d|\n",i,data[i][1],data[i][2],data[i][3]);
    printf("\n\n");*/
}