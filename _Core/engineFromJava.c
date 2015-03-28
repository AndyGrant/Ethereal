#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "_Scripts/Engine.h"
#include "_Scripts/ChessAI.h"

int convChar(char to_conv){
	return to_conv - '0';
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
	return findBestMoveIndex(board,last_move,turn);
}