#include <stdio.h>
#include <stdlib.h>

#include "TranspositionTable.h"
#include "Engine.h"

TTable * constructTranspositionTable(int depth, int num_buckets, void * hash){
	TTable * table = malloc(sizeof(TTable));
	if (table == NULL)
		exit(EXIT_FAILURE);
	
	table->maps = calloc(depth, sizeof(HashMap));
	if (table->maps == NULL)
		exit(EXIT_FAILURE);
	
	int n;
	for(n = 0; n < depth; n++)
		constructHashMap(&(table->maps[n]),num_buckets,hash);
	
	return table;
}

void constructHashMap(HashMap * map, int num_buckets, void * hash){
	map->buckets = calloc(num_buckets,sizeof(Bucket));
	if (map->buckets == NULL)
		exit(EXIT_FAILURE);
	
	map->entries = 0;
	map->num_buckets = num_buckets;
	map->Hash = hash;
	
	int n;
	for(n = 0; n < num_buckets; n++)
		constructBucket(&(map->buckets[n]));
}

void constructBucket(Bucket * bucket){
	bucket->nodes = calloc(4,sizeof(Node));
	if (bucket->nodes == NULL)
		exit(EXIT_FAILURE);
		
	bucket->entries = 0;
	bucket->max = 4;
}

Node * getElement(TTable * table, int depth, char * key){
	int (*hash)(char [64]) = table->maps[depth].Hash;
	int hash_code = hash(key);
	return getNode(&(table->maps[depth].buckets[hash_code]), key);
}

Node * getNode(Bucket * bucket, char * key){
	int n;
	for(n = 0; n < bucket->entries; n++)
		if (strcmp(&key,&(bucket->nodes[n].key)) == 0)
			return &(bucket->nodes[n]);
	
	return NULL;
}

void insertElement(TTable * table, int depth, char * key, int value){
	int (*hash)(char *) = table->maps[depth].Hash;
	int hash_code = hash(key);
	Bucket * bucket = &(table->maps[depth].buckets[hash_code]);
	if (bucket->entries == bucket->max)
		expandBucket(bucket);
	
	bucket->nodes[bucket->entries].value = value;
	bucket->nodes[bucket->entries].key = key;
	
	table->entries += 1;
	table->maps[depth].entries += 1;
	bucket->entries += 1;
}

void expandBucket(Bucket * bucket){
	printf("Expanded");
	bucket->max *= 2;
	Node * temp = calloc(bucket->max,sizeof(Node));
	
	int n;
	for(n = 0; n < bucket->entries; n++){
		temp[n].value = bucket->nodes[n].value;
		temp[n].key = bucket->nodes[n].key;
	}
	
	free(bucket->nodes);
	bucket->nodes = temp;
}

char * encodeBoard(Board * board){
	char * key = calloc(64,sizeof(char));
	int x,y,i;
	
	for(x = 0, i = 0; x < 8; x++)
		for(y = 0; y < 8; y++, i++){
			if (board->types[x][y] == 9)
				key[i] = (char)240;
			int m = board->moved[x][y] * 100;
			int c = board->colors[x][y] * 50;
			int t = board->types[x][y];
			key[i] = (char)(t+c+m);
		}
	
	return key;
}

int hashBoard(Board * board){
	int v,x,y,i;
	for(v = 0, i = 0, x = 0; x < 8; x++)
		for(y = 0; y < 8; i++)
			if (board->types[x][y] != 9)
				v += i;
	return v % 64;
}



