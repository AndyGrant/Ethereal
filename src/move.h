/*
  Ethereal is a UCI chess playing engine authored by Andrew Grant.
  <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>

  Ethereal is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Ethereal is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MOVE_H
#define _MOVE_H

#include <stdint.h>

#include "types.h"

#define NULL_MOVE (11)
#define NONE_MOVE ( 0)

#define NORMAL_MOVE    (0 << 12)
#define CASTLE_MOVE    (1 << 12)
#define ENPASS_MOVE    (2 << 12)
#define PROMOTION_MOVE (3 << 12)

#define PROMOTE_TO_KNIGHT (0 << 14)
#define PROMOTE_TO_BISHOP (1 << 14)
#define PROMOTE_TO_ROOK   (2 << 14)
#define PROMOTE_TO_QUEEN  (3 << 14)

#define KNIGHT_PROMO_MOVE (PROMOTION_MOVE | PROMOTE_TO_KNIGHT)
#define BISHOP_PROMO_MOVE (PROMOTION_MOVE | PROMOTE_TO_BISHOP)
#define ROOK_PROMO_MOVE   (PROMOTION_MOVE | PROMOTE_TO_ROOK  )
#define QUEEN_PROMO_MOVE  (PROMOTION_MOVE | PROMOTE_TO_QUEEN )

int apply(Thread *thread, Board *board, uint16_t move, int height);
void applyMove(Board* board, uint16_t move, Undo* undo);
void applyNormalMove(Board* board, uint16_t move, Undo* undo);
void applyCastleMove(Board* board, uint16_t move, Undo* undo);
void applyEnpassMove(Board* board, uint16_t move, Undo* undo);
void applyPromotionMove(Board* board, uint16_t move, Undo* undo);
void applyNullMove(Board* board, Undo* undo);

void revert(Thread *thread, Board *board, uint16_t move, int height);
void revertMove(Board* board, uint16_t move, Undo* undo);
void revertNullMove(Board* board, Undo* undo);

void moveToString(uint16_t move, char *str);

#define MoveFrom(move)         (((move) >> 0) & 63)
#define MoveTo(move)           (((move) >> 6) & 63)
#define MoveType(move)         ((move) & (3 << 12))
#define MovePromoType(move)    ((move) & (3 << 14))
#define MovePromoPiece(move)   (1 + ((move) >> 14))
#define MoveMake(from,to,flag) ((from) | ((to) << 6) | (flag))

#endif
