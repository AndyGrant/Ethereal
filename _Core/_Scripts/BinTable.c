#include <stdio.h>
#include <stdlib.h>

#include "BinTable.h"
#include "engine.h"


int KEY_SIZE = 66;

BinaryTable * createTable(int size){
	BinaryTable * table = calloc(1,sizeof(BinaryTable));
	table->size = size;
	table->elements = 0;
	
	table->trees = calloc(size,sizeof(void *));
	
	int n;
	for(n = 0; n < size; n++)
		table->trees[n] = createTree();
	
	return table;
}

BinaryTree * createTree(){
	BinaryTree * tree = calloc(1,sizeof(BinaryTree));
	tree->elements = 0;
	tree->root = NULL;
	return tree;
}

Node * createNode(int value, char * key){
	Node * node = calloc(1,sizeof(Node));
	node->value = value;
	node->key = key;
	node->left = NULL;
	node->right = NULL;
	return node;
}


void destroyTable(BinaryTable * table){
	int n;
	for(n = 0; n < table->size; n++)
		destroyTree(table->trees[n]);
	free(table);
}

void destroyTree(BinaryTree * tree){
	destroyNode(tree->root);
	free(tree);
}

void destroyNode(Node * node){
	free(node->key);
	if (node->left != NULL)
		destroyNode(node->left);
	if (node->right != NULL)
		destroyNode(node->right);
	free(node);	
}


void insertElement(BinaryTable * table, int depth, int value, char * key){
	table->elements += 1;
	table->trees[depth]->elements += 1;
	
	if (table->trees[depth]->root == NULL){
		table->trees[depth]->root = createNode(value,key);
		return;
	}
		
	Node * node = table->trees[depth]->root;
	while(1){
		int comp = compareKey(key,node->key);
		if (comp == -1){
			if (node->left == NULL){
				node->left = createNode(value,key);
				return;
			}
			else
				node = node->left;
		}
		
		if (comp == 1){
			if (node->right == NULL){
				node->right = createNode(value,key);
				return;
			}
			else
				node = node->right;
		}
		
		if (comp == 0)
			return;
	}
}

Node * getElement(BinaryTable * table, int depth, char * key){
	Node * node = table->trees[depth]->root;
	
	if (node == NULL)
		return NULL;
		
	while(1){
		int comp = compareKey(key,node->key);
		if (comp == -1){
			if (node->left == NULL)
				return NULL;
			node = node->left;
		}
		else if(comp == 1){
			if (node->right == NULL)
				return NULL;
			node = node->right;
		}
		else
			return node;
	}
}


int compareKey(char * key1, char * key2){
	int i = 0;
	for(i = 0; i < KEY_SIZE; i++){
		if (key1[i] > key2[i])
			return 1;
		else if(key1[i] < key2[i])
			return -1;
	}
	return 0;
}

char * encodeBoard(Board * board, int enpass, int turn){
	char * key = calloc(66,sizeof(char));
	int x,y,i,m,c,t;
	
	for(x = 0, i = 0; x < 8; x++){
		for(y = 0; y < 8; y++, i++){
			if (board->types[x][y] == 9)
				key[i] = (char)200;
			else{
				if (board->types[x][y] > PAWN && board->types[x][y] < KING && board->types[x][y] != ROOK)
					m = 40;
				else
					m = board->moved[x][y] * 40;
				c = board->colors[x][y] * 15;
				t = board->types[x][y];
				key[i] = (char)(t+c+m);
			}
		}
	}
	
	key[64] = (char)(enpass);
	key[65] = (char)(turn);
	return key;
}