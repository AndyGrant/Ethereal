#ifndef __MOVEGEN_H
#define __MOVEGEN_H

#include "Board.h"
#include "Move.h"

#define boundsCheck(x,y) ((x >= 0 && x < 8 && y >= 0 && y < 8))

extern void (*GetPieceMoves[6])(Board *, int, int *, int, int, int);

static int MOVE_MAP_KNIGHT[8][2] = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};
static int MOVE_MAP_DIAGONAL[4][2] = {{1,1},{-1,1},{-1,-1},{1,-1}};
static int MOVE_MAP_STRAIGHT[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
static int MOVE_MAP_ALL[8][2] = {{1,1},{-1,1},{-1,-1},{1,-1},{1,0},{-1,0},{0,1},{0,-1}};

static int ATTACK_DIRECTION_MAP[6][8] = {
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{0,0,0,0,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
};

int * getAllMoves(Board * board, int turn, int * size);
void getPawnMoves(Board * board, int turn, int * size, int x, int y, int check);
void getKnightMoves(Board * board, int turn, int * size, int x, int y, int check);
void getBishopMoves(Board * board, int turn, int * size, int x, int y, int check);
void getRookMoves(Board * board, int turn, int * size, int x, int y, int check);
void getQueenMoves(Board * board, int turn, int * size, int x, int y, int check);
void getKingMoves(Board * board, int turn, int * size, int x, int y, int check);

int validateMove(Board * board, int turn);

void pruneCheckValidations(Board * board, int turn);
void fillDirection(Board * board, int turn, int move);

#endif