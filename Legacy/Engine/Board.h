#ifndef __BOARD_H
#define __BOARD_H

#define WHITE 0
#define BLACK 1
#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5
#define EMPTY 9

#define MOVE_SIZE 5
#define BUFFER_SIZE 256

typedef struct Board {
  int Types[8][8];
  int Colors[8][8];
  
  int * TYPES;
  int * COLORS;
  
  int PieceCount[2][6];
  
  char CHECK_VALIDATIONS[8][8];
  
  int MOVES_BUFFER[MOVE_SIZE * BUFFER_SIZE];
  
  int KingLocations[2];
  int Castled[2];
  int ValidCastles[2][2];
  
  int FiftyMoveRule;
  int * LastMove;
} Board;

Board * copyBoard(Board * old);
Board * createBoard(char setup[137]);

#endif