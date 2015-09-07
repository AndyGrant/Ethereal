#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "Engine.h"
#include "TTable.h"

Node * createNode(int * key, int value, int depth, int type, int turn){
	Node * node = malloc(sizeof(Node));
	node->key = key;
	node->value = value;
	node->depth = depth;
	node->type = type;
	node->turn = turn;
	return node;
}

Bucket * createBucket(int num){
	Bucket * bucket = malloc(sizeof(Bucket));
	bucket->max = BUCKET_SIZE;
	bucket->size = 0;
	pthread_mutex_t a = PTHREAD_MUTEX_INITIALIZER;
	bucket->lock = a;
	bucket->nodes = malloc(sizeof(Node * ) * BUCKET_SIZE);
	return bucket;
}

TTable * createTTable(){
	TTable * table = malloc(sizeof(TTable));
	table->size = 0;
	int i;
	for(i = 0; i < NUM_BUCKETS; i++)
		table->buckets[i] = createBucket(i);
	return table;
}

Node * getNode(TTable * table, int hash, int * key){
	pthread_mutex_lock(&(table->buckets[hash]->lock));
	Bucket * bucket = table->buckets[hash];
	
	int i = 0;
	while(i < bucket->size){
		if (memcmp(key,bucket->nodes[i]->key,sizeof(int) * KEY_SIZE) == 0){
			pthread_mutex_unlock(&(table->buckets[hash]->lock));
			return bucket->nodes[i];
		}
		i += 1;
	}
	pthread_mutex_unlock(&(table->buckets[hash]->lock));
	return NULL;
}

void storeNode(TTable * table, int hash, Node * node){
	pthread_mutex_lock(&(table->buckets[hash]->lock));
	table->size += 1;
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
		free(bucket->nodes);
		bucket->nodes = nodes;
		bucket->size += 1;
		bucket->max *= 2;
	}
	pthread_mutex_unlock(&(table->buckets[hash]->lock));
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
	int i, hash = 0;
	
	// Bob Jenkins - One-At-A-Time
	for(i = 0; i < 8; i++){
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	
	return abs(hash) % NUM_BUCKETS;
}

int getNodeType(int alpha, int beta, int value){
	if (value <= alpha)
		return LOWERBOUND;
	else if (value >= beta)
		return UPPERBOUND;
	return EXACT;	
}

int getNonEmptyBucketCount(TTable * table){
	int size = 0;
	int i;
	for(i = 0; i < NUM_BUCKETS; i++)
		if (table->buckets[i]->size == 0)
			size += 1;
	
	return size;
}



