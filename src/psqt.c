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
#include "square.h"

int PSQT[32][SQUARE_NB];

// Undefined after all PSQT terms
#define S(mg, eg) (MakeScore((mg), (eg)))


const int PawnPSQT32[32] = {
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
    S( -23,   2), S(  12,   3), S(  -5,   6), S(  -6,  -2),
    S( -25,   0), S(  -3,  -1), S(  -5,  -6), S(  -3, -10),
    S( -21,   7), S(  -6,   6), S(   3,  -9), S(   3, -22),
    S( -12,  14), S(   3,   8), S(  -1,  -3), S(   4, -23),
    S(   3,  25), S(  14,  23), S(  18,   3), S(  16, -23),
    S( -40,   6), S( -32,   9), S(   2, -17), S(   4, -33),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

const int KnightPSQT32[32] = {
    S( -33, -46), S(   4, -40), S( -12, -12), S(   9,  -8),
    S(  10, -50), S(   2, -12), S(  17, -24), S(  20,  -3),
    S(   5, -18), S(  29, -18), S(  16,   5), S(  29,  15),
    S(   7,  11), S(  27,   9), S(  31,  34), S(  38,  37),
    S(  28,   9), S(  36,  15), S(  35,  41), S(  48,  42),
    S( -23,   9), S(  38,   7), S(  42,  39), S(  50,  36),
    S( -44, -18), S( -41,   7), S(  60, -26), S(  14,  -1),
    S(-163, -29), S(-103, -30), S(-154,  -6), S( -55, -23),
};

const int BishopPSQT32[32] = {
    S(  14, -21), S(  21, -21), S(   2, -10), S(  19, -14),
    S(  31, -28), S(  33, -23), S(  23, -14), S(  10,  -2),
    S(  25, -11), S(  32, -12), S(  25,   1), S(  19,   7),
    S(  10,  -3), S(  13,   0), S(  11,  15), S(  33,  20),
    S(  -9,  14), S(  23,   4), S(  10,  17), S(  35,  21),
    S(  -6,  10), S(   8,   9), S(  32,   8), S(  22,   5),
    S( -55,   6), S(  13,  -3), S(  -2, -10), S( -36,   3),
    S( -35,  -1), S( -55,  -3), S(-144,   7), S(-124,  16),
};

const int RookPSQT32[32] = {
    S(  -4, -32), S(  -7, -16), S(   5, -13), S(  11, -20),
    S( -33, -25), S(  -7, -26), S(   0, -20), S(  10, -26),
    S( -22, -21), S(   2, -13), S(   1, -21), S(   1, -20),
    S( -23,  -1), S( -12,   5), S(  -7,   3), S(  -1,   2),
    S( -15,  14), S(  -8,   9), S(  20,   8), S(  21,   8),
    S( -14,  17), S(  18,  12), S(  20,  15), S(  22,  14),
    S(   0,  18), S(  -4,  18), S(  40,   3), S(  22,   9),
    S(  -1,  25), S(  17,  15), S( -19,  24), S(  11,  30),
};

const int QueenPSQT32[32] = {
    S(  -3, -45), S( -13, -26), S(  -6, -17), S(  14, -39),
    S(   3, -49), S(  12, -36), S(  19, -50), S(  14, -16),
    S(   6, -22), S(  23, -19), S(   6,   3), S(   4,   3),
    S(   4,  -5), S(   6,   3), S(   0,  11), S(  -5,  45),
    S(  -9,  10), S( -12,  31), S(  -5,  20), S( -20,  50),
    S(  -3,   3), S(   4,  21), S(   7,  18), S(  -6,  45),
    S(   8,  15), S( -53,  55), S(  32,  12), S( -13,  70),
    S( -18, -32), S(   5, -18), S(   0, -14), S( -10,  10),
};

const int KingPSQT32[32] = {
    S(  78,-103), S(  88, -79), S(  37, -35), S(  20, -37),
    S(  69, -53), S(  61, -45), S(  14,  -6), S( -16,   1),
    S(  -1, -41), S(  46, -29), S(  20,  -1), S( -10,  14),
    S( -52, -35), S(  29, -21), S(   7,  15), S( -46,  35),
    S( -27, -19), S(  53,   1), S(   7,  30), S( -34,  37),
    S(  38, -17), S(  82,   2), S(  63,  19), S(  -3,  17),
    S(  25, -15), S(  60,  -2), S(  39,   4), S(  23,   7),
    S(   1, -80), S(  97, -59), S( -12, -34), S( -21, -31),
};


#undef S // Undefine MakeScore

void initializePSQT(){
    
    int sq, w32, b32;
    
    for (sq = 0; sq < SQUARE_NB; sq++){

        w32 = relativeSquare32(sq, WHITE);
        b32 = relativeSquare32(sq, BLACK);

        PSQT[WHITE_PAWN  ][sq] = +MakeScore(PieceValues[PAWN  ][MG], PieceValues[PAWN  ][EG]) +   PawnPSQT32[w32];
        PSQT[WHITE_KNIGHT][sq] = +MakeScore(PieceValues[KNIGHT][MG], PieceValues[KNIGHT][EG]) + KnightPSQT32[w32];
        PSQT[WHITE_BISHOP][sq] = +MakeScore(PieceValues[BISHOP][MG], PieceValues[BISHOP][EG]) + BishopPSQT32[w32];
        PSQT[WHITE_ROOK  ][sq] = +MakeScore(PieceValues[ROOK  ][MG], PieceValues[ROOK  ][EG]) +   RookPSQT32[w32];
        PSQT[WHITE_QUEEN ][sq] = +MakeScore(PieceValues[QUEEN ][MG], PieceValues[QUEEN ][EG]) +  QueenPSQT32[w32];
        PSQT[WHITE_KING  ][sq] = +MakeScore(PieceValues[KING  ][MG], PieceValues[KING  ][EG]) +   KingPSQT32[w32];
                                                                          
        PSQT[BLACK_PAWN  ][sq] = -MakeScore(PieceValues[PAWN  ][MG], PieceValues[PAWN  ][EG]) -   PawnPSQT32[b32];
        PSQT[BLACK_KNIGHT][sq] = -MakeScore(PieceValues[KNIGHT][MG], PieceValues[KNIGHT][EG]) - KnightPSQT32[b32];
        PSQT[BLACK_BISHOP][sq] = -MakeScore(PieceValues[BISHOP][MG], PieceValues[BISHOP][EG]) - BishopPSQT32[b32];
        PSQT[BLACK_ROOK  ][sq] = -MakeScore(PieceValues[ROOK  ][MG], PieceValues[ROOK  ][EG]) -   RookPSQT32[b32];
        PSQT[BLACK_QUEEN ][sq] = -MakeScore(PieceValues[QUEEN ][MG], PieceValues[QUEEN ][EG]) -  QueenPSQT32[b32];
        PSQT[BLACK_KING  ][sq] = -MakeScore(PieceValues[KING  ][MG], PieceValues[KING  ][EG]) -   KingPSQT32[b32];
    }
}
