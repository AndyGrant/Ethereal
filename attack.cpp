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
// attack.cpp

// includes

#include "attack.h"
#include "board.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "util.h"
#include "vector.h"

// variables

int DeltaIncLine[DeltaNb];
int DeltaIncAll[DeltaNb];

int DeltaMask[DeltaNb];
int IncMask[IncNb];

static int PieceCode[PieceNb];

static int PieceDeltaSize[4][256]; // 4kB
static int PieceDeltaDelta[4][256][4]; // 16 kB

// prototypes

static void add_attack (int piece, int king, int target);

// functions

// attack_init()

void attack_init() {

   int delta, inc;
   int piece;
   int dir, dist;
   int size;
   int king;
   int from, to;
   int pos;

   // clear

   for (delta = 0; delta < DeltaNb; delta++) {
      DeltaIncLine[delta] = IncNone;
      DeltaIncAll[delta] = IncNone;
      DeltaMask[delta] = 0;
   }

   for (inc = 0; inc < IncNb; inc++) {
      IncMask[inc] = 0;
   }

   // pawn attacks

   DeltaMask[DeltaOffset-17] |= BlackPawnFlag;
   DeltaMask[DeltaOffset-15] |= BlackPawnFlag;

   DeltaMask[DeltaOffset+15] |= WhitePawnFlag;
   DeltaMask[DeltaOffset+17] |= WhitePawnFlag;

   // knight attacks

   for (dir = 0; dir < 8; dir++) {

      delta = KnightInc[dir];
      ASSERT(delta_is_ok(delta));

      ASSERT(DeltaIncAll[DeltaOffset+delta]==IncNone);
      DeltaIncAll[DeltaOffset+delta] = delta;
      DeltaMask[DeltaOffset+delta] |= KnightFlag;
   }

   // bishop/queen attacks

   for (dir = 0; dir < 4; dir++) {

      inc = BishopInc[dir];
      ASSERT(inc!=IncNone);

      IncMask[IncOffset+inc] |= BishopFlag;

      for (dist = 1; dist < 8; dist++) {

         delta = inc*dist;
         ASSERT(delta_is_ok(delta));

         ASSERT(DeltaIncLine[DeltaOffset+delta]==IncNone);
         DeltaIncLine[DeltaOffset+delta] = inc;
         ASSERT(DeltaIncAll[DeltaOffset+delta]==IncNone);
         DeltaIncAll[DeltaOffset+delta] = inc;
         DeltaMask[DeltaOffset+delta] |= BishopFlag;
      }
   }

   // rook/queen attacks

   for (dir = 0; dir < 4; dir++) {

      inc = RookInc[dir];
      ASSERT(inc!=IncNone);

      IncMask[IncOffset+inc] |= RookFlag;

      for (dist = 1; dist < 8; dist++) {

         delta = inc*dist;
         ASSERT(delta_is_ok(delta));

         ASSERT(DeltaIncLine[DeltaOffset+delta]==IncNone);
         DeltaIncLine[DeltaOffset+delta] = inc;
         ASSERT(DeltaIncAll[DeltaOffset+delta]==IncNone);
         DeltaIncAll[DeltaOffset+delta] = inc;
         DeltaMask[DeltaOffset+delta] |= RookFlag;
      }
   }

   // king attacks

   for (dir = 0; dir < 8; dir++) {

      delta = KingInc[dir];
      ASSERT(delta_is_ok(delta));

      DeltaMask[DeltaOffset+delta] |= KingFlag;
   }

   // PieceCode[]

   for (piece = 0; piece < PieceNb; piece++) {
      PieceCode[piece] = -1;
   }

   PieceCode[WN] = 0;
   PieceCode[WB] = 1;
   PieceCode[WR] = 2;
   PieceCode[WQ] = 3;

   PieceCode[BN] = 0;
   PieceCode[BB] = 1;
   PieceCode[BR] = 2;
   PieceCode[BQ] = 3;

   // PieceDeltaSize[][] & PieceDeltaDelta[][][]

   for (piece = 0; piece < 4; piece++) {
      for (delta = 0; delta < 256; delta++) {
         PieceDeltaSize[piece][delta] = 0;
      }
   }

   for (king = 0; king < SquareNb; king++) {

      if (SQUARE_IS_OK(king)) {

         for (from = 0; from < SquareNb; from++) {

            if (SQUARE_IS_OK(from)) {

               // knight

               for (pos = 0; (inc=KnightInc[pos]) != IncNone; pos++) {
                  to = from + inc;
                  if (SQUARE_IS_OK(to) && DISTANCE(to,king) == 1) {
                     add_attack(0,king-from,to-from);
                  }
               }

               // bishop

               for (pos = 0; (inc=BishopInc[pos]) != IncNone; pos++) {
                  for (to = from+inc; SQUARE_IS_OK(to); to += inc) {
                     if (DISTANCE(to,king) == 1) {
                        add_attack(1,king-from,to-from);
                        break;
                     }
                  }
               }

               // rook

               for (pos = 0; (inc=RookInc[pos]) != IncNone; pos++) {
                  for (to = from+inc; SQUARE_IS_OK(to); to += inc) {
                     if (DISTANCE(to,king) == 1) {
                        add_attack(2,king-from,to-from);
                        break;
                     }
                  }
               }

               // queen

               for (pos = 0; (inc=QueenInc[pos]) != IncNone; pos++) {
                  for (to = from+inc; SQUARE_IS_OK(to); to += inc) {
                     if (DISTANCE(to,king) == 1) {
                        add_attack(3,king-from,to-from);
                        break;
                     }
                  }
               }
            }
         }
      }
   }

   for (piece = 0; piece < 4; piece++) {
      for (delta = 0; delta < 256; delta++) {
         size = PieceDeltaSize[piece][delta];
         ASSERT(size>=0&&size<3);
         PieceDeltaDelta[piece][delta][size] = DeltaNone;
      }
   }
}

