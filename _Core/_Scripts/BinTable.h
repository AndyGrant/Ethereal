#ifndef BinTable
#define BinTable

#include "Engine.h"

extern int KEY_SIZE;
extern int LAST_KEY;
extern int EXACT;
extern int LOWERBOUND;
extern int UPPERBOUND;

typedef struct BinaryTable{
	struct Node * root;
	int elements;
	
} BinaryTable;

typedef struct Node{
	struct Node * left;
	struct Node * right;
	int value;
	int * key;
	int depth;
	int type;
	
} Node;


BinaryTable * createTable();
Node * createNode(int value, int * key, int depth, int type);

void destroyTable(BinaryTable * table);
void destroyNode(Node * node);

void insertElement(BinaryTable * table, int value, int * key, int depth, int type);
Node * getElement(BinaryTable * table, int * key);

int * encodeBoard(Board * b, int enpass, int turn);

#endif
