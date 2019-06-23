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

#pragma once

#include <stdint.h>

#include "types.h"

enum {
    NONE_MOVE = 0, NULL_MOVE = 11,

    NORMAL_MOVE = 0 << 12, CASTLE_MOVE    = 1 << 12,
    ENPASS_MOVE = 2 << 12, PROMOTION_MOVE = 3 << 12,

    CASTLE_KING_SIDE  = 0 << 14, CASTLE_QUEEN_SIDE = 1 << 14,

    PROMOTE_TO_KNIGHT = 0 << 14, PROMOTE_TO_BISHOP = 1 << 14,
    PROMOTE_TO_ROOK   = 2 << 14, PROMOTE_TO_QUEEN  = 3 << 14,

    CASTLE_KING_MOVE  = CASTLE_MOVE | CASTLE_KING_SIDE,
    CASTLE_QUEEN_MOVE = CASTLE_MOVE | CASTLE_QUEEN_SIDE,

    KNIGHT_PROMO_MOVE = PROMOTION_MOVE | PROMOTE_TO_KNIGHT,
    BISHOP_PROMO_MOVE = PROMOTION_MOVE | PROMOTE_TO_BISHOP,
    ROOK_PROMO_MOVE   = PROMOTION_MOVE | PROMOTE_TO_ROOK,
    QUEEN_PROMO_MOVE  = PROMOTION_MOVE | PROMOTE_TO_QUEEN
};

int apply(Thread *thread, Board *board, uint16_t move, int height);
void applyMove(Board *board, uint16_t move, Undo *undo);
void applyNormalMove(Board *board, uint16_t move, Undo *undo);
void applyCastleMove(Board *board, uint16_t move, Undo *undo);
void applyEnpassMove(Board *board, uint16_t move, Undo *undo);
void applyPromotionMove(Board *board, uint16_t move, Undo *undo);
void applyNullMove(Board *board, Undo *undo);

void revert(Thread *thread, Board *board, uint16_t move, int height);
void revertMove(Board *board, uint16_t move, Undo *undo);
void revertNullMove(Board *board, Undo *undo);

int moveIsTactical(Board *board, uint16_t move);
int moveEstimatedValue(Board *board, uint16_t move);
int moveBestCaseValue(Board *board);
int moveIsPsuedoLegal(Board *board, uint16_t move);
int moveWasLegal(Board *board);
void moveToString(Board *board, uint16_t move, char *str);

#define MoveFrom(move)         (((move) >> 0) & 63)
#define MoveTo(move)           (((move) >> 6) & 63)
#define MoveType(move)         ((move) & (3 << 12))
#define MoveCastleSide(move)   ((move) & (1 << 14))
#define MovePromoType(move)    ((move) & (3 << 14))
#define MovePromoPiece(move)   (1 + ((move) >> 14))
#define MoveMake(from,to,flag) ((from) | ((to) << 6) | (flag))
