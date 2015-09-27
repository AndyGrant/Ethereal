#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Board.h"

Board * copyBoard(Board * old){
	Board * new = malloc(sizeof(Board));
	memcpy(new->Types,old->Types,sizeof(int) * 64);
	memcpy(new->Colors,old->Colors,sizeof(int) * 64);
	new->TYPES = *(new->Types);
	new->COLORS = *(new->Colors);		
	new->KingLocations[0] = old->KingLocations[0];
	new->KingLocations[1] = old->KingLocations[1];
	new->Castled[0] = old->Castled[0];
	new->Castled[1] = old->Castled[1];
	new->ValidCastles[0][0] = old->ValidCastles[0][0];
	new->ValidCastles[0][1] = old->ValidCastles[0][1];
	new->ValidCastles[1][0] = old->ValidCastles[1][0];
	new->ValidCastles[1][1] = old->ValidCastles[1][1];
	memcpy(new->PieceCount,old->PieceCount,sizeof(old->PieceCount));
	return new;
}

Board * createBoard(char setup[137]){
	int x,y,i;
	Board * board = malloc(sizeof(Board));
	
	for(x = 0, i = 0; x < 8; x++){
		for(y = 0; y < 8; y++){
			board->Types[x][y] = setup[i++] - '0';
			board->Colors[x][y] = setup[i++] - '0';				
			if (board->Types[x][y] == KING)
				board->KingLocations[board->Colors[x][y]] = x*8 + y;
		}
	}
	
	for(x = 0; x < 2; x++)
		for(y = 0; y < 6; y++)
			board->PieceCount[x][y] = 0;
			
	for(x = 0; x < 8; x++)
		for(y = 0; y < 8; y++)
			if (board->Types[x][y] != EMPTY)
				board->PieceCount[board->Colors[x][y]][board->Types[x][y]] += 1;
	
	board->Castled[0] = setup[i++] - '0';
	board->Castled[1] = setup[i++] - '0';
	board->ValidCastles[0][0] = setup[i++] - '0';
	board->ValidCastles[0][1] = setup[i++] - '0';
	board->ValidCastles[1][0] = setup[i++] - '0';
	board->ValidCastles[1][1] = setup[i++] - '0';
	
	int * move = malloc(sizeof(int) * 5);
	int enpass = (10 * (setup[134] - '0')) + (setup[135] - '0');
	if (enpass == 99){
		move[0] = 0;
		move[2] = 0;
		board->LastMove = move;
	} else {
		move[0] = 4;
		move[2] = enpass;
		board->LastMove = move;
	}
	
	board->FiftyMoveRule = 50;
	
	board->TYPES = *(board->Types);
	board->COLORS = *(board->Colors);
	
	return board;
}
