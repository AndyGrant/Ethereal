#ifndef Moves
#define Moves

#include "board.h"
#include "types.h"
#include "engine.h"

void ApplyNoFlags(Board * board, Move * move, int turn);
void RevertNoFlags(Board * board, Move * move, int turn);

#endif