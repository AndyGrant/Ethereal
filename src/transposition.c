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
    uint32_t hashComp = (uint32_t)(hash >> 32);
    
    if (bucket->slot1[0].hash == hashComp)
        toReturn = &(bucket->slot1[0]);
    
    else if (bucket->slot1[1].hash == hashComp)
        toReturn = &(bucket->slot1[1]);
   
    else if (bucket->slot2[0].hash == hashComp)
        toReturn = &(bucket->slot2[0]);
    
    else if (bucket->slot2[1].hash == hashComp)
        toReturn = &(bucket->slot2[1]);
    
    else if (bucket->always.hash == hashComp)
        toReturn = &(bucket->always);
    
    if (toReturn != NULL)
        toReturn->info = (table->age << 3) | (toReturn->info & 0b111);
    
    return toReturn;
}

void storeTranspositionEntry(TransTable * table, int8_t depth, int8_t turn, int8_t type, int value, uint16_t bestMove, uint64_t hash){
    int i, j, hashComp = (uint32_t)(hash >> 32);
    TransBucket * bucket = &(table->buckets[hash % table->maxSize]);
    TransEntry * toReplace = NULL;
    TransEntry * candidateOne;
    TransEntry * candidateTwo;
    TransEntry * entries[5] = {
        &(bucket->slot1[0]),
        &(bucket->slot1[1]),
        &(bucket->slot2[0]),
        &(bucket->slot2[1]),
        &(bucket->always  )
    };
    
    
    // SEARCH FOR ENTRY WITH MATCHING HASH
    for (i = 0; i < 5; i++){
        if (entries[i]->hash == hashComp){
            if (depth >= entries[i]->depth){
                toReplace = entries[i];
                goto Replace;
            }
        }
        
        if (entries[i]->info == 0){
            toReplace = entries[i];
            goto Replace;
        }
    }
    
    // GRAB THE ENTRIES IN THE PROPER SLOT
    j = 2 * (hash & 1);
    candidateOne = entries[j];
    candidateTwo = entries[j+1];
    
    // SEARCH FOR A LOWER DEPTH ENTRY TO REPLACE
    toReplace = candidateOne->depth > candidateTwo->depth ? candidateTwo : candidateOne;
    toReplace = depth >= toReplace->depth ? toReplace : NULL;
    if (toReplace != NULL)
        goto Replace;
    
    // SEARCH FOR OLD ENTRY WITH LOWEST DEPTH TO REPLACE
    candidateOne = EntryAge(candidateOne) != table->age ? candidateOne : NULL;
    candidateTwo = EntryAge(candidateTwo) != table->age ? candidateTwo : NULL;
    if (candidateOne != NULL || candidateTwo != NULL){
        if (candidateOne != NULL && candidateTwo != NULL)
            toReplace = candidateOne->depth < candidateTwo->depth ? candidateOne : candidateTwo;
        else
            toReplace = candidateOne == NULL ? candidateTwo : candidateOne;
        goto Replace;
    }
    
    // NO ENTRY FOUND TO REPLACE, USE THE ALWAYS REPLACE
    toReplace = entries[4];
    
    // PERFORM THE REPLACEMENT
    Replace:
        toReplace->depth = depth;
        toReplace->info  = (table->age << 3) | (type << 1) | (turn);
        toReplace->value = (int16_t)value;
        toReplace->best  = bestMove;
        toReplace->hash  = hashComp;
}

void dumpTranspositionTable(TransTable * table){
    /*
    int i, j, data[MaxHeight][4];
    
    for (i = 0; i < MaxHeight; i++)
        for (j = 0; j < 4; j++)
            data[i][j] = 0;
        
    for (i = 0; i < table->maxSize; i++)
        data[table->entries[i].depth][table->entries[i].type]++;
    
    printf("\n\n");
    printf("<-----------TRANSPOSITION TABLE---------->\n");
    printf("| Depth  |   PV    |    CUT    |   ALL   |\n");
    for (i = 0; i < MaxHeight; i++)
        if (data[i][1] || data[i][2] || data[i][3])
            printf("|%8d|%9d|%11d|%9d|\n",i,data[i][1],data[i][2],data[i][3]);
    printf("\n\n");*/
}