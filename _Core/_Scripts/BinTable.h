#ifndef BinTable
#define BinTable

#include "engine.h"

typedef struct BinaryTable{
	int size;
	int elements;
	struct BinaryTree ** trees;
} BinaryTable;

typedef struct BinaryTree{
	int elements;
	struct Node * root;
} BinaryTree;

typedef struct Node{
	int value;
	char * key;
	struct Node * left;
	struct Node * right;
} Node;


BinaryTable * createTable(int size);
BinaryTree * createTree();
Node * createNode(int value, char * key);

void destroyTable(BinaryTable * table);
void destroyTree(BinaryTree * tree);
void destroyNode(Node * node);

void insertElement(BinaryTable * table, int depth, int value, char * key);
Node * getElement(BinaryTable * table, int depth, char * key);

int compareKey(char * key1, char * key2);
char * encodeBoard(Board * b, int enpass, int turn);

#endif
