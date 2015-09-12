#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "Engine.h"
#include "Search.h"

/*
	Move Encodings

	NormalMove: [Move Type, Start, End, Captured Type, Breaks Castles]
	
	CastleMove: [Move Type, Start, End, Rook Start, Rook End]
	
	PromotionMove: [Move Type, Start, End, Captured Type, Promte Type]
	
	EnpassMove: [Move Type, Start, End, Enpass]

*/

const int MOVE_MAP_KNIGHT[8][2] = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};
const int MOVE_MAP_DIAGONAL[4][2] = {{1,1},{-1,1},{-1,-1},{1,-1}};
const int MOVE_MAP_STRAIGHT[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
const int MOVE_MAP_ALL[8][2] = {{1,1},{-1,1},{-1,-1},{1,-1},{1,0},{-1,0},{0,1},{0,-1}};

char base[137] = "3111214151211131010101010101010199999999999999999999999999999999999999999999999999999999999999990000000000000000301020405020103000111199";
//char base[135] = "99999999519999999999010101999999999999999999999999999999999999999999999999999999999999999999999999999999990000009999999950999999000000";
//char base[135]   = "11999999511111119999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999991099991050101010000000";

int ATTACK_DIRECTION_MAP[6][8] = {
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{0,0,0,0,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
};

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
/*
int main(){
	Board * board = createBoard(base);
	int move[5] = {0,0,0,0,0};
	board->LastMove = move;
	
	getBestMoveIndex(board, WHITE);
	/*
	time_t start = time(NULL);
	int i;
	printf("Moves Searched : %llu\n",depthSearch(board,WHITE,5,move));
	printf("Seconds Taken: %d \n\n",(int)(time(NULL)-start));

	return 0;
}*/


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
	memcpy(new->KnightMap,old->KnightMap,sizeof(old->KnightMap));
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
	
	buildKnightMap(board);
	
	board->FiftyMoveRule = 50;
	
	board->TYPES = *(board->Types);
	board->COLORS = *(board->Colors);
	
	return board;
}

void buildKnightMap(Board * board){
	int x, y, a, b;
	for(x = 0; x < 8; x++){
		for(y = 0; y < 8; y++){
			for(a = 0, b = 0; a < 8; a++){
				int nx = x + MOVE_MAP_KNIGHT[a][0];
				int ny = y + MOVE_MAP_KNIGHT[a][1];
				if(boundsCheck(nx,ny)){
					board->KnightMap[x*8+y][b] = nx * 8 + ny;
					b += 1;
				}
			}
			board->KnightMap[x*8+y][b] = -1;
		}
	}
}

int * getAllMoves(Board * board, int turn, int * size){
	pruneCheckValidations(board,turn);
	*size = 0;
	int x,y;
	for(x = 0; x < 8; x++)
		for(y = 0; y < 8; y++)
			if (board->Colors[x][y] == turn)
				(*GetPieceMoves[board->Types[x][y]])(board,turn,size,x,y,board->CHECK_VALIDATIONS[x][y]);
				
				
	return memcpy(malloc(sizeof(int) * MOVE_SIZE * *size),board->MOVES_BUFFER,sizeof(int) * MOVE_SIZE * *size);
}

void getPawnMoves(Board * board, int turn, int * size, int x, int y, int check){
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
	if (board->LastMove[0] == 4 && abs(y - (board->LastMove[2]%8)) == 1){
		if (3 + turn == x && board->COLORS[board->LastMove[2]] != turn){
			int move[5] = {3,start,board->LastMove[2]+(8*dir),board->LastMove[2],0};
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
	int start = x * 8 + y;
	int * map = board->KnightMap[start];
	while(1){
		if (*map == -1)
			return;
		if (board->COLORS[*map] != turn){
			int move[5] = {0,start,*map,board->TYPES[*map],0};
			createNormalMove(board,turn,size,move,check);
		}
		map++;
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
	int checkBreak = 0;
	if (board->Types[x][y] != ROOK)
		checkBreak = 0;
	else if(board->ValidCastles[turn][x != 0])
		checkBreak = 1 + 2*x != 0;
		
	int i, nx, ny, start = x * 8 + y;
	for(i = 0; i < 4; i++){
		nx = x;
		ny = y;
		while(1){
			nx += MOVE_MAP_STRAIGHT[i][0];
			ny += MOVE_MAP_STRAIGHT[i][1];
		
			if(boundsCheck(nx,ny) && board->Colors[nx][ny] != turn){
				int move[5] = {0,start,nx*8+ny,board->Types[nx][ny],checkBreak};
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
	int checkBreak;
	if(board->ValidCastles[turn][0]){
		if(board->ValidCastles[turn][1])
			checkBreak = 2;
		else
			checkBreak = 1;
	}
	else if(board->ValidCastles[turn][1])
		checkBreak = 3;
	else 
		checkBreak = 0;
		
	int moveWasValid[8] = {0,0,0,0,0,0,0,0};
	int ls, i, nx, ny, start = x * 8 + y;
	for(i = 0; i < 8; i++){
		nx = x + MOVE_MAP_ALL[i][0];
		ny = y + MOVE_MAP_ALL[i][1];
		
		if(boundsCheck(nx,ny)){
			if(board->Colors[nx][ny] != turn){
				int move[5] = {0,start,nx*8+ny,board->Types[nx][ny],checkBreak};
				ls = *size;
				createNormalMove(board,turn,size,move,check);
				if (*size != ls)
					moveWasValid[i] = 1;
			}
		}
	}
	
	if (board->Castled[turn] == 0 && validateMove(board,turn)){
		if (board->ValidCastles[turn][0] && moveWasValid[6]){
			if (board->TYPES[start+3] == ROOK && board->TYPES[start+1] == EMPTY && board->TYPES[start+2] == EMPTY){
				int move[5] = {1,start,start+2,start+3,start+1};
				createCastleMove(board,turn,size,move,check);
			}
		}
		if (board->ValidCastles[turn][1] && moveWasValid[7]){
			if (board->TYPES[start-4] == ROOK && board->TYPES[start-1] == EMPTY && board->TYPES[start-2] == EMPTY && board->TYPES[start-3] == EMPTY){
				int move[5] = {1,start,start-2,start-4,start-1};
				createCastleMove(board,turn,size,move,check);
			}
		}
	}
}

int validateMove(Board * board, int turn){
	int x, y, kingX, kingY,i;
	kingX = board->KingLocations[turn] / 8;
	kingY = board->KingLocations[turn] % 8;
		
	int dir = -1;
	if (turn == 1)
		dir = 1;
		
	
	if (board->PieceCount[!turn][PAWN]){
		if ((kingX != 0 && dir == -1) || (kingX != 7 && dir == 1)){
			if (kingY != 7 && board->Types[kingX+dir][kingY + 1] == 0 && board->Colors[kingX+dir][kingY+1] != turn)
				return 0;	
			if (kingY != 0 && board->Types[kingX+dir][kingY - 1] == 0 && board->Colors[kingX+dir][kingY-1] != turn)
				return 0;
		}
	}
	
	
	if (board->PieceCount[!turn][KNIGHT]){
		int * map = board->KnightMap[board->KingLocations[turn]];
		while(1){
			if (*map == -1)
				break;
			if (board->COLORS[*map] == !turn && board->TYPES[*map] == KNIGHT)
				return 0;
			map++;
		}
	}
	
	if (board->PieceCount[!turn][BISHOP] || board->PieceCount[!turn][QUEEN]){
		for(i = 0; i < 4; i++){
			x = kingX;
			y = kingY;
			while(1){
				x += MOVE_MAP_DIAGONAL[i][0];
				y += MOVE_MAP_DIAGONAL[i][1];
				
				if (x < 0 || y < 0 || x > 7 || y > 7)
					break;
				else if (board->Types[x][y] == 9)
					continue;
				else if ((board->Types[x][y] == 2 || board->Types[x][y] == 4) && board->Colors[x][y] != turn)
					return 0;
				else
					break;
			}
		}
	}
	
	if (board->PieceCount[!turn][ROOK] || board->PieceCount[!turn][QUEEN]){
		for(i = 0; i < 4; i++){
			x = kingX;
			y = kingY;
			while(1){
				x += MOVE_MAP_STRAIGHT[i][0];
				y += MOVE_MAP_STRAIGHT[i][1];
				
				if (x < 0 || y < 0 || x > 7 || y > 7)
					break;
				else if (board->Types[x][y] == 9)
					continue;
				else if ((board->Types[x][y] >= 3 && board->Types[x][y] != 5) && board->Colors[x][y] != turn)
					return 0;
				else
					break;
			}
		}
	}
	
	for(i = 0; i < 8; i++){
		x = kingX + MOVE_MAP_ALL[i][0];
		y = kingY + MOVE_MAP_ALL[i][1];
		
		if (x < 0 || y < 0 || x > 7 || y > 7)
			continue;
		if (board->Types[x][y] == 5 && board->Colors[x][y] != turn)
			return 0;
	}
	
	return 1;
}

void pruneCheckValidations(Board * board, int turn){
	if (validateMove(board,turn)){
		memset(board->CHECK_VALIDATIONS,0,sizeof(board->CHECK_VALIDATIONS));
		
		int kx = board->KingLocations[turn]/8;
		int ky = board->KingLocations[turn]%8;
		
		int i,x,y;
		for(i = 0, x = kx, y = ky; i < 8; i++, x = kx, y = ky){
		
			int blockers = 0;
			while(1){
				x += MOVE_MAP_ALL[i][0];
				y += MOVE_MAP_ALL[i][1];
				
				if (!boundsCheck(x,y))
					break;
					
				else if (board->Colors[x][y] == turn){
					if (blockers == 1)
						break;
					else
						blockers = 1;
				}
				else if(board->Types[x][y] != EMPTY){
					if (ATTACK_DIRECTION_MAP[board->Types[x][y]][i] == 0)
						break;
					else{
						fillDirection(board,turn,i);
						break;
					}
				}
			}
		}
		
		board->CHECK_VALIDATIONS[kx][ky] = 1;
		
	} else {
		memset(board->CHECK_VALIDATIONS,1,sizeof(board->CHECK_VALIDATIONS));
	}
}

void fillDirection(Board * board, int turn, int move){
	int x = board->KingLocations[turn]/8;
	int y = board->KingLocations[turn]%8;
	
	while(1){
		x += MOVE_MAP_ALL[move][0];
		y += MOVE_MAP_ALL[move][1];
		
		if (!boundsCheck(x,y))
			return;
		board->CHECK_VALIDATIONS[x][y] = 1;
	}
}

void createNormalMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyNormalMove(board,move);
		if (validateMove(board,turn))
			memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertNormalMove(board,move);	
	} else
		memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createCastleMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyCastleMove(board,move);
		if (validateMove(board,turn))
			memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertCastleMove(board,move);	
	} else
		memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createPromotionMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyPromotionMove(board,move);
		if (validateMove(board,turn))
			memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertPromotionMove(board,move);	
	} else
		memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void createEnpassMove(Board * board, int turn, int * size, int * move, int check){
	if (check){
		applyEnpassMove(board,move);
		if (validateMove(board,turn))
			memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
		revertEnpassMove(board,move);	
	} else
		memcpy(board->MOVES_BUFFER + (*size)++ * MOVE_SIZE, move, sizeof(int) * MOVE_SIZE);
}

void applyNormalMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	
	if (move[3] != EMPTY)
		board->PieceCount[!(COLORS[move[1]])][move[3]] -= 1;
	
	TYPES[move[2]] = TYPES[move[1]];
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	if(TYPES[move[2]] == KING)
		board->KingLocations[COLORS[move[2]]] = move[2];
		
	if(move[4]){
		if(move[4] <= 2)
			board->ValidCastles[COLORS[move[2]]][0] = 0;
		if(move[4] >= 2)
			board->ValidCastles[COLORS[move[2]]][1] = 0;
	}
}

void applyCastleMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
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
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	if (move[3] != EMPTY)
		board->PieceCount[!(COLORS[move[1]])][move[3]] -= 1;
	board->PieceCount[COLORS[move[1]]][move[4]] += 1;
	board->PieceCount[COLORS[move[1]]][PAWN] -= 1;
	
	TYPES[move[2]] = move[4];
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
}

void applyEnpassMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	board->PieceCount[!(COLORS[move[1]])][PAWN] -= 1;
	
	TYPES[move[2]] = PAWN;
	COLORS[move[2]] = COLORS[move[1]];
	
	TYPES[move[1]] = EMPTY;
	COLORS[move[1]] = EMPTY;
	
	TYPES[move[3]] = EMPTY;
	COLORS[move[3]] = EMPTY;

}

void revertNormalMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	if (move[3] != EMPTY)
		board->PieceCount[!(COLORS[move[2]])][move[3]] += 1;
	
	TYPES[move[1]] = TYPES[move[2]];
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = move[3];
	COLORS[move[2]] = move[3] != EMPTY ? !COLORS[move[1]] : EMPTY;
	
	if(TYPES[move[1]] == KING)
		board->KingLocations[COLORS[move[1]]] = move[1];
		
	if(move[4]){
		if(move[4] <= 2)
			board->ValidCastles[COLORS[move[1]]][0] = 1;
		if(move[4] >= 2)
			board->ValidCastles[COLORS[move[1]]][1] = 1;
	}
}

void revertCastleMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	TYPES[move[1]] = KING;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = EMPTY;
	COLORS[move[2]] = EMPTY;
	
	TYPES[move[3]] = ROOK;
	COLORS[move[3]] = COLORS[move[4]];
	
	TYPES[move[4]] = EMPTY;
	COLORS[move[4]] = EMPTY;
	
	board->KingLocations[COLORS[move[1]]] = move[1];
	board->Castled[COLORS[move[1]]] = 0;
}

void revertPromotionMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	if (move[3] != EMPTY)
		board->PieceCount[!(COLORS[move[2]])][move[3]] += 1;
	board->PieceCount[(COLORS[move[2]])][move[4]] -= 1;
	board->PieceCount[(COLORS[move[2]])][PAWN] += 1;
	
	TYPES[move[1]] = PAWN;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = move[3];
	COLORS[move[2]] = move[3] != EMPTY ? !COLORS[move[1]] : EMPTY;
}

void revertEnpassMove(Board * board, int * move){
	int * TYPES = board->TYPES;
	int * COLORS = board->COLORS;
	
	board->PieceCount[!(COLORS[move[2]])][PAWN] += 1;
	
	TYPES[move[1]] = PAWN;
	COLORS[move[1]] = COLORS[move[2]];
	
	TYPES[move[2]] = EMPTY;
	COLORS[move[2]] = EMPTY;
	
	TYPES[move[3]] = PAWN;
	COLORS[move[3]] = !COLORS[move[1]];
}

