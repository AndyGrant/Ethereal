#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Engine.h"

char * encodeBoard(Board * board){
	char * str = malloc(sizeof(char) * 137);
	
	int i;
	for(i = 0; i < 64; i++){
		str[i*2] = board->TYPES[i] + '0';
		str[i*2+1] = board->COLORS[i] + '0';
	}
	
	str[128] = board->Castled[0] + '0';
	str[129] = board->Castled[1] + '0';
	str[130] = board->ValidCastles[0][0] + '0';
	str[131] = board->ValidCastles[0][1] + '0';
	str[132] = board->ValidCastles[1][0] + '0';
	str[133] = board->ValidCastles[1][1] + '0';
	
	if (board->LastMove[0] != 4){
		str[134] = '9';
		str[135] = '9';
	} else {
		str[134] = (board->LastMove[2] / 10) + '0';
		str[135] = (board->LastMove[2] % 10) + '0';
	}
	str[136] = 0;
	return str;
}

int main(int argc, char * argv[]){
	Board * board = createBoard(argv[1]);
	int turn = argv[2][0] == '0' ? WHITE : BLACK;
	int bestIndex = getBestMoveIndex(board,turn);
	int size = 0;
	int * moves = getAllMoves(board,turn,&size);
	ApplyMove(board,(moves + (5*bestIndex)));
	printf("NEWBOARD=%s",encodeBoard(board));
}
	