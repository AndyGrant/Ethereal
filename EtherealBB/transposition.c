#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "move.h"
#include "types.h"
#include "transposition.h"

void init_transposition_table(TranspositionTable * table, int key_size){
    table->entries = malloc(sizeof(TranspositionEntry) * (1 << key_size));
    printf("Size of Table = %d MegaBytes\n",(sizeof(TranspositionEntry) * (1 << key_size))/(1024*1024));
    table->max_size = 1 << key_size;
    table->key_size = key_size;
    table->num_entries = 0;
    table->hits = 0;
    table->misses = 0;
    table->key_collisions = 0;
    
    int i;
    for (i = 0; i < table->max_size; i++){
        table->entries[i].depth = 0;
        table->entries[i].turn = 0;
        table->entries[i].type = 0;
        table->entries[i].value = 0;
        table->entries[i].best_move = 0;
        table->entries[i].hash = 0;
    }
}

TranspositionEntry * get_transposition_entry(TranspositionTable * table, uint64_t hash){
    TranspositionEntry * entry = &(table->entries[hash % table->max_size]);
    
    if (entry->hash != hash){
        if (entry->type != 0)
            table->key_collisions++;
        else
            table->misses++;
        return NULL;
    }
    
    table->hits++;  
    return entry;
}

void store_transposition_entry(TranspositionTable * table, int8_t depth, int8_t turn, int8_t type, int value, uint16_t best_move, uint64_t hash){
    TranspositionEntry * entry = &(table->entries[hash % table->max_size]);
    
    if (entry->type == 0){
        entry->depth = depth;
        entry->turn = turn;
        entry->type = type;
        entry->value = value;
        entry->best_move = best_move;
        entry->hash = hash;
        table->num_entries++;
        return;
    } 
    
    else if (entry->type != PVNODE){
        if (depth >= entry->depth || type == PVNODE){
            entry->depth = depth;
            entry->turn = turn;
            entry->type = type;
            entry->value = value;
            entry->best_move = best_move;
            entry->hash = hash;
            return;
        }   
    } 
    
    else if (type == PVNODE && depth > entry->depth){
        entry->depth = depth;
        entry->turn = turn;
        entry->type = type;
        entry->value = value;
        entry->best_move = best_move;
        entry->hash = hash;
        return;
    }
}

void dump_transposition_table(TranspositionTable * table){
    int i, j, data[MaxHeight][4];
    
    for (i = 0; i < MaxHeight; i++)
        for (j = 0; j < 4; j++)
            data[i][j] = 0;
        
    for (i = 0; i < table->max_size; i++)
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

void extract_pv_from_transposition_table(TranspositionTable * table, Board * board, int depth, uint16_t dest[MaxHeight]){
    
    TranspositionEntry * PV[MaxHeight];
    Undo undo[MaxHeight];
    
    int i, j;
    for (i = 0; i < depth; i++){
        PV[i] = get_transposition_entry(table,board->hash);
        apply_move(board,PV[i]->best_move,&(undo[i]));
    }
    
    for (i = depth-1; i >= 0 ; i--)
        revert_move(board,PV[i]->best_move,&(undo[i]));
    
    for (i = 0; i < depth; i++)
        dest[i] = PV[i]->best_move;
}

