// add_attack()

static void add_attack(int piece, int king, int target) {

   int size;
   int i;

   ASSERT(piece>=0&&piece<4);
   ASSERT(delta_is_ok(king));
   ASSERT(delta_is_ok(target));

   size = PieceDeltaSize[piece][DeltaOffset+king];
   ASSERT(size>=0&&size<3);

   for (i = 0; i < size; i++) {
      if (PieceDeltaDelta[piece][DeltaOffset+king][i] == target) return; // already in the table
   }

   if (size < 2) {
      PieceDeltaDelta[piece][DeltaOffset+king][size] = target;
      size++;
      PieceDeltaSize[piece][DeltaOffset+king] = size;
   }
}

// is_attacked()

bool is_attacked(const board_t * board, int to, int colour) {

   int inc;
   int pawn;
   const sq_t * ptr;
   int from;
   int piece;
   int delta;
   int sq;

   ASSERT(board!=NULL);
   ASSERT(SQUARE_IS_OK(to));
   ASSERT(COLOUR_IS_OK(colour));

   // pawn attack

   inc = PAWN_MOVE_INC(colour);
   pawn = PAWN_MAKE(colour);

   if (board->square[to-(inc-1)] == pawn) return true;
   if (board->square[to-(inc+1)] == pawn) return true;

   // piece attack

   for (ptr = &board->piece[colour][0]; (from=*ptr) != SquareNone; ptr++) {

      piece = board->square[from];
      delta = to - from;

      if (PSEUDO_ATTACK(piece,delta)) {

         inc = DELTA_INC_ALL(delta);
         ASSERT(inc!=IncNone);

         sq = from;
         do {
            sq += inc;
            if (sq == to) return true;
         } while (board->square[sq] == Empty);
      }
   }

   return false;
}

// line_is_empty()

bool line_is_empty(const board_t * board, int from, int to) {

   int delta;
   int inc, sq;

   ASSERT(board!=NULL);
   ASSERT(SQUARE_IS_OK(from));
   ASSERT(SQUARE_IS_OK(to));

   delta = to - from;
   ASSERT(delta_is_ok(delta));

   inc = DELTA_INC_ALL(delta);
   ASSERT(inc!=IncNone);

   sq = from;
   do {
      sq += inc;
      if (sq == to) return true;
   } while (board->square[sq] == Empty);

   return false; // blocker
}

// is_pinned()

bool is_pinned(const board_t * board, int square, int colour) {

   int from, to;
   int inc;
   int sq, piece;

   ASSERT(board!=NULL);
   ASSERT(SQUARE_IS_OK(square));
   ASSERT(COLOUR_IS_OK(colour));

   from = square;
   to = KING_POS(board,colour);

   inc = DELTA_INC_LINE(to-from);
   if (inc == IncNone) return false; // not a line

   sq = from;
   do sq += inc; while (board->square[sq] == Empty);

   if (sq != to) return false; // blocker

   sq = from;
   do sq -= inc; while ((piece=board->square[sq]) == Empty);

   return COLOUR_IS(piece,COLOUR_OPP(colour)) && SLIDER_ATTACK(piece,inc);
}

