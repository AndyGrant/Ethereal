#ifndef TTABLE_HEADER
#define TTABLE_HEADER

#include "Engine.h"

#define EXACT 1
#define LOWERBOUND 2
#define UPPERBOUND 3

#define NUM_BUCKETS 33554432
#define BUCKET_SIZE 4

#define KEY_SIZE 9

typedef struct Node{
	int * key;
	int value;
	int depth;
	int type;
	int turn;
} Node;

typedef struct Bucket{
	int max;
	int size;
	Node ** nodes;
} Bucket;

typedef struct TTable{
	int size;
	Bucket * buckets[NUM_BUCKETS];
} TTable;

Node * createNode(int * key, int value, int depth, int type, int turn);
Bucket * createBucket();
TTable * createTTable();

void setNodeType(Node * node, int alpha, int beta, int value);
Node * getNode(TTable * table, int hash, int * key);
void storeNode(TTable * table, int hash, Node * node);

int * createKey(Board * board);
int createHash(int * key);
 
#endif
