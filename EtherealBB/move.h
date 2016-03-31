#ifndef _MOVE_H
#define _MOVE_H

#include <stdint.h> // For uint16_t
#include "types.h" // For Board & Undo

/* Moves are stored in uint16_t variables
 * Usage of bits : 
 *
 *   [15 14 13 12] [11 10 9 8 7 6] [5 4 3 2 1 0]
 *   [ Move Type ] [  To Square  ] [From Square]
*/

/* Taken from Fruit v2.1 */
#define NoneMove ( 0) // HACK: a1a1 cannot be a legal move
#define NullMove (11) // HACK: a1d2 cannot be a legal move

#define NormalMove		(0 << 12)
#define CastleMove		(1 << 12)
#define EnpassMove		(2 << 12)
#define PromotionMove	(3 << 12)

#define PromoteToKnight (0 << 14)
#define PromoteToBishop (1 << 14)
#define PromoteToRook	(2 << 14)
#define PromoteToQueen	(3 << 14)

#define MOVE_FROM(move)			(((move) >> 0) & 63)
#define MOVE_TO(move)			(((move) >> 6) & 63)
#define MOVE_TYPE(move)			((move) & (3 << 12))
#define MOVE_PROMO_TYPE(move)	((move)	& (3 << 14))
#define MOVE_MAKE(from,to,flag)	((from) | ((to) << 6) | (flag))

void apply_move(Board * board, uint16_t move, Undo * undo);
void revert_move(Board * board, uint16_t move, Undo * undo);
void print_move(uint16_t move);

#endif