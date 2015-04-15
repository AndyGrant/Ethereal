#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Engine.h"

int move_map_knight[8][2] = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};
int move_map_bishop[4][2] = {{1,1},{-1,1},{-1,-1},{1,-1}};
int move_map_rook[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
int move_map_queen[8][2] = {{1,1},{-1,1},{-1,-1},{1,-1},{1,0},{-1,0},{0,1},{0,-1}};
int move_map_king[8][2] = {{1,1},{-1,1},{-1,-1},{1,-1},{1,0},{-1,0},{0,1},{0,-1}};

int DIRECTION_MAPPINGS[6][8] = {
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{0,0,0,0,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
};

int WHITE = 0;
int BLACK = 1;

int PAWN = 0;
int KNIGHT = 1;
int BISHOP = 2;
int ROOK = 3;
int QUEEN = 4;
int KING = 5;
int EMPTY = 9;

int SIZE_OF_MOVE = 7;

/*
 * Function : printBoard
 * ---------------------
 *	Print types, colours and moved of a Board structure
 *
 *	Arguments:
 *		board : Board structure pointer
 */
void printBoard(Board * board){
	int row,col;
	for(row = 0; row < 8; row++){
		for(col = 0; col < 8; col++)
			printf("%d ",board->types[row][col]);
		printf("\n");
	}
	printf("\n\n");
	
	for(row = 0; row < 8; row++){
		for(col = 0; col < 8; col++)
			printf("%d ",board->colors[row][col]);
		printf("\n");
	}
	printf("\n\n");
	
	for(row = 0; row < 8; row++){
		for(col = 0; col < 8; col++)
			printf("%d ",board->moved[row][col]);
		printf("\n");
	}
	printf("\n\n");
}


/*
 * Function : createBoard
 * ----------------------
 *	Create Board structure from 2d integer array
 * 
 * 	Arguments:
 *		board : encoded version for board, integer[8][8]
 *  
 * 	Return:
 * 		Board Structure Pointer
 */
Board * createBoard(int board[8][8]){
	
	static Board b;
	int x, y;
	for(x = 0; x < 8; x++){
		for(y = 0; y < 8; y++){
			b.types[x][y] = board[x][y] / 10;
			b.colors[x][y] = board[x][y] % 10;
			if (board[x][y] == 99)
				b.moved[x][y] = 9;
			else 
				b.moved[x][y] = 1;
		}
	}
	
	for(x = 0; x < 8; x++)
		for(y = 0; y < 8; y++)
			if (b.types[x][y] == 0 && ((x != 6 && b.colors[x][y] == 0) || (x!=1 && b.colors[x][y] == 1)))
				b.moved[x][y] = 0;

	for(x = 0; x < 8;  x++){
		for(y = 0; y < 8; y++){
			if (b.types[x][y] == 5){
	
				if (b.colors[x][y] == 0){
					b.kingLocations[0] = (x * 8) + y;
					if (b.kingLocations[0] != 60)
						b.moved[x][y] = 0;
				}
				
				if (b.colors[x][y] == 1){
					b.kingLocations[1] = (x * 8) + y;
					if (b.kingLocations[1] != 4)
						b.moved[x][y] = 0;
				}
			}
		}
	}
	
	return &b;
}


/*
 * Function : findAllValidMoves
 * ----------------------------
 * 	Return integer array of sets of 7 that define each move that  
 *		can currently be made on the board
 *
 * 	Arguments:
 * 		b : Board structure pointer
 *		turn : WHITE or BLACK
 * 		moves_found : integer pointer for moves found
 * 		last_move : integer array for last move
 */
int * findAllValidMoves(Board * b, int turn, int * moves_found, int * last_move){
	int moves[1400];
	int row,col,i;	
	
	int eval_check[8][8];
	determineCheckValidations(eval_check, b, turn);
	
	
	for(row = 0; row < 8; row++){
		for(col = 0; col < 8; col++){
			if (b->colors[row][col] == turn){
				int type = b->types[row][col];
				if (type == PAWN)
					findPawnMoves(b,moves,moves_found,turn,row,col,last_move,eval_check[row][col]);
				else if (type == KNIGHT)
					findMappedNoIters(b,moves,moves_found,turn,row,col,*move_map_knight, 8, eval_check[row][col]);
				else if (type == BISHOP)
					findMappedIters(b,moves,moves_found,turn,row,col,*move_map_bishop, 4, eval_check[row][col]);
				else if (type == ROOK)
					findMappedIters(b,moves,moves_found,turn,row,col,*move_map_rook, 4, eval_check[row][col]);
				else if (type == QUEEN)
					findMappedIters(b,moves,moves_found,turn,row,col,*move_map_queen, 8, eval_check[row][col]);
				else if (type == KING){
					findMappedNoIters(b,moves,moves_found,turn,row,col,*move_map_king, 8, eval_check[row][col]);
					findCastles(b,moves,moves_found,turn,row,col);
				}
			}
		}
	}
	
	int * moves_final = malloc(SIZE_OF_MOVE * sizeof(int) * *moves_found);
	int * moves_final_p = moves_final;
	int * moves_p = moves;
	
	for(i = 0; i < *moves_found * SIZE_OF_MOVE; i++){
		*moves_final_p = *moves_p;
		moves_p++;
		moves_final_p++;
	}

	return moves_final;
}


/*
 * Function : findPawnMoves
 * ------------------------
 * 	Fill integer array with moves from a pawn
 *
 *	Arguments: 
 * 		b : Board structure pointer
 * 		moves : integer array of moves
 * 		moves_found : integer pointer to moves size
 * 		turn : WHITE or BLACK
 * 		x : x location in board arrays
 * 		y : y location in board arrays
 * 		last_move : integer array of last move made
 *		eval_check : integer determining need to validate move
 */
void findPawnMoves(Board * b, int * moves, int * moves_found, int turn, int x, int y, int * last_move, int eval_check){
	int dir;
	if (b->colors[x][y] == WHITE)
		dir = -1;
	else 
		dir = 1;
		
	int pt;
	
	// Forward One / Two
	if (b->types[x+dir][y] == EMPTY){
		if (x+dir == 0 || x+dir == 7){
			for(pt = QUEEN; pt != PAWN; pt--){
				int move[9] = {2,x,y,x+dir,y,pt};
				createPromotionMove(b,moves,moves_found,move,turn,eval_check);
			}
		}
		else{
			int move[9] = {0,x,y,x+dir,y,0,0,0,0};
			createNormalMove(b,moves,moves_found,move,turn,eval_check);
			
			if (b->moved[x][y] == 1 && b->types[x+dir+dir][y] == 9){
				int move[9] = {4,x,y,x+dir+dir,y,0,0,0,0};
				createNormalMove(b,moves,moves_found,move,turn,eval_check);
			}		
		}
	}
	
	// En Passant
	if (last_move[0] == 4 && abs((y - last_move[2] % 8)) == 1 && x == 3 + b->colors[x][y]){
		if(b->colors[last_move[2]/8][last_move[2]%8] != turn){
			int move[9] = {3,x,y,x+dir,last_move[2] % 8,last_move[2] / 8,last_move[2] % 8,0,0};
			createEnpassMove(b,moves,moves_found,move,turn,eval_check);
		}
	}
	
	// Capture King Side
	if (y + 1 < 8 && b->types[x+dir][y+1] != EMPTY && b->colors[x+dir][y+1] != turn){
		if (x+dir == 0 || x+dir == 7){
			for(pt = QUEEN; pt != PAWN; pt--){
				int move[9] = {2,x,y,x+dir,y+1,pt};
				createPromotionMove(b,moves,moves_found,move,turn,eval_check);
			}
		}
		else{
			int move[9] = {0,x,y,x+dir,y+1,0,0,0,0};
			createNormalMove(b,moves,moves_found,move,turn,eval_check);
		}
	}
	
	//Capture Queen Side
	if (y - 1 >= 0 && b->types[x+dir][y-1] != EMPTY && b->colors[x+dir][y-1] != turn){
		if (x+dir == 0 || x+dir == 7){
			for(pt = QUEEN; pt != PAWN; pt--){
				int move[9] = {2,x,y,x+dir,y-1,pt};
				createPromotionMove(b,moves,moves_found,move,turn,eval_check);
			}
		}
		else{
			int move[9] = {0,x,y,x+dir,y-1,0,0,0,0};
			createNormalMove(b,moves,moves_found,move,turn,eval_check);
		}
	}
}


/*
 * Function : findMappedIters
 * ------------------------
 * 	Find moves for bishops, rooks and queens by 
 * 		iterating over an array of integers that define
 *		how the pieces are able to move
 *
 *	Arguments: 
 * 		b : Board structure pointer
 * 		moves : integer array of moves
 * 		moves_found : integer pointer to moves size
 * 		turn : WHITE or BLACK
 * 		x : x location in board arrays
 * 		y : y location in board arrays
 * 		map : integer array of move mappings
 * 		map_size : length of map
 *		eval_check : integer determining need to validate move
 */
void findMappedIters(Board * b, int * moves, int *moves_found, int turn, int x, int y, int * map, int map_size, int eval_check){
	int mapiter, newX, newY;

	for(mapiter = map_size; mapiter > 0; mapiter--){
		newX = x;
		newY = y;
		while(1){
			newX += *map;
			newY += *(map+1);
			if (newX < 0 || newY < 0 || newX > 7 || newY > 7)
				break;
			
			if (b->types[newX][newY] == EMPTY){
				int data[5] = {0,x,y,newX,newY};
				createNormalMove(b,moves,moves_found,data,turn,eval_check);
			}
			else if (b->colors[newX][newY] != turn){
				int data[5] = {0,x,y,newX,newY};
				createNormalMove(b,moves,moves_found,data,turn,eval_check);
				break;
			}
			else
				break;
		}
		map += 2;
	}
}


/*
 * Function : findMappedNoIters
 * --------------------------	
 *	Find moves for knights and kings by using each move map
 *	
 *	Arguments: 
 * 		b : Board structure pointer
 * 		moves : integer array of moves
 * 		moves_found : integer pointer to moves size
 * 		turn : WHITE or BLACK
 * 		x : x location in board arrays
 * 		y : y location in board arrays
 * 		map : integer array of move mappings
 * 		map_size : length of map
 *		eval_check : integer determining need to validate move
 */
void findMappedNoIters(Board * b, int * moves, int * moves_found, int turn, int x, int y, int * map, int map_size, int eval_check){
	int mapiter, newX, newY;
	for(mapiter = map_size; mapiter > 0; mapiter--){
		newX = x + *map; 
		map+=1;
		newY = y + *map; 
		map+=1;
		if (newX < 0 || newY < 0 || newX > 7 || newY > 7)
			continue;
		
		if (b->types[newX][newY] == EMPTY){
			int data[5] = {0,x,y,newX,newY};
			createNormalMove(b,moves,moves_found,data,turn,eval_check);
		}
		else if (b->colors[newX][newY] != turn){
			int data[5] = {0,x,y,newX,newY};
			createNormalMove(b,moves,moves_found,data,turn,eval_check);
		}
	}
}


/*
 * Function : findCastles
 * ----------------------
 * 	Add castle moves to the moves integer array
 *	
 *	Arguments: 
 * 		b : Board structure pointer
 * 		moves : integer array of moves
 * 		moves_found : integer pointer to moves size
 * 		turn : WHITE or BLACK
 * 		x : x location in board arrays
 * 		y : y location in board arrays
 */
void findCastles(Board * b, int * moves, int * moves_found, int turn, int x, int y){
	// Moved kings cannot castle
	if (b->moved[x][y] == 0)
		return;
	
	// Cannot castle out of check
	int isChecked = 0;
	checkMove(b,&isChecked,turn);
	if (isChecked == 0)
		return;
	
	// Validate Castle Cross-overs
	int leftCastleIsValid = 1;
	int rightCastleIsValid = 1;
	int total_moves = *moves_found;
	int i, j;
	int king_moves = 0;
	int castle_moves = 0;
	
	for(i = 0; i < total_moves; i++){
		if (moves[i * 7 +1] == b->kingLocations[turn]){
			king_moves += 1;
			if (moves[i * 7 + 2] - moves[i * 7 + 1] == -1)
				leftCastleIsValid = 0;
			if (moves[i * 7 + 2] - moves[i * 7 + 1] == 1)
				rightCastleIsValid = 0;
		}
	}
	
	// Create right castle
	if (leftCastleIsValid == 0){
		if (b->types[x][0] == ROOK && b->moved[x][0] == 1){
			if (b->types[x][3] == EMPTY && b->types[x][2] == EMPTY && b->types[x][1] == EMPTY){
				int valid = *moves_found;
				int data[9] = {1,x,y,x,y-2,x,0,x,3};
				createCastleMove(b,moves,moves_found,data,turn);
				if (valid != *moves_found)
					castle_moves += 1;
			}	
		}
	}
	
	// Create left castle
	if (rightCastleIsValid == 0)
		if (b->types[x][7] == ROOK && b->moved[x][7] == 1)
			if (b->types[x][5] == EMPTY && b->types[x][6] == EMPTY){
				int valid = *moves_found;
				int data[9] = {1,x,y,x,y+2,x,7,x,5};
				createCastleMove(b,moves,moves_found,data,turn);
				if (valid != *moves_found)
					castle_moves += 1;
			}
				
	// No castles means nothing to be sorted
	if (castle_moves == 0)
		return;
	
	// Sort castles to front of array
	int temp_size = 7 * (king_moves + castle_moves);
	int * temp = malloc(4 * temp_size);
	int startOfKingMoves = 7 * (*moves_found - king_moves - castle_moves);
	
	for(i = 0, j = startOfKingMoves; i < temp_size; i++, j++)
		temp[i] = moves[j];
	
	for(i = (king_moves) * 7, j = startOfKingMoves; i < temp_size; i++, j++)
		moves[j] = temp[i];
	
	for(i = 0, j = startOfKingMoves + (7 * castle_moves); i < temp_size - (7 * castle_moves);  i++, j++)
		moves[j] = temp[i];
	
	free(temp);

}


/*
 * Function : checkMove
 * --------------------
 * 	If move is valid, increments the moves_found counter
 *
 *	Arguments: 
 *		b : Board structure pointer
 * 		moves_found : integer pointer to moves size
 * 		turn : WHITE or BLACK	
*/
void checkMove(Board *b, int * moves_found, int turn){
	int x, y, kingX, kingY,i;
	kingX = b->kingLocations[turn] / 8;
	kingY = b->kingLocations[turn] % 8;
		
	int dir = -1;
	if (turn == 1)
		dir = 1;
	
	// Pawn
	if ((kingX != 0 && dir == -1) || (kingX != 7 && dir == 1)){
		if (kingY != 7 && b->types[kingX+dir][kingY + 1] == 0 && b->colors[kingX+dir][kingY+1] != turn)
			return;	
		if (kingY != 0 && b->types[kingX+dir][kingY - 1] == 0 && b->colors[kingX+dir][kingY-1] != turn)
			return;
	}
	
	
	// Knight
	for(i = 0; i < 8; i += 1){
		x = kingX + move_map_knight[i][0];
		y = kingY + move_map_knight[i][1];
		
		if (x < 0 || y < 0 || x > 7 || y > 7)
			continue;
		if (b->types[x][y] == 1 && b->colors[x][y] != turn)
			return;
	}
	
	
	// Bishop
	for(i = 0; i < 4; i++){
		x = kingX;
		y = kingY;
		while(1){
			x += move_map_bishop[i][0];
			y += move_map_bishop[i][1];
			
			if (x < 0 || y < 0 || x > 7 || y > 7)
				break;
			else if (b->types[x][y] == 9)
				continue;
			else if ((b->types[x][y] == 2 || b->types[x][y] == 4) && b->colors[x][y] != turn)
				return;
			else
				break;
		}
	}
	
	
	// Rook
	for(i = 0; i < 4; i++){
		x = kingX;
		y = kingY;
		while(1){
			x += move_map_rook[i][0];
			y += move_map_rook[i][1];
			
			if (x < 0 || y < 0 || x > 7 || y > 7)
				break;
			else if (b->types[x][y] == 9)
				continue;
			else if ((b->types[x][y] >= 3 && b->types[x][y] != 5) && b->colors[x][y] != turn)
				return;
			else
				break;
		}
	}
	
	
	// King
	for(i = 0; i < 8; i++){
		x = kingX + move_map_king[i][0];
		y = kingY + move_map_king[i][1];
		
		if (x < 0 || y < 0 || x > 7 || y > 7)
			continue;
		if (b->types[x][y] == 5 && b->colors[x][y] != turn)
			return;
	}
	
	*moves_found += 1;
	return;
}


/*
 * Function : determineCheckValidations
 * ------------------------------------
 * 	Build a two dimensional integer array that can determine 
 * 		with no false positives when a piece does not need
 * 		to have it's moves validated for check. 
 *
 *	Arguments:
 *		eval_check : two dimensional array to fill with bits
 *		board : Board structure pointer
 *		turn : WHITE or BLACK
 */
void determineCheckValidations(int eval_check[8][8],Board * board, int turn){
	int not_in_check = 0;
	checkMove(board, &not_in_check, turn);
	
	int row, col, m;
	
	if(not_in_check){
		for(row = 0; row < 8; row++)
			for(col = 0; col < 8; col++)
				eval_check[row][col] = 0;
			
		
		int kx = board->kingLocations[turn] / 8;
		int ky = board->kingLocations[turn] % 8;
		
		for(m = 0; m < 8; m++)
			checkDirection(eval_check,board,turn,kx,ky,m);
		
		eval_check[kx][ky] = 1;
		
	}
	else{
		for(row = 0; row < 8; row++)
			for(col = 0; col < 8; col++)
				eval_check[row][col] = 1;
	}
	
	
}


/*
 * Function : checkDirection
 * ------------------------------------
 * 	Build a two dimensional integer array that can determine 
 * 		with no false positives when a piece does not need
 * 		to have it's moves validated for check. 
 *
 *	Arguments:
 *		eval_check : two dimensional array to fill with bits
 *		board : Board structure pointer
 *		turn : WHITE or BLACK
 */
void checkDirection(int eval_check[8][8], Board * board, int turn, int kx, int ky, int move){
	int x = kx;
	int y = ky;
	int blocks = 0;
	
	while(1){
		x += move_map_king[move][0];
		y += move_map_king[move][1];
		
		if (x < 0 || y < 0 || x > 7 || y > 7)
			return;
				
		else if (board->colors[x][y] == turn){
			if (blocks == 1)
				return;
			else
				blocks = 1;
		}
		
		else if (board->colors[x][y] != turn && board->types[x][y] != EMPTY){
			if (DIRECTION_MAPPINGS[board->types[x][y]][move] == 0)
				return;
			else{
				fillDirection(eval_check,kx,ky,move);
				return;
			}
			
		}			
	}
}

void fillDirection(int eval_check[8][8], int x, int y, int move){
	while(1){
		x += move_map_king[move][0];;
		y += move_map_king[move][1];;
		
		if (x < 0 || y < 0 || x > 7 || y > 7)
			return;
			
		eval_check[x][y] = 1;
	}
}
