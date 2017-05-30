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

int PSQTopening[32][SQUARE_NB];
int PSQTendgame[32][SQUARE_NB];

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


int PawnOpeningMap32[32] = {
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

int KnightOpeningMap32[32] = {
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

int BishopOpeningMap32[32] = {
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

int RookOpeningMap32[32] = {
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

int QueenOpeningMap32[32] = {
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

int KingOpeningMap32[32] = {
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
    
    int i, j;
    
    // Zero out each Piece-Square table
    for(i = 0; i < 32; i++){
        for(j = 0; j < SQUARE_NB; j++){
            PSQTopening[i][j] = 0;
            PSQTendgame[i][j] = 0;
        }
    }
    
    for(i = 0, j = 0; i < SQUARE_NB; i += 8, j += 4){
        
        // Opening Pawn
        PSQTopening[WHITE_PAWN][i+0] = PawnValue + PawnOpeningMap32[j+0];
        PSQTopening[WHITE_PAWN][i+7] = PawnValue + PawnOpeningMap32[j+0];
        PSQTopening[WHITE_PAWN][i+1] = PawnValue + PawnOpeningMap32[j+1];
        PSQTopening[WHITE_PAWN][i+6] = PawnValue + PawnOpeningMap32[j+1];
        PSQTopening[WHITE_PAWN][i+2] = PawnValue + PawnOpeningMap32[j+2];
        PSQTopening[WHITE_PAWN][i+5] = PawnValue + PawnOpeningMap32[j+2];
        PSQTopening[WHITE_PAWN][i+3] = PawnValue + PawnOpeningMap32[j+3];
        PSQTopening[WHITE_PAWN][i+4] = PawnValue + PawnOpeningMap32[j+3];

        // Endgame Pawn
        PSQTendgame[WHITE_PAWN][i+0] = PawnValue + PawnEndgameMap32[j+0];
        PSQTendgame[WHITE_PAWN][i+7] = PawnValue + PawnEndgameMap32[j+0];
        PSQTendgame[WHITE_PAWN][i+1] = PawnValue + PawnEndgameMap32[j+1];
        PSQTendgame[WHITE_PAWN][i+6] = PawnValue + PawnEndgameMap32[j+1];
        PSQTendgame[WHITE_PAWN][i+2] = PawnValue + PawnEndgameMap32[j+2];
        PSQTendgame[WHITE_PAWN][i+5] = PawnValue + PawnEndgameMap32[j+2];
        PSQTendgame[WHITE_PAWN][i+3] = PawnValue + PawnEndgameMap32[j+3];
        PSQTendgame[WHITE_PAWN][i+4] = PawnValue + PawnEndgameMap32[j+3];
        
        // Opening Knight
        PSQTopening[WHITE_KNIGHT][i+0] = KnightValue + KnightOpeningMap32[j+0];
        PSQTopening[WHITE_KNIGHT][i+7] = KnightValue + KnightOpeningMap32[j+0];
        PSQTopening[WHITE_KNIGHT][i+1] = KnightValue + KnightOpeningMap32[j+1];
        PSQTopening[WHITE_KNIGHT][i+6] = KnightValue + KnightOpeningMap32[j+1];
        PSQTopening[WHITE_KNIGHT][i+2] = KnightValue + KnightOpeningMap32[j+2];
        PSQTopening[WHITE_KNIGHT][i+5] = KnightValue + KnightOpeningMap32[j+2];
        PSQTopening[WHITE_KNIGHT][i+3] = KnightValue + KnightOpeningMap32[j+3];
        PSQTopening[WHITE_KNIGHT][i+4] = KnightValue + KnightOpeningMap32[j+3];

        // Ending Knight
        PSQTendgame[WHITE_KNIGHT][i+0] = KnightValue + KnightEndgameMap32[j+0];
        PSQTendgame[WHITE_KNIGHT][i+7] = KnightValue + KnightEndgameMap32[j+0];
        PSQTendgame[WHITE_KNIGHT][i+1] = KnightValue + KnightEndgameMap32[j+1];
        PSQTendgame[WHITE_KNIGHT][i+6] = KnightValue + KnightEndgameMap32[j+1];
        PSQTendgame[WHITE_KNIGHT][i+2] = KnightValue + KnightEndgameMap32[j+2];
        PSQTendgame[WHITE_KNIGHT][i+5] = KnightValue + KnightEndgameMap32[j+2];
        PSQTendgame[WHITE_KNIGHT][i+3] = KnightValue + KnightEndgameMap32[j+3];
        PSQTendgame[WHITE_KNIGHT][i+4] = KnightValue + KnightEndgameMap32[j+3];
        
        // Opening Bishop
        PSQTopening[WHITE_BISHOP][i+0] = BishopValue + BishopOpeningMap32[j+0];
        PSQTopening[WHITE_BISHOP][i+7] = BishopValue + BishopOpeningMap32[j+0];
        PSQTopening[WHITE_BISHOP][i+1] = BishopValue + BishopOpeningMap32[j+1];
        PSQTopening[WHITE_BISHOP][i+6] = BishopValue + BishopOpeningMap32[j+1];
        PSQTopening[WHITE_BISHOP][i+2] = BishopValue + BishopOpeningMap32[j+2];
        PSQTopening[WHITE_BISHOP][i+5] = BishopValue + BishopOpeningMap32[j+2];
        PSQTopening[WHITE_BISHOP][i+3] = BishopValue + BishopOpeningMap32[j+3];
        PSQTopening[WHITE_BISHOP][i+4] = BishopValue + BishopOpeningMap32[j+3];

        // Ending Bishop
        PSQTendgame[WHITE_BISHOP][i+0] = BishopValue + BishopEndgameMap32[j+0];
        PSQTendgame[WHITE_BISHOP][i+7] = BishopValue + BishopEndgameMap32[j+0];
        PSQTendgame[WHITE_BISHOP][i+1] = BishopValue + BishopEndgameMap32[j+1];
        PSQTendgame[WHITE_BISHOP][i+6] = BishopValue + BishopEndgameMap32[j+1];
        PSQTendgame[WHITE_BISHOP][i+2] = BishopValue + BishopEndgameMap32[j+2];
        PSQTendgame[WHITE_BISHOP][i+5] = BishopValue + BishopEndgameMap32[j+2];
        PSQTendgame[WHITE_BISHOP][i+3] = BishopValue + BishopEndgameMap32[j+3];
        PSQTendgame[WHITE_BISHOP][i+4] = BishopValue + BishopEndgameMap32[j+3];
        
        // Opening Rook
        PSQTopening[WHITE_ROOK][i+0] = RookValue + RookOpeningMap32[j+0];
        PSQTopening[WHITE_ROOK][i+7] = RookValue + RookOpeningMap32[j+0];
        PSQTopening[WHITE_ROOK][i+1] = RookValue + RookOpeningMap32[j+1];
        PSQTopening[WHITE_ROOK][i+6] = RookValue + RookOpeningMap32[j+1];
        PSQTopening[WHITE_ROOK][i+2] = RookValue + RookOpeningMap32[j+2];
        PSQTopening[WHITE_ROOK][i+5] = RookValue + RookOpeningMap32[j+2];
        PSQTopening[WHITE_ROOK][i+3] = RookValue + RookOpeningMap32[j+3];
        PSQTopening[WHITE_ROOK][i+4] = RookValue + RookOpeningMap32[j+3];

        // Ending Rook
        PSQTendgame[WHITE_ROOK][i+0] = RookValue + RookEndgameMap32[j+0];
        PSQTendgame[WHITE_ROOK][i+7] = RookValue + RookEndgameMap32[j+0];
        PSQTendgame[WHITE_ROOK][i+1] = RookValue + RookEndgameMap32[j+1];
        PSQTendgame[WHITE_ROOK][i+6] = RookValue + RookEndgameMap32[j+1];
        PSQTendgame[WHITE_ROOK][i+2] = RookValue + RookEndgameMap32[j+2];
        PSQTendgame[WHITE_ROOK][i+5] = RookValue + RookEndgameMap32[j+2];
        PSQTendgame[WHITE_ROOK][i+3] = RookValue + RookEndgameMap32[j+3];
        PSQTendgame[WHITE_ROOK][i+4] = RookValue + RookEndgameMap32[j+3];
        
        // Opening Queen
        PSQTopening[WHITE_QUEEN][i+0] = QueenValue + QueenOpeningMap32[j+0];
        PSQTopening[WHITE_QUEEN][i+7] = QueenValue + QueenOpeningMap32[j+0];
        PSQTopening[WHITE_QUEEN][i+1] = QueenValue + QueenOpeningMap32[j+1];
        PSQTopening[WHITE_QUEEN][i+6] = QueenValue + QueenOpeningMap32[j+1];
        PSQTopening[WHITE_QUEEN][i+2] = QueenValue + QueenOpeningMap32[j+2];
        PSQTopening[WHITE_QUEEN][i+5] = QueenValue + QueenOpeningMap32[j+2];
        PSQTopening[WHITE_QUEEN][i+3] = QueenValue + QueenOpeningMap32[j+3];
        PSQTopening[WHITE_QUEEN][i+4] = QueenValue + QueenOpeningMap32[j+3];
        
        // Ending Queen
        PSQTendgame[WHITE_QUEEN][i+0] = QueenValue + QueenEndgameMap32[j+0];
        PSQTendgame[WHITE_QUEEN][i+7] = QueenValue + QueenEndgameMap32[j+0];
        PSQTendgame[WHITE_QUEEN][i+1] = QueenValue + QueenEndgameMap32[j+1];
        PSQTendgame[WHITE_QUEEN][i+6] = QueenValue + QueenEndgameMap32[j+1];
        PSQTendgame[WHITE_QUEEN][i+2] = QueenValue + QueenEndgameMap32[j+2];
        PSQTendgame[WHITE_QUEEN][i+5] = QueenValue + QueenEndgameMap32[j+2];
        PSQTendgame[WHITE_QUEEN][i+3] = QueenValue + QueenEndgameMap32[j+3];
        PSQTendgame[WHITE_QUEEN][i+4] = QueenValue + QueenEndgameMap32[j+3];
        
        // Opening King
        PSQTopening[WHITE_KING][i+0] = KingValue + KingOpeningMap32[j+0];
        PSQTopening[WHITE_KING][i+7] = KingValue + KingOpeningMap32[j+0];
        PSQTopening[WHITE_KING][i+1] = KingValue + KingOpeningMap32[j+1];
        PSQTopening[WHITE_KING][i+6] = KingValue + KingOpeningMap32[j+1];
        PSQTopening[WHITE_KING][i+2] = KingValue + KingOpeningMap32[j+2];
        PSQTopening[WHITE_KING][i+5] = KingValue + KingOpeningMap32[j+2];
        PSQTopening[WHITE_KING][i+3] = KingValue + KingOpeningMap32[j+3];
        PSQTopening[WHITE_KING][i+4] = KingValue + KingOpeningMap32[j+3];
        
        // Ending King
        PSQTendgame[WHITE_KING][i+0] = KingValue + KingEndgameMap32[j+0];
        PSQTendgame[WHITE_KING][i+7] = KingValue + KingEndgameMap32[j+0];
        PSQTendgame[WHITE_KING][i+1] = KingValue + KingEndgameMap32[j+1];
        PSQTendgame[WHITE_KING][i+6] = KingValue + KingEndgameMap32[j+1];
        PSQTendgame[WHITE_KING][i+2] = KingValue + KingEndgameMap32[j+2];
        PSQTendgame[WHITE_KING][i+5] = KingValue + KingEndgameMap32[j+2];
        PSQTendgame[WHITE_KING][i+3] = KingValue + KingEndgameMap32[j+3];
        PSQTendgame[WHITE_KING][i+4] = KingValue + KingEndgameMap32[j+3];
    }
    
    // Mirror each value for black
    for(i = BLACK_PAWN; i <= BLACK_KING; i+= 4){
        for(j = 0; j < SQUARE_NB; j++){
            PSQTopening[i][j] = -PSQTopening[i-1][InversionTable[j]];
            PSQTendgame[i][j] = -PSQTendgame[i-1][InversionTable[j]];
        }
    }
}