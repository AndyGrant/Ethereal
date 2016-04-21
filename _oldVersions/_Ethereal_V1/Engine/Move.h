#ifndef __MOVE_H
#define __MOVE_H

#include "Board.h"

extern void (*ApplyTypes[5])(Board *, int *);
extern void (*RevertTypes[5])(Board *, int *);

#define ApplyMove(b,m) ((*ApplyTypes[*m])(b,m))
#define RevertMove(b,m) ((*RevertTypes[*m])(b,m))

void createNormalMove(Board * board, int turn, int * size, int * move, int check);
void createCastleMove(Board * board, int turn, int * size, int * move, int check);
void createPromotionMove(Board * board, int turn, int * size, int * move, int check);
void createEnpassMove(Board * board, int turn, int * size, int * move, int check);

void applyNormalMove(Board * board, int * move);
void applyCastleMove(Board * board, int * move);
void applyPromotionMove(Board * board, int * move);
void applyEnpassMove(Board * board, int * move);

void revertNormalMove(Board * board, int * move);
void revertCastleMove(Board * board, int * move);
void revertPromotionMove(Board * board, int * move);
void revertEnpassMove(Board * board, int * move);

#endif
