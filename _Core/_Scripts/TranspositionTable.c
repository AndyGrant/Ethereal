#include <stdio.h>
#include <stdlib.h>

#include "TranspositionTable.h"
#include "Engine.h"

int NUM_BUCKETS = 128;

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
		
	table->num_maps = depth;
	table->entries = 0;
	
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
	bucket->nodes = calloc(NUM_BUCKETS,sizeof(Node));
	if (bucket->nodes == NULL)
		exit(EXIT_FAILURE);
		
	bucket->entries = 0;
	bucket->max = NUM_BUCKETS;
}

Node * getElement(TTable * table, int depth, char * key){
	int (*hash)(char [66]) = table->maps[depth].Hash;
	int hash_code = hash(key);
	return getNode(&(table->maps[depth].buckets[hash_code]), key);
}

Node * getNode(Bucket * bucket, char * key){
	int n;
	for(n = 0; n < bucket->entries; n++){
		if(compareString(key,bucket->nodes[n].key) == 1)
			return &(bucket->nodes[n]);
	}
	
	return NULL;
}

int compareString(char * a, char * b){
	int i;
	for(i = 0; i < 66; i++)
		if (a[i] != b[i])
			return 0;
	return 1;
}

void insertElement(TTable * table, int depth, char * key, int value){
	int (*hash)(char [66]) = table->maps[depth].Hash;
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

char * encodeBoard(Board * board, int enpass, int turn){
	char * key = calloc(66,sizeof(char));
	int x,y,i;
	
	for(x = 0, i = 0; x < 8; x++)
		for(y = 0; y < 8; y++, i++){
			if (board->types[x][y] == 9)
				key[i] = (char)200;
			else{
				int m = board->moved[x][y] * 40;
				int c = board->colors[x][y] * 15;
				int t = board->types[x][y];
				key[i] = (char)(t+c+m);
			}
		}
	key[64] = (char)(enpass);
	key[65] = (char)(turn);
	return key;
}

int hashBoard(char * key){
	int v = 0;
	int i;
	for(i = 0; i < 64; i++)
		if (key[i] != (char)(200))
			v += i;
	return v % NUM_BUCKETS;
}

void deleteTranspositionTable(TTable * table){
	int n;
	for(n = 0; n < table->num_maps; n++)
		deleteHashMap(&(table->maps[n]));
		
	free(table->maps);
	free(table);
}

void deleteHashMap(HashMap * map){
	int n;
	for(n = 0; n < map->num_buckets; n++)
		deleteBucket(&(map->buckets[n]));
	
	free(map->buckets);
}

void deleteBucket(Bucket * bucket){
	int n;
	for(n = 0; n < bucket->entries; n++)
		deleteNode(&(bucket->nodes[n]));
	
	free(bucket->nodes);
}

void deleteNode(Node * node){
	free(node->key);
}



