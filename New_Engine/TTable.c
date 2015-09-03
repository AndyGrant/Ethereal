#include <stdio.h>
#include <stdlib.h>
#include "Engine.h"
#include "TTable.h"


Node * createNode(int * key, int depth, int type, int turn){
	Node * node = malloc(sizeof(Node));
	node->key = key;
	node->depth = depth;
	node->type = type;
	node->turn = turn;
	return node;
}

Bucket * createBucket(){
	Bucket * bucket = malloc(sizeof(Bucket));
	bucket->max = BUCKET_SIZE;
	bucket->size = 0;
	bucket->nodes = malloc(sizeof(Node * ) * BUCKET_SIZE);
	return bucket;
}

TTable * createTTable(){
	TTable * table = malloc(sizeof(TTable));
	table->size = 0;
	
	int i;
	for(i = 0; i < NUM_BUCKETS; i++)
		table->buckets[i] = createBucket();
		
	return table;
}

Node * getNode(TTable * table, int hash, int * key){
	Bucket * bucket = table->buckets[hash];
	
	int i = 0;
	while(i < bucket->size){
		if (memcmp(key,bucket->nodes[i]->key,sizeof(int) * KEY_SIZE) == 0)
			return bucket->nodes[i];
		i += 1;
	}
	return NULL;
}

void storeNode(TTable * table, int hash, Node * node){
	
	Bucket * bucket = table->buckets[hash];
	
	if(bucket->size < bucket->max){
		bucket->nodes[bucket->size] = node;
		bucket->size += 1;
	} else {
		Node ** nodes = malloc(sizeof(Node *) * bucket->max * 2);
		
		int i;
		for(i = 0; i < bucket->size; i++)
			nodes[i] = bucket->nodes[i];
			
		nodes[bucket->size] = node;
		bucket->nodes = nodes;
		bucket->size += 1;
		bucket->max *= 2;
	}
}

int * createKey(Board * board){
	int * key = malloc(sizeof(int) * KEY_SIZE);
	
	int x,y;
	for(x = 0; x < 8; x++){
		key[x] = 0;
		for(y = 0; y < 8; y++){
			key[x] <<= 4;
			if (board->Types[x][y] == EMPTY)
				key[x] += 15;
			else
				key[x] += board->Colors[x][y] * 6 + board->Types[x][y];
		}
	}	
	
	key[8] = 0;
	return key;		
}

int createHash(int * key){
	int hash = 0;
	int i;
	for(i = 0; i < 8;){
		hash += key[i++];
		hash += 2*key[i++];
		hash += 3*key[i++];
		hash += 4*key[i++];
	}
	return abs(hash) % NUM_BUCKETS;
}








