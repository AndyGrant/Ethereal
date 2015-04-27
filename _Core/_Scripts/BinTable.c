#include <stdio.h>
#include <stdlib.h>

#include "BinTable.h"
#include "Engine.h"


int KEY_SIZE = 9;
int KEY_BYTES = 9 * sizeof(int);

int enc[6] = {1,1,2,2,3,3};
int MOVED_MATTERS[6] = {0,1,1,0,1,0};

/*
 * Function : createTable
 * ----------------------
 * 	Construct a Binary Tree to hold Transpositions
 * 
 * 	Arguments:
 * 		None
 */
BinaryTable * createTable(){
	BinaryTable * tree = malloc(sizeof(BinaryTable));
	tree->elements = 0;
	tree->root = NULL;
	return tree;
}


/* 
 * Function : createNode
 * ---------------------
 * 	Create a node with given value and key
 * 
 * 	Arguments:
 * 		value : value of position
 * 		key : encoded board
 */
Node * createNode(int value, int * key){
	Node * node = malloc(sizeof(Node));
	node->value = value;
	node->key = key;
	node->left = NULL;
	node->right = NULL;
	return node;
}


/*
 * Function : destroyTable
 * -----------------------
 * 	Free a table and all it's transpositions
 * 	
 * 	Arguments:
 * 		table : BinaryTable to free
 */
void destroyTable(BinaryTable * table){
	if (table->root != NULL)
		destroyNode(table->root);
	else
		free(table->root);
	free(table);
}


/*
 * Function : destroyNode
 * ----------------------
 * 	Free a node, it's key and it's branches
 * 	
 * 	Arguments:
 * 		node : Node to free
 */
void destroyNode(Node * node){
	free(node->key);
	if (node->left != NULL)
		destroyNode(node->left);
	else
		free(node->left);
	if (node->right != NULL)
		destroyNode(node->right);
	else
		free(node->right);
	free(node);	
}


/*
 * Function insertElement
 * -----------------------
 * 	Create and insert a node into it's position based
 * 		off of a given board encoding called key
 * 	
 * 	Arguments:
 * 		table : BinaryTable to store in
 * 		value : value for created Node
 * 		key : board encoding for locating
 */
void insertElement(BinaryTable * table, int value, int * key){
	table->elements += 1;
	
	if (table->root == NULL){
		table->root = createNode(value,key);
		return;
	}
		
	Node * node = table->root;
	while(1){
		int comp = memcmp(key,node->key,KEY_BYTES);
		if (comp == -1){
			if (node->left == NULL){
				node->left = createNode(value,key);
				return;
			}
			else
				node = node->left;
		}
		
		else{
			if (node->right == NULL){
				node->right = createNode(value,key);
				return;
			}
			else
				node = node->right;
		}
	}
}


/*
 * Function : getElement
 * ---------------------
 * 	Retrieve a node from the tree which matches the
 * 		the key passed into the function
 * 
 * 	Arguments:
 * 		table : BinaryTable to serach
 * 		key : key to search for 
 */
Node * getElement(BinaryTable * table, int * key){
	Node * node = table->root;
	
	if (node == NULL)
		return NULL;
		
	while(1){
		int comp = memcmp(key,node->key,KEY_BYTES);
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


/*
 * Function : encodeBoard
 * ----------------------
 * 	Uniquly identify the relevant components of a board
 * 	 as nine integers, the first eight being a row compressed
 * 	 into a single integer, each tile in the row represented 
 * 	 with a total of 4 bits
 * 
 * 	Arguments: 
 * 		board : board to encode
 * 		enpass : zero or index of last pawn move enabling enpass
 */
int * encodeBoard(Board * board, int enpass){
	int * key = malloc(KEY_SIZE * sizeof(int));
	int x, y, i, j;
	int t,c,m;	
	
	for(x = 0, i = 0; x < 8; x++, i++){
		key[i] = 0;
		for(y = 0; y < 8; y++){
			key[i] <<= 4;
			t = board->types[x][y];
			c = board->colors[x][y];
			m = board->moved[x][y];
			
			if (t == EMPTY)
				key[i] += 0;
			else if (MOVED_MATTERS[t])
				key[i] += enc[t] + (3 * c);
			else{
				if (m == 0)
					key[i] += 15;
				else
					key[i] += enc[t] + (3 * c) + 7;
				
			}
		}
	}
	
	key[8] = enpass;	
	return key;			
}
