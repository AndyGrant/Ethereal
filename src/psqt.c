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
    S( -20,   3), S(  12,   2), S(  -5,   5), S(  -3,   0),
    S( -24,   0), S(  -4,   0), S(  -6,  -5), S(   0, -12),
    S( -19,   8), S(  -6,   6), S(   7,  -9), S(   5, -22),
    S(  -8,  15), S(   3,   8), S(   1,  -1), S(   4, -22),
    S(   0,  28), S(  12,  27), S(  17,   5), S(  24, -21),
    S( -45,   7), S( -34,  10), S(  -3, -16), S(   1, -32),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

const int KnightPSQT32[32] = {
    S( -40, -49), S(  -2, -39), S( -11, -16), S(   6,  -8),
    S(   5, -50), S(   8, -11), S(   9, -25), S(  12,  -3),
    S(   3, -22), S(  25, -19), S(  14,   2), S(  29,  11),
    S(   5,   5), S(  29,   8), S(  34,  30), S(  48,  33),
    S(  27,   5), S(  45,  13), S(  43,  39), S(  53,  42),
    S( -28,   8), S(  27,   4), S(  41,  37), S(  50,  34),
    S( -35, -23), S( -36,   4), S(  42, -33), S(  13,  -3),
    S(-169, -35), S(-103, -31), S(-158,  -8), S( -41, -27),
};

const int BishopPSQT32[32] = {
    S(  25, -22), S(  20, -27), S(  -5, -10), S(  17, -14),
    S(  35, -29), S(  28, -25), S(  23, -15), S(   8,  -3),
    S(  24, -12), S(  31, -13), S(  21,   0), S(  21,   7),
    S(  11,  -5), S(  15,   0), S(  18,  14), S(  35,  19),
    S( -12,  11), S(  35,   4), S(   4,  16), S(  30,  19),
    S(  -2,   6), S(   0,   6), S(  26,   6), S(  20,   5),
    S( -69,   1), S(  -3,  -4), S( -10, -13), S( -41,   0),
    S( -50,   0), S( -63,  -3), S(-127,   4), S(-112,  10),
};

const int RookPSQT32[32] = {
    S(  -2, -31), S(  -7, -17), S(   4, -14), S(  11, -21),
    S( -36, -25), S(  -6, -28), S(   1, -20), S(   9, -27),
    S( -20, -19), S(   4, -14), S(  -1, -18), S(   1, -20),
    S( -20,  -1), S( -11,   4), S(  -3,   2), S(  -1,   2),
    S( -13,  11), S( -12,   9), S(  16,   5), S(  19,   6),
    S( -17,  14), S(  15,   9), S(  11,  13), S(  18,  13),
    S(  -2,  16), S(  -8,  16), S(  35,   2), S(  20,   8),
    S(   0,  22), S(  11,  13), S( -23,  22), S(   3,  27),
};

const int QueenPSQT32[32] = {
    S(   0, -47), S( -10, -30), S(  -3, -21), S(  12, -41),
    S(   7, -48), S(  15, -37), S(  20, -52), S(  12, -15),
    S(   6, -23), S(  24, -17), S(   7,   6), S(   3,   3),
    S(   6,  -5), S(   8,   4), S(  -5,  15), S(  -6,  46),
    S( -13,  10), S( -14,  33), S(  -8,  22), S( -24,  52),
    S( -14,   3), S(  -5,  19), S(   0,  21), S( -10,  46),
    S(  -6,  12), S( -75,  55), S(  23,  11), S( -20,  67),
    S( -21, -23), S(   2, -13), S(   8,  -5), S( -19,   9),
};

const int KingPSQT32[32] = {
    S(  80,-106), S(  91, -80), S(  38, -35), S(  21, -39),
    S(  70, -54), S(  60, -44), S(   9,  -4), S( -20,   3),
    S(   0, -41), S(  44, -30), S(  16,   0), S( -14,  16),
    S( -53, -33), S(  33, -19), S(   1,  15), S( -46,  37),
    S( -19, -18), S(  55,   1), S(   8,  31), S( -31,  38),
    S(  39, -17), S(  84,   0), S(  74,  20), S(   9,  17),
    S(  16, -17), S(  51,  -4), S(  34,   0), S(   8,   0),
    S(  28, -81), S(  85, -67), S( -22, -35), S( -16, -36),
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
