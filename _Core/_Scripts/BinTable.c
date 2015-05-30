#include <stdio.h>
#include <stdlib.h>

#include "BinTable.h"
#include "Engine.h"


int KEY_SIZE = 9;
int KEY_BYTES = 9 * sizeof(int);

int enc[6] = {1,1,2,2,3,3};
int MOVED_MATTERS[6] = {0,1,1,0,1,0};


BinaryTable * createTable(){
	BinaryTable * tree = malloc(sizeof(BinaryTable));
	tree->elements = 0;
	tree->root = NULL;
	return tree;
}

Node * createNode(int value, int * key){
	Node * node = malloc(sizeof(Node));
	node->value = value;
	node->key = key;
	node->left = NULL;
	node->right = NULL;
	return node;
}

void destroyTable(BinaryTable * table){
	if (table->root != NULL)
		destroyNode(table->root);
	else
		free(table->root);
	free(table);
}

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

Node * getElement(BinaryTable * table, int * key){
	/*
	__asm__(
		"sub			$0x28,		%esp;"	
		"mov 		0x8(%ebp),	%eax;"
		"mov			(%eax),		%eax;"
		"mov 		%eax,		-0xc(%ebp);"
		"cmpl		$0x0,		-0xc(%ebp);"
		"je			not_found;"
		"jmp 		main_loop;"
		
		"main_loop:"
		"movl		$0x24,		0x8(%esp);"
		"mov			0xc(%ebp),	%eax;"
		"mov			%eax,		0x0(%esp);"
		"mov 		-0xc(%ebp),	%eax;"
		"mov			0xc(%eax),	%eax;"
		"mov 		%eax,		0x4(%esp);"
		"call 		_memcmp;"
		"cmpl		$0xffffffff,%eax;"
		"je			left_branch;"
		"cmpl		$0x1,		%eax;"
		"je 			right_branch;"
		"jmp			found;"
		
	
		"left_branch:"
		"mov 		-0xc(%ebp),	%eax;"
		"mov			0x0(%eax),	%eax;"
		"test		%eax, 		%eax;"
		"je 			not_found;"
		"mov 		%eax,		-0xc(%ebp);"
		"jmp 		main_loop;"
		
		"right_branch:"
		"mov 		-0xc(%ebp),	%eax;"
		"mov			0x4(%eax),	%eax;"
		"test		%eax, 		%eax;"
		"je 			not_found;"
		"mov 		%eax,		-0xc(%ebp);"
		"jmp 		main_loop;"
		
		
		"not_found:"
		"mov 		$0x0,		%eax;"
		"leave;"
		"ret;"
		
		"found:"
		"mov 		-0xc(%ebp),	%eax;"
		"leave;"
		"ret;"
		
	);*/
	
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

int * encodeBoard(Board * board, int enpass){
	int * key = calloc(KEY_SIZE * sizeof(int));
	int x, y, i, j;
	int t,c,m;	
	
	for(x = 0, i = 0; x < 8; x++, i++){
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
