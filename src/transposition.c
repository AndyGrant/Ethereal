#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "move.h"
#include "types.h"
#include "transposition.h"

void initalizeTranspositionTable(TranspositionTable * table, int keySize){
    table->entries = malloc(sizeof(TranspositionEntry) * (1 << keySize));
    table->maxSize = 1 << keySize;
    table->keySize = keySize;    
    
    int i;
    for (i = 0; i < table->maxSize; i++){
        table->entries[i].depth = 0;
        table->entries[i].turn = 0;
        table->entries[i].type = 0;
        table->entries[i].value = 0;
        table->entries[i].bestMove = 0;
        table->entries[i].hash = 0;
    }
}

TranspositionEntry * getTranspositionEntry(TranspositionTable * table, uint64_t hash){
    TranspositionEntry * entry = &(table->entries[hash % table->maxSize]);
    
    if (entry->hash != hash)
        return NULL;
    
    return entry;
}

void storeTranspositionEntry(TranspositionTable * table, int8_t depth, int8_t turn, int8_t type, int value, uint16_t bestMove, uint64_t hash){
    TranspositionEntry * entry = &(table->entries[hash % table->maxSize]);
    
    if (entry->type == 0){
        entry->depth = depth;
        entry->turn = turn;
        entry->type = type;
        entry->value = value;
        entry->bestMove = bestMove;
        entry->hash = hash;
        return;
    } 
    
    else if (entry->type != PVNODE){
        if (depth >= entry->depth || type == PVNODE){
            entry->depth = depth;
            entry->turn = turn;
            entry->type = type;
            entry->value = value;
            entry->bestMove = bestMove;
            entry->hash = hash;
            return;
        }   
    } 
    
    else if (type == PVNODE && depth > entry->depth){
        entry->depth = depth;
        entry->turn = turn;
        entry->type = type;
        entry->value = value;
        entry->bestMove = bestMove;
        entry->hash = hash;
        return;
    }
}

void dumpTranspositionTable(TranspositionTable * table){
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
    printf("\n\n");
    
    free(table->entries);
}