// attack_is_ok()

bool attack_is_ok(const attack_t * attack) {

   int i;
   int sq, inc;

   if (attack == NULL) return false;

   // checks

   if (attack->dn < 0 || attack->dn > 2) return false;

   for (i = 0; i < attack->dn; i++) {
      sq = attack->ds[i];
      if (!SQUARE_IS_OK(sq)) return false;
      inc = attack->di[i];
      if (inc != IncNone && !inc_is_ok(inc)) return false;
   }

   if (attack->ds[attack->dn] != SquareNone) return false;
   if (attack->di[attack->dn] != IncNone) return false;

   return true;
}

// attack_set()

void attack_set(attack_t * attack, const board_t * board) {

   int me, opp;
   const sq_t * ptr;
   int from, to;
   int inc;
   int pawn;
   int delta, piece;
   int sq;

   ASSERT(attack!=NULL);
   ASSERT(board!=NULL);

   // init

   attack->dn = 0;

   me = board->turn;
   opp = COLOUR_OPP(me);

   to = KING_POS(board,me);

   // pawn attacks

   inc = PAWN_MOVE_INC(opp);
   pawn = PAWN_MAKE(opp);

   from = to - (inc-1);
   if (board->square[from] == pawn) {
      attack->ds[attack->dn] = from;
      attack->di[attack->dn] = IncNone;
      attack->dn++;
   }

   from = to - (inc+1);
   if (board->square[from] == pawn) {
      attack->ds[attack->dn] = from;
      attack->di[attack->dn] = IncNone;
      attack->dn++;
   }

   // piece attacks

   for (ptr = &board->piece[opp][1]; (from=*ptr) != SquareNone; ptr++) { // HACK: no king

      piece = board->square[from];

      delta = to - from;
      ASSERT(delta_is_ok(delta));

      if (PSEUDO_ATTACK(piece,delta)) {

         inc = IncNone;

         if (PIECE_IS_SLIDER(piece)) {

            // check for blockers

            inc = DELTA_INC_LINE(delta);
            ASSERT(inc!=IncNone);

            sq = from;
            do sq += inc; while (board->square[sq] == Empty);

            if (sq != to) continue; // blocker => next attacker
         }

         attack->ds[attack->dn] = from;
         attack->di[attack->dn] = -inc; // HACK
         attack->dn++;
      }
   }

   attack->ds[attack->dn] = SquareNone;
   attack->di[attack->dn] = IncNone;

   // debug

   ASSERT(attack_is_ok(attack));
}

// piece_attack_king()

bool piece_attack_king(const board_t * board, int piece, int from, int king) {

   const inc_t * inc_ptr;
   int code;
   const int * delta_ptr;
   int delta, inc;
   int to;
   int sq;

   ASSERT(board!=NULL);
   ASSERT(piece_is_ok(piece));
   ASSERT(SQUARE_IS_OK(from));
   ASSERT(SQUARE_IS_OK(king));

   inc_ptr = PIECE_INC(piece);

   code = PieceCode[piece];
   ASSERT(code>=0&&code<4);

   if (PIECE_IS_SLIDER(piece)) {

      for (delta_ptr = PieceDeltaDelta[code][DeltaOffset+(king-from)]; (delta=*delta_ptr) != DeltaNone; delta_ptr++) {

         ASSERT(delta_is_ok(delta));

         inc = DeltaIncLine[DeltaOffset+delta];
         ASSERT(inc!=IncNone);

         to = from + delta;

         sq = from;
         do {
            sq += inc;
            if (sq == to && SQUARE_IS_OK(to)) {
               ASSERT(DISTANCE(to,king)==1);
               return true;
            }
         } while (board->square[sq] == Empty);
      }

   } else { // non-slider

      for (delta_ptr = PieceDeltaDelta[code][DeltaOffset+(king-from)]; (delta=*delta_ptr) != DeltaNone; delta_ptr++) {

         ASSERT(delta_is_ok(delta));

         to = from + delta;

         if (SQUARE_IS_OK(to)) {
            ASSERT(DISTANCE(to,king)==1);
            return true;
         }
      }
   }

   return false;
}

// end of attack.cpp
