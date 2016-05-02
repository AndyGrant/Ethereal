#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "move.h"
#include "types.h"
#include "transposition.h"

void initalizeTranspositionTable(TranspositionTable * table, int keySize){
    if (table->buckets != NULL)
        return;
    
    table->buckets = malloc(sizeof(TranspositionBucket) * (1 << keySize));
    table->maxSize = 1 << keySize;
    table->keySize = keySize;    
    
    int i;
    for (i = 0; i < table->maxSize; i++){
        table->buckets[i].entry1.age = 0;
        table->buckets[i].entry1.depth = 0;
        table->buckets[i].entry1.turn = 0;
        table->buckets[i].entry1.type = 0;
        table->buckets[i].entry1.value = 0;
        table->buckets[i].entry1.bestMove = 0;
        table->buckets[i].entry1.hash = 0;
        
        table->buckets[i].entry2.age = 0;
        table->buckets[i].entry2.depth = 0;
        table->buckets[i].entry2.turn = 0;
        table->buckets[i].entry2.type = 0;
        table->buckets[i].entry2.value = 0;
        table->buckets[i].entry2.bestMove = 0;
        table->buckets[i].entry2.hash = 0;
    }
}

TranspositionEntry * getTranspositionEntry(TranspositionTable * table, uint64_t hash){
    TranspositionBucket * bucket = &(table->buckets[hash % table->maxSize]);
    
    if (bucket->entry1.hash == hash)
        return &(bucket->entry1);
    
    if (bucket->entry2.hash == hash)
        return &(bucket->entry2);
    
    return NULL;
}

void storeTranspositionEntry(TranspositionTable * table, int8_t depth, int8_t turn, int8_t type, int value, uint16_t bestMove, uint64_t hash){
    TranspositionBucket * bucket = &(table->buckets[hash % table->maxSize]);
    TranspositionEntry * entry1 = &(bucket->entry1);
    TranspositionEntry * entry2 = &(bucket->entry2);
    
    int useDepthFirst = 0;
    
    if (entry1->type == 0)
        useDepthFirst = 1;
    
    else if (entry1->age != 0)
        useDepthFirst = 1;
    
    else if (entry1->type != PVNODE){
        if (depth >= entry1->depth || type == PVNODE){
            useDepthFirst = 1;
        }   
    }
    
    else if (type == PVNODE && depth > entry1->depth)
        useDepthFirst = 1;    
    
    
    if (useDepthFirst != 0){
        entry2->age      = entry1->age;
        entry2->depth    = entry1->depth;
        entry2->turn     = entry1->turn;
        entry2->type     = entry1->type;    
        entry2->value    = entry1->value;
        entry2->bestMove = entry1->bestMove;
        entry2->hash     = entry1->hash;

        entry1->age      = 0;
        entry1->depth    = depth;
        entry1->turn     = turn;
        entry1->type     = type;
        entry1->value    = value;
        entry1->bestMove = bestMove;
        entry1->hash     = hash;
    }
    
    else {
        entry2->age      = 0;
        entry2->depth    = depth;
        entry2->turn     = turn;
        entry2->type     = type;
        entry2->value    = value;
        entry2->bestMove = bestMove;
        entry2->hash     = hash;
    }  
}

void dumpTranspositionTable(TranspositionTable * table){
    /*int i, j, data[MaxHeight][4];
    
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
    printf("\n\n");
    
    free(table->entries);*/
}

void updateTranspositionTable(TranspositionTable * table){
    int i;
    for (i = 0; i < table->maxSize; i++){
        table->buckets[i].entry1.age++;
        table->buckets[i].entry2.age++;
    }
}
