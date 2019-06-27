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

#include "assert.h"
#include "bitboards.h"
#include "evaluate.h"
#include "psqt.h"
#include "types.h"

int PSQT[32][SQUARE_NB];

#define S(mg, eg) MakeScore((mg), (eg))

const int PawnPSQT32[32] = {
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
    S( -21,  11), S(   6,   3), S( -15,   8), S(  -9,  -2),
    S( -22,   3), S( -14,   3), S(  -9,  -7), S(  -5, -15),
    S( -17,  13), S( -10,  12), S(  16, -15), S(  15, -28),
    S(  -2,  19), S(   6,  13), S(   3,  -1), S(  18, -22),
    S(  -1,  35), S(   3,  34), S(  14,  23), S(  43,  -4),
    S( -17, -40), S( -65,  -7), S(   3, -21), S(  40, -34),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

const int KnightPSQT32[32] = {
    S( -50, -26), S(  -9, -43), S( -19, -30), S(  -3, -20),
    S(  -8, -22), S(   4, -13), S(  -1, -31), S(  11, -20),
    S(   2, -24), S(  21, -25), S(  13, -19), S(  25,  -3),
    S(  19,   4), S(  25,   7), S(  32,  17), S(  31,  22),
    S(  28,  18), S(  28,  13), S(  42,  26), S(  32,  40),
    S( -13,  16), S(   8,  14), S(  33,  28), S(  35,  31),
    S(   8, -10), S(  -3,   5), S(  40, -17), S(  44,   3),
    S(-166, -16), S( -81,  -2), S(-110,  20), S( -30,   3),
};

const int BishopPSQT32[32] = {
    S(  18, -17), S(  17, -19), S( -11,  -8), S(   9, -13),
    S(  34, -32), S(  27, -33), S(  25, -22), S(  11, -11),
    S(  18, -14), S(  32, -14), S(  18,  -5), S(  20,  -3),
    S(  19, -10), S(  18,  -2), S(  18,   6), S(  19,  11),
    S( -10,   9), S(  19,   4), S(   7,  13), S(  12,  22),
    S(   2,   5), S(   0,  15), S(  18,  12), S(  22,  10),
    S( -47,  14), S( -35,  12), S(  -2,   6), S( -19,   9),
    S( -39,   3), S( -48,   9), S( -87,  17), S( -91,  25),
};

const int RookPSQT32[32] = {
    S(  -6, -30), S( -12, -20), S(   1, -24), S(   9, -30),
    S( -54, -13), S( -14, -30), S(  -9, -30), S(  -1, -33),
    S( -27, -13), S(  -6, -12), S( -16, -15), S(  -3, -23),
    S( -14,   1), S(  -5,   8), S(  -6,   4), S(   6,  -3),
    S(   2,  13), S(  17,  10), S(  27,   5), S(  38,   0),
    S(  -6,  23), S(  30,  11), S(   6,  21), S(  37,   5),
    S(   4,  12), S( -16,  20), S(   9,  10), S(  21,   9),
    S(  37,  24), S(  26,  27), S(   6,  32), S(  16,  27),
};

const int QueenPSQT32[32] = {
    S(  16, -53), S(  -3, -39), S(   6, -49), S(  20, -44),
    S(  15, -38), S(  29, -56), S(  31, -71), S(  22, -24),
    S(  14, -20), S(  30, -17), S(  11,   8), S(  12,   4),
    S(  15,   0), S(  18,  20), S(   3,  25), S( -13,  70),
    S(  -2,  21), S(  -2,  45), S( -10,  26), S( -27,  77),
    S( -19,  30), S( -10,  22), S( -18,  27), S(  -7,  31),
    S(  -4,  28), S( -59,  67), S(  -6,  20), S( -38,  57),
    S(  -4,  20), S(  20,  11), S(  11,  12), S(  -5,  22),
};

const int KingPSQT32[32] = {
    S(  41, -85), S(  41, -55), S( -11, -17), S( -26, -26),
    S(  30, -36), S(  -4, -25), S( -38,   2), S( -51,   4),
    S(   8, -35), S(  18, -32), S(  16,  -8), S( -10,   7),
    S(   2, -36), S(  82, -40), S(  40,  -1), S(  -6,  19),
    S(   4, -18), S(  95, -28), S(  46,  12), S(   2,  21),
    S(  47, -16), S( 120, -13), S(  95,  13), S(  39,  12),
    S(   7, -36), S(  48,   0), S(  33,  15), S(  10,   8),
    S(  10, -95), S(  76, -48), S( -18,  -8), S( -17,  -8),
};

#undef S

int relativeSquare32(int s, int c) {
    assert(0 <= c && c < COLOUR_NB);
    assert(0 <= s && s < SQUARE_NB);
    static const int edgeDistance[FILE_NB] = {0, 1, 2, 3, 3, 2, 1, 0};
    return 4 * relativeRankOf(c, s) + edgeDistance[fileOf(s)];
}

void initializePSQT() {

    for (int s = 0; s < SQUARE_NB; s++) {
        const int w32 = relativeSquare32(s, WHITE);
        const int b32 = relativeSquare32(s, BLACK);

        PSQT[WHITE_PAWN  ][s] = +MakeScore(PieceValues[PAWN  ][MG], PieceValues[PAWN  ][EG]) +   PawnPSQT32[w32];
        PSQT[WHITE_KNIGHT][s] = +MakeScore(PieceValues[KNIGHT][MG], PieceValues[KNIGHT][EG]) + KnightPSQT32[w32];
        PSQT[WHITE_BISHOP][s] = +MakeScore(PieceValues[BISHOP][MG], PieceValues[BISHOP][EG]) + BishopPSQT32[w32];
        PSQT[WHITE_ROOK  ][s] = +MakeScore(PieceValues[ROOK  ][MG], PieceValues[ROOK  ][EG]) +   RookPSQT32[w32];
        PSQT[WHITE_QUEEN ][s] = +MakeScore(PieceValues[QUEEN ][MG], PieceValues[QUEEN ][EG]) +  QueenPSQT32[w32];
        PSQT[WHITE_KING  ][s] = +MakeScore(PieceValues[KING  ][MG], PieceValues[KING  ][EG]) +   KingPSQT32[w32];

        PSQT[BLACK_PAWN  ][s] = -MakeScore(PieceValues[PAWN  ][MG], PieceValues[PAWN  ][EG]) -   PawnPSQT32[b32];
        PSQT[BLACK_KNIGHT][s] = -MakeScore(PieceValues[KNIGHT][MG], PieceValues[KNIGHT][EG]) - KnightPSQT32[b32];
        PSQT[BLACK_BISHOP][s] = -MakeScore(PieceValues[BISHOP][MG], PieceValues[BISHOP][EG]) - BishopPSQT32[b32];
        PSQT[BLACK_ROOK  ][s] = -MakeScore(PieceValues[ROOK  ][MG], PieceValues[ROOK  ][EG]) -   RookPSQT32[b32];
        PSQT[BLACK_QUEEN ][s] = -MakeScore(PieceValues[QUEEN ][MG], PieceValues[QUEEN ][EG]) -  QueenPSQT32[b32];
        PSQT[BLACK_KING  ][s] = -MakeScore(PieceValues[KING  ][MG], PieceValues[KING  ][EG]) -   KingPSQT32[b32];
    }
}
