#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "_Scripts/Engine.h"
#include "_Scripts/ChessAI.h"

int convChar(char to_conv){
	return to_conv - '0';
}

unsigned long long global_foo;
void foo(Board * board, int turn, int depth, int * last_move){
	if (depth == 0)
		return;
		
	int size = 0;
	int * moves = findAllValidMoves(board,turn,&size,last_move);
	
	int i;
	for(i = 0; i < size; i++){
		applyGenericMove(board,moves + i*7);
		
		
		global_foo += 1;
		if(global_foo % 10000000 == 0)
			printf("\r#%llu million",global_foo/10000000);
		
		foo(board,!turn,depth-1,moves + i*7);
		revertGenericMove(board,moves + i*7);
	}
	
	free(moves);
}

int main(int argc, char *argv[]){
	struct Board b;
	struct Board * board = &b;
	int last_move[3];
	int turn = convChar(argv[1][192]);
	
	int x,y,i = 0;
	for(x = 0; x < 8; x++)
		for(y = 0; y < 8; y++, i += 3){
			board->types[x][y] = convChar(argv[1][i]);
			board->colors[x][y] = convChar(argv[1][i+1]);
			board->moved[x][y] = convChar(argv[1][i+2]);
		}
		
	for(x = 0; x < 8; x++)
		for(y = 0; y < 8; y++)
			if (board->types[x][y] == 5)
				board->kingLocations[board->colors[x][y]] = x * 8 + y;
	
	last_move[0] = convChar(argv[1][193]);
	last_move[2] = 8 * convChar(argv[1][194]) + convChar(argv[1][195]);
	
	time_t start = time(NULL);
	
	//foo(board,WHITE,6,last_move);
	//printf("\n#%llu",global_foo);
	
	int best = -1;
	best = findBestMoveIndex(board,last_move,turn);
	printf("Seconds Taken: %d \n\n",(int)(time(NULL)-start));
	
	
	return best;
}
