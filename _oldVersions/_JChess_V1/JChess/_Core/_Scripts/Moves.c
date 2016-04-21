#include "Engine.h"
#include "Moves.h"

#include <stdio.h>

void createNormalMove(Board * b, int * moves, int * moves_found, int * move, int turn, int eval_check){
	int *types = *(b->types);
	int *colors = *(b->colors);
	int *moved = *(b->moved);

	int start = (move[1] * 8) + move[2];
	int end = (move[3] * 8) + move[4];	
	int start_index = *moves_found * 7;
	
	moves[start_index] = move[0];
	moves[start_index+1] = start;
	moves[start_index+2] = end;
	moves[start_index+3] = types[end];
	moves[start_index+4] = colors[end];
	moves[start_index+5] = moved[end];
	moves[start_index+6] = moved[start];
	
	if(eval_check){
		applyNormalMove(b, moves + start_index);
		checkMove(b,moves_found,turn);
		revertNormalMove(b, moves + start_index);
	}
	else
		*moves_found += 1;
}

void createCastleMove(Board * b, int * moves, int * moves_found, int * move, int turn){
	int start = (move[1] << 3) + move[2];
	int end = (move[3] << 3) + move[4];
	int rookStart = (move[5] << 3) + move[6];
	int rookEnd = (move[7] << 3) + move[8];
	int start_index = *moves_found * 7;
	
	moves[start_index] = 1;
	moves[start_index+1] = start;
	moves[start_index+2] = end;
	moves[start_index+3] = rookStart;
	moves[start_index+4] = rookEnd;
	moves[start_index+5] = 0;
	moves[start_index+6] = 0;
	
	applyCastleMove(b,moves + start_index);
	checkMove(b,moves_found,turn);
	revertCastleMove(b,moves + start_index);
}

void createPromotionMove(Board * b, int * moves, int * moves_found, int * move, int turn, int eval_check){
	int *types = *(b->types);
	int *colors = *(b->colors);
	int *moved = *(b->moved);
	
	int start = (move[1] << 3) + move[2];
	int end = (move[3] << 3) + move[4];
	int start_index = *moves_found * 7;
	
	moves[start_index] = 2;
	moves[start_index+1] = start;
	moves[start_index+2] = end;
	moves[start_index+3] = types[end];
	moves[start_index+4] = colors[end];
	moves[start_index+5] = moved[end];
	moves[start_index+6] = move[5];
	
	if(eval_check){
		applyPromotionMove(b,moves + start_index);
		checkMove(b,moves_found,turn);
		revertPromotionMove(b,moves + start_index);
	}
	else
		*moves_found += 1;
}

void createEnpassMove(Board * b, int * moves, int * moves_found, int * move, int turn, int eval_check){
	
	int start = (move[1] << 3) + move[2];
	int end = (move[3] << 3) + move[4];
	int pass = (move[5] << 3) + move[6];
	int start_index = *moves_found * 7;

	moves[start_index] = 3;
	moves[start_index+1] = start;
	moves[start_index+2] = end;
	moves[start_index+3] = pass;
	
	if(eval_check){
		applyEnpassMove(b,moves + start_index);
		checkMove(b,moves_found,turn);
		revertEnpassMove(b,moves + start_index);
	}
	else
		*moves_found += 1;
	
}

void applyNormalMove(Board * b, int * move){
	int * types = *(b->types);
	int * colors = *(b->colors);
	int * moved = *(b->moved);
	
	types[move[2]] = types[move[1]];
	colors[move[2]] = colors[move[1]];
	moved[move[2]] = 0;
	
	types[move[1]] = 9;
	colors[move[1]] = 9;
	moved[move[1]] = 9;
	
	if (types[move[2]] == 5)
		b->kingLocations[colors[move[2]]] = move[2];
	
}

