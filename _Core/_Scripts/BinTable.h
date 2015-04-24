#ifndef BinTable
#define BinTable

#include "engine.h"

extern int KEY_SIZE;
extern int LAST_KEY;

typedef struct BinaryTable{
	int elements;
	struct Node * root;
} BinaryTable;

typedef struct Node{
	int value;
	int * key;
	struct Node * left;
	struct Node * right;
} Node;


BinaryTable * createTable();
Node * createNode(int value, int * key);

void destroyTable(BinaryTable * table);
void destroyNode(Node * node);

void insertElement(BinaryTable * table, int value, int * key);
Node * getElement(BinaryTable * table, int * key);

int * encodeBoard(Board * b, int enpass);

#endif
