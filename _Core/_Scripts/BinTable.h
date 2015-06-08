#ifndef BinTable
#define BinTable

#include "Engine.h"

extern int KEY_SIZE;
extern int LAST_KEY;
extern int LOWERBOUND;
extern int UPPERBOUND;
extern int EXACT;

typedef struct BinaryTable{
	struct BinaryTree ** trees;
	int elements;
	int size;
} BinaryTable;

typedef struct BinaryTree{
	struct Node * root;
	int elements;
} BinaryTree;

typedef struct Node{
	struct Node * left;
	struct Node * right;
	int value;
	int * key;
	int depth;
	int type;
} Node;


BinaryTable * createTable(int size);
BinaryTree * createTree();
Node * createNode(int value, int * key, int depth, int type);

void destroyTable(BinaryTable * table);
void destroyTree(BinaryTree * tree);
void destroyNode(Node * node);

void insertElement(BinaryTable * table, int value, int * key, int depth, int type);
Node * getElement(BinaryTable * table, int depth, int * key);

int * encodeBoard(Board * b, int enpass);

#endif
