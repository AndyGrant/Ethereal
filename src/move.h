#ifndef __MOVE_H
#define __MOVE_H

#include <stdint.h>

#include "types.h"

void applyMove(Board * board, uint16_t move, Undo * undo);
void revertMove(Board * board, uint16_t move, Undo * undo);
void printMove(uint16_t move);

#define NoneMove ( 0)
#define NullMove (11)

#define NormalMove      (0 << 12)
#define CastleMove      (1 << 12)
#define EnpassMove      (2 << 12)
#define PromotionMove   (3 << 12)

#define PromoteToKnight (0 << 14)
#define PromoteToBishop (1 << 14)
#define PromoteToRook   (2 << 14)
#define PromoteToQueen  (3 << 14)

#define MoveFrom(move)         (((move) >> 0) & 63)
#define MoveTo(move)           (((move) >> 6) & 63)
#define MoveType(move)         ((move) & (3 << 12))
#define MovePromoType(move)    ((move) & (3 << 14))
#define MoveMake(from,to,flag) ((from) | ((to) << 6) | (flag))

#endif