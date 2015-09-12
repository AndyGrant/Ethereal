#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Engine.h"

char * encodeBoard(Board * board){
	char * str = malloc(sizeof(char) * 135);
	
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
	str[134] = 0;
	return str;
}

int main(int argc, char * argv[]){
	Board * board = createBoard(argv[1]);
	int move[5] = {0,0,0,0,0};
	board->LastMove = move;
	int turn = argv[2][0] == '1' ? WHITE : BLACK;
	int size = 0;
	int * moves = getAllMoves(board,turn,&size);
	char test[4] = "see\0";
	int i;
	for (i = 0; i < size; i++, moves+=5){
		ApplyMove(board,moves);
		printf("%d %d %d %d %s \n",moves[1],moves[2],moves[0],moves[4],encodeBoard(board));
		RevertMove(board,moves);
	}
}
