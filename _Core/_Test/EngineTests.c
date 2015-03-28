#include <stdio.h>
#include <stdlib.h>
#include "../_Scripts/Engine.h"
#include "../_Scripts/Moves.h"

void printBoardStruct(struct Board *b);
void printMoves(int * argv, int sets, int stepSize);

int main(int argc, char *argv[]){
	int board[8][8];
	int lastMove[3];
	int expectedMovesCount = ((argc - 2) / 3) - 1;
	int * expectedMoves = malloc(28 * expectedMovesCount);
	int turn = argv[1][128] - '0';
	
	int i,x,y;
	
	for(x = 0; x < 8; x++)
		for(y = 0; y < 8; y++)
			board[x][y] = 10 * (argv[1][2*(x*8+y)] - '0') + argv[1][2*(x*8+y)+1] - '0';
	struct Board *b = createBoard(board);
	
	for(i = 2; i < 5; i++)
		lastMove[i-2] = 10 * (argv[i][0] - '0') + argv[i][1] - '0';
		
	for(i = 5; i < argc; i++)
		expectedMoves[i-5] = 10 * (argv[i][0] - '0') + argv[i][1] - '0';
		
	int size = 0;
	int * moves = findAllValidMoves(b,turn,&size,lastMove);
	
	int failed = 0;
	
	if (size != expectedMovesCount){
		failed = 1;
		printf("Sizes %d :: %d",size,expectedMovesCount);
	}
		
	else{
		for(x = 0; x < size; x++){
			for(y = 0; y < 3; y++){
				if (expectedMoves[x*3+y] != moves[x*7+y]){
					failed = 1; break;
				}
			}
		}
	}		

	if (failed){
	
		printf("FAILED \n");
		printBoardStruct(b);
		printf("Expected Moves \n");
		printMoves(expectedMoves,expectedMovesCount,3);
		printf("Found Moves \n");
		printMoves(moves,size,7);
		
		while (1){
			x = 2;
		}
	}
}

void printBoardStruct(struct Board *b){
	int x,y;
	for(x = 0; x < 8; x++){
		for(y = 0; y < 8; y++)
			printf("%d%d ",b->types[x][y],b->colors[x][y]);
		printf("\n");
	}	
}

void printMoves(int * argv, int sets, int stepSize){
	int x,y;
	for(x = 0; x < sets; x++){
		for(y = 0; y < 3; y++)
			printf("%d ",argv[x*stepSize+y]);
		printf("\n");
	}
}