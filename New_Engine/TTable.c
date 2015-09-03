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
		if (memcmp(key,bucket->nodes[i]->key,sizeof(int) * KEY_SIZE))
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

int main(){
	char base[135] = "31112141512111310101010101010101999999999999999999999999999999999999999999999999999999999999999900000000000000003010204050201030001111";
	
	Board * board = createBoard(base);
	int move[5] = {0,0,0,0,0};
	board->LastMove= move;
	printf("Created Board");
	
	TTable * table = createTTable();
	printf("Created Table");
	
	int * key = createKey(board);
	Node * node = createNode(key,2,1,WHITE);
	int hash = createHash(key);
	
	printf("Created Node");
	
	storeNode(table,hash,node);
	
	printf("Stored Node");
	return;
	
	Node * n = getNode(table,hash,key);
	
	printf("Depth=%d",n->depth);
	
	
	/*
	
	int size = 0;
	int * moves = getAllMoves(board,WHITE,&size);
	int * moves_p = moves;
	
	int i;
	for(i = 0; i < size; i++,moves+=5){
		ApplyMove(board,moves);
		printf("key=%d\n",createHash(createKey(board)));
		RevertMove(board,moves);
	}
	
	free(moves_p);
	
	*/
}







