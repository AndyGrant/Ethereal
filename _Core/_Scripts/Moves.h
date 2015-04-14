#ifndef Moves
#define Moves

#include "Engine.h"

void createNormalMove(Board *b, int *moves, int* moves_found, int *move, int turn);
void createCastleMove(Board *b, int *moves, int* moves_found, int *move, int turn);
void createPromotionMove(Board *b, int *moves, int* moves_found, int *move, int turn);
void createEnpassMove(Board *b, int *moves, int* moves_found, int *move, int turn);

void applyGenericMove(Board * board, int * moves);
void applyNormalMove(Board * board, int * moves);
void applyCastleMove(Board * board, int * moves);
void applyPromotionMove(Board * board, int * moves);
void applyEnpassMove(Board * board, int * moves);

void revertGenericMove(Board * board, int * moves);
void revertNormalMove(Board * board, int * moves);
void revertCastleMove(Board * board, int * moves);
void revertPromotionMove(Board * board, int * moves);
void revertEnpassMove(Board * board, int * moves);

#endif
