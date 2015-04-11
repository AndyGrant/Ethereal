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


struct BinaryTable * createTable(int size);
struct BinaryTree * createTree();
Node * createNode(int value, char * key);

void destroyTable(struct BinaryTable * table);
void destroyTree(struct BinaryTree * tree);
void destroyNode(Node * node);

void insertElement(struct BinaryTable * table, int depth, int value, char * key);
Node * getElement(struct BinaryTable * table, int depth, char * key);

int compareKey(char * key1, char * key2);
char * encodeBoard(struct Board * b, int enpass, int turn);

#endif
