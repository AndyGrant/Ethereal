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
    S( -23,   2), S(  13,   3), S(  -6,   6), S(  -5,   0),
    S( -26,   0), S(  -4,  -1), S(  -6,  -6), S(  -2, -11),
    S( -21,   7), S(  -7,   6), S(   5, -10), S(   3, -23),
    S( -10,  15), S(   3,   9), S(   0,  -2), S(   4, -23),
    S(   1,  29), S(  13,  28), S(  18,   6), S(  24, -21),
    S( -46,   7), S( -34,  11), S(  -4, -16), S(   1, -33),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

const int KnightPSQT32[32] = {
    S( -40, -49), S(   4, -39), S(  -9, -15), S(  11,  -8),
    S(   7, -50), S(  10, -11), S(  15, -26), S(  21,  -4),
    S(   5, -21), S(  27, -19), S(  15,   3), S(  29,  12),
    S(   7,   7), S(  28,   8), S(  30,  31), S(  36,  34),
    S(  28,   6), S(  33,  13), S(  38,  39), S(  44,  41),
    S( -29,   9), S(  28,   5), S(  39,  37), S(  48,  34),
    S( -37, -24), S( -37,   5), S(  43, -33), S(  13,  -4),
    S(-170, -37), S(-103, -31), S(-159,  -9), S( -42, -28),
};

const int BishopPSQT32[32] = {
    S(  27, -22), S(  23, -27), S(   1,  -8), S(  20, -14),
    S(  37, -29), S(  34, -24), S(  26, -15), S(  11,  -3),
    S(  26, -12), S(  32, -13), S(  24,   1), S(  19,   7),
    S(  13,  -5), S(  12,  -1), S(  10,  14), S(  33,  19),
    S( -13,  12), S(  25,   3), S(   4,  16), S(  28,  19),
    S(  -5,   6), S(  -1,   7), S(  25,   6), S(  19,   5),
    S( -69,   2), S(  -4,  -5), S( -12, -14), S( -43,  -1),
    S( -52,  -1), S( -65,  -4), S(-129,   6), S(-114,  12),
};

const int RookPSQT32[32] = {
    S(  -4, -32), S(  -6, -18), S(   5, -14), S(  11, -20),
    S( -35, -25), S(  -6, -28), S(   2, -20), S(  10, -26),
    S( -20, -19), S(   4, -14), S(  -1, -18), S(   2, -20),
    S( -21,  -1), S( -12,   4), S(  -4,   2), S(  -2,   2),
    S( -14,  11), S( -13,   9), S(  16,   5), S(  19,   6),
    S( -18,  14), S(  15,   9), S(  11,  13), S(  18,  13),
    S(  -3,  16), S(  -9,  16), S(  36,   2), S(  20,   8),
    S(   0,  22), S(  11,  13), S( -24,  22), S(   3,  27),
};

const int QueenPSQT32[32] = {
    S(  -1, -47), S( -10, -30), S(  -3, -21), S(  17, -41),
    S(   7, -49), S(  15, -37), S(  21, -52), S(  16, -15),
    S(   7, -23), S(  23, -18), S(   7,   6), S(   4,   4),
    S(   6,  -6), S(   8,   4), S(  -6,  15), S(  -8,  46),
    S( -14,  10), S( -15,  33), S(  -9,  22), S( -25,  52),
    S( -15,   3), S(  -6,  19), S(  -1,  21), S( -11,  46),
    S(  -7,  12), S( -76,  55), S(  23,  11), S( -21,  67),
    S( -22, -24), S(   2, -14), S(   8,  -6), S( -20,   9),
};

const int KingPSQT32[32] = {
    S(  81,-106), S(  89, -80), S(  40, -35), S(  22, -39),
    S(  71, -54), S(  60, -45), S(  10,  -5), S( -21,   3),
    S(   0, -41), S(  44, -31), S(  16,  -1), S( -15,  16),
    S( -53, -33), S(  33, -19), S(   1,  15), S( -47,  37),
    S( -19, -18), S(  56,   2), S(   8,  31), S( -32,  38),
    S(  40, -17), S(  85,   0), S(  74,  21), S(   9,  18),
    S(  17, -17), S(  52,  -4), S(  35,   0), S(   9,   1),
    S(  29, -81), S(  86, -67), S( -22, -35), S( -16, -36),
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
