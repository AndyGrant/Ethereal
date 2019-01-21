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
    S( -21,  10), S(   5,   4), S( -15,   7), S(  -9,  -3),
    S( -21,   3), S( -13,   3), S( -10,  -7), S(  -5, -16),
    S( -17,  12), S( -10,  12), S(  14, -14), S(  13, -28),
    S(  -2,  18), S(   5,  13), S(   3,  -1), S(  17, -21),
    S(   0,  33), S(   4,  37), S(  16,  25), S(  44,   0),
    S( -20, -41), S( -65,  -7), S(   3, -19), S(  37, -33),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

const int KnightPSQT32[32] = {
    S( -49, -28), S(  -8, -49), S( -18, -32), S(  -2, -22),
    S(  -7, -23), S(   4, -15), S(   0, -33), S(  11, -21),
    S(   2, -27), S(  21, -26), S(  13, -18), S(  25,  -1),
    S(  19,   4), S(  24,   7), S(  32,  18), S(  31,  23),
    S(  28,  18), S(  29,  12), S(  41,  29), S(  32,  41),
    S( -14,  16), S(  10,  14), S(  32,  33), S(  36,  33),
    S(   6, -11), S(  -6,   3), S(  41, -20), S(  40,   1),
    S(-163, -18), S( -85,  -5), S(-117,  18), S( -34,  -1),
};

const int BishopPSQT32[32] = {
    S(  18, -19), S(  16, -22), S( -10, -11), S(   9, -15),
    S(  34, -32), S(  27, -33), S(  25, -22), S(  11, -11),
    S(  18, -14), S(  33, -14), S(  18,  -4), S(  20,  -2),
    S(  18,  -8), S(  18,  -1), S(  18,   8), S(  18,  14),
    S(  -9,   9), S(  20,   4), S(   6,  15), S(  12,  24),
    S(   2,   6), S(   0,  13), S(  17,  12), S(  22,  10),
    S( -46,  10), S( -32,   7), S(  -2,   1), S( -22,   3),
    S( -44,   0), S( -52,   4), S( -95,  12), S( -99,  19),
};

const int RookPSQT32[32] = {
    S(  -7, -31), S( -13, -21), S(   0, -25), S(   7, -29),
    S( -55, -16), S( -16, -31), S( -11, -32), S(  -2, -34),
    S( -28, -15), S(  -7, -12), S( -18, -16), S(  -4, -24),
    S( -15,   0), S(  -6,   6), S(  -8,   2), S(   3,  -3),
    S(   0,  12), S(  13,   9), S(  24,   4), S(  35,   0),
    S(  -8,  22), S(  28,   9), S(   0,  19), S(  31,   5),
    S(   5,   9), S( -13,  16), S(   5,   7), S(  18,   6),
    S(  37,  21), S(  26,  23), S(   3,  29), S(  14,  25),
};

const int QueenPSQT32[32] = {
    S(   7, -54), S( -10, -38), S(  -1, -53), S(  14, -47),
    S(   9, -40), S(  22, -58), S(  25, -74), S(  15, -27),
    S(   7, -22), S(  23, -19), S(   5,   3), S(   4,   2),
    S(   8,  -2), S(  12,  15), S(  -2,  17), S( -18,  63),
    S(  -9,  22), S(  -7,  40), S( -16,  19), S( -34,  70),
    S( -24,  27), S( -15,  20), S( -20,  18), S( -13,  22),
    S(  -7,  27), S( -64,  61), S( -10,  15), S( -43,  54),
    S( -20,   6), S(  11,  -3), S(   5,  -1), S( -13,  13),
};

const int KingPSQT32[32] = {
    S(  41, -81), S(  43, -51), S( -10, -12), S( -28, -20),
    S(  34, -34), S(   1, -25), S( -33,   3), S( -47,   6),
    S(  15, -37), S(  27, -34), S(  23,  -8), S(  -2,   8),
    S(  -1, -38), S(  83, -41), S(  42,   0), S(  -6,  18),
    S(   1, -22), S(  89, -27), S(  39,  11), S(  -4,  19),
    S(  46, -20), S( 112, -14), S(  89,  10), S(  32,   9),
    S(   7, -42), S(  47,  -5), S(  33,  11), S(   8,   6),
    S(  14, -92), S(  79, -51), S( -19,  -9), S( -16,  -7),
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
