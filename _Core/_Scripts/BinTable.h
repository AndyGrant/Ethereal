#ifndef BinTable
#define BinTable

#include "Engine.h"

extern int KEY_SIZE;
extern int LAST_KEY;

typedef struct BinaryTable{
	struct Node * root;
	int elements;
	
} BinaryTable;

typedef struct Node{
	struct Node * left;
	struct Node * right;
	int value;
	int * key;
	
} Node;


BinaryTable * createTable();
Node * createNode(int value, int * key);

void destroyTable(BinaryTable * table);
void destroyNode(Node * node);

void insertElement(BinaryTable * table, int value, int * key);
Node * getElement(BinaryTable * table, int * key);

int * encodeBoard(Board * b, int enpass);

#endif
