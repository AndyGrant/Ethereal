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

#include "evaluate.h"
#include "piece.h"
#include "psqt.h"

int PSQTMidgame[32][SQUARE_NB];
int PSQTEndgame[32][SQUARE_NB];

int InversionTable[SQUARE_NB] = {
    56,  57,  58,  59,  60,  61,  62,  63,
    48,  49,  50,  51,  52,  53,  54,  55,
    40,  41,  42,  43,  44,  45,  46,  47,
    32,  33,  34,  35,  36,  37,  38,  39,
    24,  25,  26,  27,  28,  29,  30,  31,
    16,  17,  18,  19,  20,  21,  22,  23,
     8,   9,  10,  11,  12,  13,  14,  15,
     0,   1,   2,   3,   4,   5,   6,   7
};

int PawnMidgameMap32[32] = {
     -15, -10,   0,   5,
     -15, -10,   0,   5,
     -15, -10,   0,  15,
     -15, -10,   0,  25,
     -15, -10,   0,  15,
     -15, -10,   0,   5,
     -15, -10,   0,   5,
     -15, -10,   0,   5,
};

int PawnEndgameMap32[32] = {
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
};

int KnightMidgameMap32[32] = {
     -50, -40, -30, -25,
     -35, -25, -15, -10,
     -20, -10,   0,   5,
     -10,   0,  10,  15,
      -5,   5,  15,  20,
      -5,   5,  15,  20,
     -20, -10,   0,   5,
    -135, -25, -15, -10,
};

int KnightEndgameMap32[32] = {
     -40, -30, -20, -15,
     -30, -20, -10,  -5,
     -20, -10,   0,   5,
     -15,  -5,   5,  10,
     -15,  -5,   5,  10,
     -20, -10,   0,   5,
     -30, -20, -10,  -5,
     -40, -30, -20, -15,
};

int BishopMidgameMap32[32] = {
     -18, -18, -16, -14,
      -8,   0,  -2,   0,
      -6,  -2,   4,   2,
      -4,   0,   2,   8,
      -4,   0,   2,   8,
      -6,  -2,   4,   2,
      -8,   0,  -2,   0,
      -8,  -8,  -6,  -4,
};

int BishopEndgameMap32[32] = {
     -18, -12,  -9,  -6,
     -12,  -6,  -3,   0,
      -9,  -3,   0,   3,
      -6,   0,   3,   6,
      -6,   0,   3,   6,
      -9,  -3,   0,   3,
     -12,  -6,  -3,   0,
     -18, -12,  -9,  -6,
};

int RookMidgameMap32[32] = {
      -2,  -1,   0,   1,
      -2,  -1,   0,   1,
      -2,  -1,   0,   1,
      -2,  -1,   0,   1,
      -2,  -1,   0,   1,
      -2,  -1,   0,   1,
      -2,  -1,   0,   1,
      -2,  -1,   0,   1,
};

int RookEndgameMap32[32] = {
      -5,  -2,  -2,  -2, 
      -5,   0,   0,   0,
      -5,   0,   0,   0,
      -5,   0,   0,   0,
      -5,   0,   0,   0,
      -5,   0,   0,   0,
      -5,   0,   0,   0,
      -5,   0,   0,   0
};

int QueenMidgameMap32[32] = {
      -5,  -5,  -5,  -5,
       0,   0,   0,   0,
       0,   0,   2,   4,
       0,   2,   4,   6,
       0,   2,   4,   6,
       0,   0,   2,   4,
       0,   0,   0,   2,
       0,   0,   0,   0
};

int QueenEndgameMap32[32] = {
     -24, -16, -12,  -8,
     -16,  -8,  -4,   0,
     -12,  -4,   0,   4,
      -8,   0,   4,   8,
      -8,   0,   4,   8,
     -12,  -4,   0,   4,
     -16,  -8,  -4,   0,
     -24, -16, -12,  -8,
};