void applyCastleMove(Board * b, int * move){
	int * types = *(b->types);
	int * colors = *(b->colors);
	int * moved = *(b->moved);
	
	b->kingLocations[colors[move[1]]] = move[2];
	
	types[move[2]] = types[move[1]];
	colors[move[2]] = colors[move[1]];
	moved[move[2]] = 0;
	
	types[move[1]] = 9;
	colors[move[1]] = 9;
	moved[move[1]] = 9;
	
	types[move[4]] = types[move[3]];
	colors[move[4]] = colors[move[3]];
	moved[move[4]] = 0;
	
	types[move[3]] = 9;
	colors[move[3]] = 9;
	moved[move[3]] = 9;
}

void applyPromotionMove(Board * b, int * move){
	int * types = *(b->types);
	int * colors = *(b->colors);
	int * moved = *(b->moved);
	
	types[move[2]] = move[6];
	colors[move[2]] = colors[move[1]];
	moved[move[2]] = 0;
	
	types[move[1]] = 9;
	colors[move[1]] = 9;
	moved[move[1]] = 9;
}

void applyEnpassMove(Board * b, int * move){
	int * types = *(b->types);
	int * colors = *(b->colors);
	int * moved = *(b->moved);
	
	types[move[2]] = 0;
	colors[move[2]] = colors[move[1]];
	moved[move[2]] = 0;
	
	types[move[1]] = 9;
	colors[move[1]] = 9;
	moved[move[1]] = 9;
	
	types[move[3]] = 9;
	colors[move[3]] = 9;
	moved[move[3]] = 9;
}

void revertNormalMove(Board * b, int * move){
	int * types = *(b->types);
	int * colors = *(b->colors);
	int * moved = *(b->moved);

	types[move[1]] = types[move[2]];
	colors[move[1]] = colors[move[2]];
	moved[move[1]] = move[6];
	
	types[move[2]] = move[3];
	colors[move[2]] = move[4];
	moved[move[2]] = move[5];
	
	if (types[move[1]] == 5)
		b->kingLocations[colors[move[1]]] = move[1];
}

void revertCastleMove(Board * b, int * move){
	int * types = *(b->types);
	int * colors = *(b->colors);
	int * moved = *(b->moved);
	
	b->kingLocations[colors[move[2]]] = move[1];
	
	types[move[1]] = types[move[2]];
	colors[move[1]] = colors[move[2]];
	moved[move[1]] = 1;
	
	types[move[2]] = 9;
	colors[move[2]] = 9;
	moved[move[2]] = 1;
	
	types[move[3]] = types[move[4]];
	colors[move[3]] = colors[move[4]];
	moved[move[3]] = 1;
	
	types[move[4]] = 9;
	colors[move[4]] = 9;
	moved[move[4]] = 1;
}

void revertPromotionMove(Board * b , int * move){
	int * types = *(b->types);
	int * colors = *(b->colors);
	int * moved = *(b->moved);
	
	types[move[1]] = 0;
	colors[move[1]] = colors[move[2]];
	moved[move[1]] = 0;
	
	types[move[2]] = move[3];
	colors[move[2]] = move[4];
	moved[move[2]] = move[5];
}

void revertEnpassMove(Board * b, int * move){
	int * types = *(b->types);
	int * colors = *(b->colors);
	int * moved = *(b->moved);
	
	types[move[1]] = 0;
	colors[move[1]] = colors[move[2]];
	moved[move[1]] = 0;
	
	types[move[2]] = 9;
	colors[move[2]] = 9;
	moved[move[2]] = 9;
	
	types[move[3]] = 0;
	colors[move[3]] = !colors[move[1]];
	moved[move[3]] = 0;
}

void applyGenericMove(Board * b, int * move){
	int type = move[0];
	if (type == 0 || type == 4)
		applyNormalMove(b,move);
	else if (type == 1)
		applyCastleMove(b,move);
	else if (type == 2)
		applyPromotionMove(b,move);
	else if (type == 3)
		applyEnpassMove(b,move);
}

void revertGenericMove(Board * b, int * move){
	int type = move[0];
	if (type == 0 || type == 4)
		revertNormalMove(b,move);
	else if (type == 1)
		revertCastleMove(b,move);
	else if (type == 2)
		revertPromotionMove(b,move);
	else if (type == 3)
		revertEnpassMove(b,move);
}