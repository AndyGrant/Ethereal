#ifndef TranspositionTable
#define TranspositionTable

#include "Engine.h"

typedef struct TTable{
	struct HashMap * maps;
	int num_maps;
	int entries;
	int last_accessed;
} TTable;

typedef struct HashMap{
	struct Bucket * buckets;
	int num_buckets;
	int entries;
	void * Hash;
} HashMap;

typedef struct Bucket{
	struct Node * nodes;
	int entries;
	int max;
} Bucket;

typedef struct Node{
	int value;
	char * key;
} Node;

TTable * constructTranspositionTable(int depth, int num_buckets, void * hash);
void constructHashMap(HashMap * map, int num_buckets, void * hash);
void constructBucket(Bucket * bucket);
Node * getElement(TTable * table, int depth, char * key);
Node * getNode(Bucket * bucket, char * key);
void insertElement(TTable * table, int depth, char * key, int value);
void expandBucket(Bucket * bucket);
char * encodeBoard(struct Board * board);
int hashBoard(struct Board * board);


#endif