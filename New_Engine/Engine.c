#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Engine.h"

/*
	Move Encodings

	NormalMove: [Move Type, Start, End, Captured Type, Breaks Castles]
	
	CastleMove: [Move Type, Start, End, Rook Start, Rook End]
	
	PromotionMove: [Move Type, Start, End, Captured Type, Promte Type]
	
	EnpassMove: [Move Type, Start, End, Enpass]

*/

int MOVE_MAP_KNIGHT[8][2] = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};
int MOVE_MAP_DIAGONAL[4][2] = {{1,1},{-1,1},{-1,-1},{1,-1}};
int MOVE_MAP_STRAIGHT[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
int MOVE_MAP_ALL[8][2] = {{1,1},{-1,1},{-1,-1},{1,-1},{1,0},{-1,0},{0,1},{0,-1}};

char base[135] = "31112141512111310101010101010101999999999999999999999999999999999999999999999999999999999999999900000000000000003010204050201030001111";
  

int MOVES_BUFFER[BUFFER_SIZE];
int EVALUTE_CHECK[8][8];

int * TYPES;
int * COLORS;

void (*GetPieceMoves[6])(Board *, int, int *, int, int, int) = {
	&getPawnMoves,
	&getKnightMoves,
	&getBishopMoves,
	&getRookMoves,
	&getQueenMoves,
	&getKingMoves
};

void (*ApplyTypes[5])(Board *, int *) = {
	&applyNormalMove,
	&applyCastleMove,
	&applyPromotionMove,
	&applyEnpassMove,
	&applyNormalMove
};

void (*RevertTypes[5])(Board *, int *) = {
	&revertNormalMove,
	&revertCastleMove,
	&revertPromotionMove,
	&revertEnpassMove,
	&revertNormalMove
};

unsigned long long global_foo = 0;
void foo(Board * board, int turn, int depth, int * last_move){
	if (depth == 0)
		return;
		
	board->LastMove = last_move;
	int size = 0;
	int * moves = getAllMoves(board,turn,&size);
	
	int i;
	for(i = 0; i < size; i++){
		(*ApplyTypes[moves[i*5]])(board,moves +i*5);
		
		
		global_foo += 1;
		if(global_foo % 1000000 == 0)
			printf("\r#%llu million",global_foo/1000000);
		
		foo(board,!turn,depth-1,moves + i*5);
		(*RevertTypes[moves[i*5]])(board,moves +i*5);
	}
	
	free(moves);
}

int main(){
	Board * board = createBoard(base);
	int move[5] = {0,0,0,0,0};
	board->LastMove = move;
	
	time_t start = time(NULL);
	foo(board,WHITE,6,move);
	printf("#%llu \n",global_foo);
	printf("Seconds Taken: %d \n\n",(int)(time(NULL)-start));
}

Board * createBoard(char setup[135]){
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
	
	board->Castled[0] = setup[i++] - '0';
	board->Castled[1] = setup[i++] - '0';
	board->ValidCastles[0][0] = setup[i++] - '0';
	board->ValidCastles[0][1] = setup[i++] - '0';
	board->ValidCastles[1][0] = setup[i++] - '0';
	board->ValidCastles[1][1] = setup[i++] - '0';
	
	board->FiftyMoveRule = 50;
	board->PreviousMovesSize = 0;
	
	TYPES = *(board->Types);
	COLORS = *(board->Colors);
	
	return board;
}

int * getAllMoves(Board * board, int turn, int * size){
	int x,y,i;
	for(x = 0; x < 8; x++)
		for(y = 0; y < 8; y++)
			if (board->Colors[x][y] == turn)
				(*GetPieceMoves[board->Types[x][y]])(board,turn,size,x,y,0);
				
	return memcpy(malloc(sizeof(int) * MOVE_SIZE * *size),MOVES_BUFFER,sizeof(int) * MOVE_SIZE * *size);
}

void getPawnMoves(Board * board, int turn, int * size, int x, int y, int check){
	int * lastMove = board->LastMove;
	int pt, dir = board->Colors[x][y] == WHITE ? -1 : 1;
	int start = x * 8 + y;
	
	// Forward One / Two
	if (board->Types[x+dir][y] == EMPTY){
		if (x+dir == 0 || x+dir == 7){
			for(pt = QUEEN; pt != PAWN; pt--){
				int move[5] = {2,start,start+8*dir,9,pt};
				createPromotionMove(board,turn,size,move,check);
			}
		}
		else{
			int move[5] = {0,start,start+8*dir,9,0};
			createNormalMove(board,turn,size,move,check);
			
			if (((turn == WHITE && x == 6) || (turn == BLACK && x == 1)) && board->Types[x+dir+dir][y] == 9){
				int move[5] = {4,start,start+16*dir,9,0};
				createNormalMove(board,turn,size,move,check);
			}		
		}
	}
	
	// En Passant
	if (lastMove[0] == 4 && abs(y - (lastMove[2]%8)) == 1){
		if (3 + turn == x && COLORS[lastMove[2]] != turn){
			int move[5] = {3,start,lastMove[2]+(8*dir),lastMove[2],0};
			createEnpassMove(board,turn,size,move,check);
		}
	}
	
	// Capture King Side
	if (y + 1 < 8 && board->Types[x+dir][y+1] != EMPTY && board->Colors[x+dir][y+1] != turn){
		if (x+dir == 0 || x+dir == 7){
			for(pt = QUEEN; pt != PAWN; pt--){
				int move[5] = {2,start,start+(8*dir)+1,board->Types[x+dir][y+1],pt};
				createPromotionMove(board,turn,size,move,check);
			}
		}
		else{
			int move[5] = {0,start,start+(8*dir)+1,board->Types[x+dir][y+1],0};
			createNormalMove(board,turn,size,move,check);
		}
	}
	
	//Capture Queen Side
	if (y - 1 >= 0 && board->Types[x+dir][y-1] != EMPTY && board->Colors[x+dir][y-1] != turn){
		if (x+dir == 0 || x+dir == 7){
			for(pt = QUEEN; pt != PAWN; pt--){
				int move[5] = {2,start,start+(8*dir)-1,board->Types[x+dir][y-1],pt};
				createPromotionMove(board,turn,size,move,check);
			}
		}
		else{
			int move[5] = {0,start,start+(8*dir)-1,board->Types[x+dir][y-1],0};
			createNormalMove(board,turn,size,move,check);
		}
	}
}

void getKnightMoves(Board * board, int turn, int * size, int x, int y, int check){
	int i, nx, ny, start = x * 8 + y;
	for(i = 0; i < 8; i++){
		nx = x + MOVE_MAP_KNIGHT[i][0];
		ny = y + MOVE_MAP_KNIGHT[i][1];
		
		if(boundsCheck(nx,ny)){
			if(board->Colors[nx][ny] != turn){
				int move[5] = {0,start,nx*8+ny,board->Types[nx][ny],0};
				createNormalMove(board,turn,size,move,check);
			}
		}
	}
}

void getBishopMoves(Board * board, int turn, int * size, int x, int y, int check){
	int i, nx, ny, start = x * 8 + y;
	for(i = 0; i < 4; i++){
		nx = x;
		ny = y;
		while(1){
			nx += MOVE_MAP_DIAGONAL[i][0];
			ny += MOVE_MAP_DIAGONAL[i][1];
		
			if(boundsCheck(nx,ny) && board->Colors[nx][ny] != turn){
				int move[5] = {0,start,nx*8+ny,board->Types[nx][ny],0};
				createNormalMove(board,turn,size,move,check);
				if(board->Types[nx][ny] != EMPTY)
					break;
			}else
				break;
		}
	}
}

void getRookMoves(Board * board, int turn, int * size, int x, int y, int check){
	int i, nx, ny, start = x * 8 + y;
	for(i = 0; i < 4; i++){
		nx = x;
		ny = y;
		while(1){
			nx += MOVE_MAP_STRAIGHT[i][0];
			ny += MOVE_MAP_STRAIGHT[i][1];
		
			if(boundsCheck(nx,ny) && board->Colors[nx][ny] != turn){
				int move[5] = {0,start,nx*8+ny,board->Types[nx][ny],0};
				createNormalMove(board,turn,size,move,check);
				if(board->Types[nx][ny] != EMPTY)
					break;
			}else
				break;
		}
	}
}

void getQueenMoves(Board * board, int turn, int * size, int x, int y, int check){
	getBishopMoves(board, turn, size, x, y, check);
	getRookMoves(board, turn, size, x, y, check);
}

void getKingMoves(Board * board, int turn, int * size, int x, int y, int check){
	int moveWasValid[8] = {0,0,0,0,0,0,0,0};
	int ls, i, nx, ny, start = x * 8 + y;
	for(i = 0; i < 8; i++){
		nx = x + MOVE_MAP_ALL[i][0];
		ny = y + MOVE_MAP_ALL[i][1];
		
		if(boundsCheck(nx,ny)){
			if(board->Colors[nx][ny] != turn){
				int move[5] = {0,start,nx*8+ny,board->Types[nx][ny],0};
				ls = *size;
				createNormalMove(board,turn,size,move,check);
				if (*size != ls)
					moveWasValid[i] = 1;
			}
		}
	}
	
	if (!board->Castled[turn] && validateMove(board,turn)){
		if (board->ValidCastles[turn][0] && moveWasValid[7]){
			if (TYPES[start-3] == ROOK && TYPES[start-1] == EMPTY && TYPES[start-2] == EMPTY){
				int move[5] = {1,start,start-2,start-3,start-1};
				createCastleMove(board,turn,size,move,check);
			}
		}
		if (board->ValidCastles[turn][1] && moveWasValid[6]){
			if (TYPES[start+4] == ROOK && TYPES[start+1] == EMPTY && TYPES[start+2] == EMPTY && TYPES[start+3] == EMPTY){
				int move[5] = {1,start,start+2,start+4,start+1};
				createCastleMove(board,turn,size,move,check);
			}
		}
	}
}

int validateMove(Board * board, int turn){
	return 1;
}

void createNormalMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyNormalMove(board,move);
		if (validateMove(board,turn))
			memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertNormalMove(board,move);	
	} else
		memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createCastleMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyCastleMove(board,move);
		if (validateMove(board,turn))
			memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertCastleMove(board,move);	
	} else
		memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createPromotionMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyPromotionMove(board,move);
		if (validateMove(board,turn))
			memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertPromotionMove(board,move);	
	} else
		memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createEnpassMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyEnpassMove(board,move);
		if (validateMove(board,turn))
			memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertEnpassMove(board,move);	
	} else
		memcpy(MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void applyNormalMove(Board * board, int * move){
	TYPES[move[2]] = TYPES[move[1]];
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	if(TYPES[move[2]] == KING)
		board->KingLocations[COLORS[move[2]]] = move[2];
		
	if(move[4]){
		if(move[4] == 1)
			board->ValidCastles[COLORS[move[2]]][0] = 0;
		if(move[4] == 2)
			board->ValidCastles[COLORS[move[2]]][1] = 0;
	}
}

void applyCastleMove(Board * board, int * move){
	TYPES[move[2]] = KING;
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	TYPES[move[4]] = ROOK;
	COLORS[move[4]] = COLORS[move[3]];
	
	TYPES[move[3]] = EMPTY;
	COLORS[move[3]] = EMPTY;
	
	board->KingLocations[COLORS[move[2]]] = move[2];
	board->Castled[COLORS[move[2]]] = 1;
}

void applyPromotionMove(Board * board, int * move){
	TYPES[move[2]] = move[4];
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
}

void applyEnpassMove(Board * board, int * move){
	TYPES[move[2]] = PAWN;
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	TYPES[move[3]] = EMPTY;
	COLORS[move[3]] = EMPTY;

}

void revertNormalMove(Board * board, int * move){
	TYPES[move[1]] = TYPES[move[2]];
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = move[3];
	COLORS[move[2]] = move[3] != EMPTY ? !COLORS[move[1]] : EMPTY;
	
	if(TYPES[move[1]] == KING)
		board->KingLocations[COLORS[move[1]]] = move[1];
		
	if(move[4]){
		if(move[4] == 1)
			board->ValidCastles[COLORS[move[1]]][0] = 1;
		if(move[4] == 2)
			board->ValidCastles[COLORS[move[1]]][1] = 1;
	}
}

void revertCastleMove(Board * board, int * move){
	TYPES[move[1]] = KING;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = EMPTY;
	COLORS[move[2]] = EMPTY;
	
	TYPES[move[3]] = ROOK;
	COLORS[move[3]] = COLORS[move[4]];
	
	TYPES[move[4]] = EMPTY;
	COLORS[move[4]] = EMPTY;
	
	board->KingLocations[COLORS[move[2]]] = move[1];
	board->Castled[COLORS[move[2]]] = 0;
}

void revertPromotionMove(Board * board, int * move){
	TYPES[move[1]] = PAWN;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = move[3];
	COLORS[move[2]] = move[3] != EMPTY ? !COLORS[move[1]] : EMPTY;
}

void revertEnpassMove(Board * board, int * move){
	TYPES[move[1]] = PAWN;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = EMPTY;
	COLORS[move[2]] = EMPTY;
	
	TYPES[move[3]] = PAWN;
	COLORS[move[3]] = !COLORS[move[1]];
}

