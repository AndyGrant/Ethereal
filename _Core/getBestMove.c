#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "_Scripts/Engine.h"
#include "_Scripts/Moves.h"
#include "_Scripts/ChessAI.h"

int main(){
	int board_arr[8][8] = {
		{31,11,21,41,51,21,11,31},
		{01,01,01,01,01,01,01,01},
		{99,99,99,99,99,99,99,99},
		{99,99,99,99,99,99,99,99},
		{99,99,99,99,00,99,99,99},
		{99,99,99,99,99,99,99,99},
		{00,00,00,00,99,00,00,00},
		{30,10,20,40,50,20,10,30},
	};
	
	
	struct Board * board = createBoard(board_arr);
	int * last_move = malloc(12);
	last_move[0] = 9;
	int turn = 1;
	
	clock_t start = clock(), diff;
	
	int * best_move = findBestMove(board, last_move, turn);
	
	diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	printf("Time taken %d seconds %d milliseconds", msec/1000, msec%1000);
	
	printf("\n Best Move \n");
	int i;
	for(i = 0; i < 7; i++)
		printf("%d ",best_move[i]);
		
	

	
}