int KingMidgameMap32[32] = {
      40,  50,  30,  10,
      30,  40,  20,   0,
      10,  20,   0, -20,
       0,  10, -10, -30,
     -10,   0, -20, -40,
     -20, -10, -30, -50,
     -30, -20, -40, -60,
     -40, -30, -50, -70,
};

int KingEndgameMap32[32] = {
     -72, -48, -36, -24,
     -48, -24, -12,   0,
     -36, -12,   0,  12,
     -24,   0,  12,  24,
     -24,   0,  12,  24,
     -36, -12,   0,  12,
     -48, -24, -12,   0,
     -72, -48, -36, -24,
};

/**
 * Fill the opening and endgame piece-square tables using
 * the opening and endgame maps. Mirror these values for
 * and negate them for black. The tables are always scoring
 * assuming that white is positive and black is negative.
 */
void initalizePSQT(){
    
    int sq, i, j;
    
    static const int table[8] = {0, 1, 2, 3, 3, 2, 1, 0};
    
    for (sq = 0; sq < SQUARE_NB; sq++){
        
        i = 4 * (sq / 8) + table[sq % 8];
        j = 4 * (InversionTable[sq] / 8) + table[InversionTable[sq] % 8];
        
        PSQTMidgame[WHITE_PAWN  ][sq] = +PawnValue   + PawnMidgameMap32[i];
        PSQTEndgame[WHITE_PAWN  ][sq] = +PawnValue   + PawnEndgameMap32[i];
        PSQTMidgame[WHITE_KNIGHT][sq] = +KnightValue + KnightMidgameMap32[i];
        PSQTEndgame[WHITE_KNIGHT][sq] = +KnightValue + KnightEndgameMap32[i];
        PSQTMidgame[WHITE_BISHOP][sq] = +BishopValue + BishopMidgameMap32[i];
        PSQTEndgame[WHITE_BISHOP][sq] = +BishopValue + BishopEndgameMap32[i];
        PSQTMidgame[WHITE_ROOK  ][sq] = +RookValue   + RookMidgameMap32[i];
        PSQTEndgame[WHITE_ROOK  ][sq] = +RookValue   + RookEndgameMap32[i];
        PSQTMidgame[WHITE_QUEEN ][sq] = +QueenValue  + QueenMidgameMap32[i];
        PSQTEndgame[WHITE_QUEEN ][sq] = +QueenValue  + QueenEndgameMap32[i];
        PSQTMidgame[WHITE_KING  ][sq] = +KingValue   + KingMidgameMap32[i];
        PSQTEndgame[WHITE_KING  ][sq] = +KingValue   + KingEndgameMap32[i];
        
        PSQTMidgame[BLACK_PAWN  ][sq] = -PawnValue   - PawnMidgameMap32[j];
        PSQTEndgame[BLACK_PAWN  ][sq] = -PawnValue   - PawnEndgameMap32[j];
        PSQTMidgame[BLACK_KNIGHT][sq] = -KnightValue - KnightMidgameMap32[j];
        PSQTEndgame[BLACK_KNIGHT][sq] = -KnightValue - KnightEndgameMap32[j];
        PSQTMidgame[BLACK_BISHOP][sq] = -BishopValue - BishopMidgameMap32[j];
        PSQTEndgame[BLACK_BISHOP][sq] = -BishopValue - BishopEndgameMap32[j];
        PSQTMidgame[BLACK_ROOK  ][sq] = -RookValue   - RookMidgameMap32[j];
        PSQTEndgame[BLACK_ROOK  ][sq] = -RookValue   - RookEndgameMap32[j];
        PSQTMidgame[BLACK_QUEEN ][sq] = -QueenValue  - QueenMidgameMap32[j];
        PSQTEndgame[BLACK_QUEEN ][sq] = -QueenValue  - QueenEndgameMap32[j];
        PSQTMidgame[BLACK_KING  ][sq] = -KingValue   - KingMidgameMap32[j];
        PSQTEndgame[BLACK_KING  ][sq] = -KingValue   - KingEndgameMap32[j];
    }
}