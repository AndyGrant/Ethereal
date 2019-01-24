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
    S( -20,  10), S(   5,   2), S( -15,   7), S(  -9,  -3),
    S( -20,   3), S( -13,   2), S(  -9,  -7), S(  -5, -15),
    S( -16,  12), S( -10,  11), S(  15, -14), S(  14, -28),
    S(  -1,  18), S(   5,  12), S(   3,  -1), S(  17, -21),
    S(   0,  34), S(   3,  35), S(  15,  23), S(  42,  -2),
    S( -18, -40), S( -65,  -7), S(   2, -19), S(  37, -33),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

const int KnightPSQT32[32] = {
    S( -49, -26), S(  -8, -46), S( -18, -31), S(  -2, -21),
    S(  -7, -21), S(   3, -14), S(  -1, -31), S(  10, -20),
    S(   1, -25), S(  21, -24), S(  13, -18), S(  24,  -1),
    S(  19,   3), S(  24,   7), S(  31,  17), S(  30,  22),
    S(  28,  17), S(  27,  12), S(  41,  27), S(  31,  39),
    S( -13,  15), S(   8,  13), S(  31,  30), S(  34,  32),
    S(   6, -10), S(  -4,   2), S(  40, -18), S(  40,   1),
    S(-163, -16), S( -83,  -3), S(-114,  17), S( -33,   0),
};

const int BishopPSQT32[32] = {
    S(  17, -17), S(  16, -20), S( -11,  -8), S(   8, -13),
    S(  34, -31), S(  26, -32), S(  24, -21), S(  10, -11),
    S(  17, -13), S(  31, -14), S(  17,  -4), S(  19,  -2),
    S(  18,  -8), S(  17,  -1), S(  17,   6), S(  18,  12),
    S(  -9,   8), S(  18,   3), S(   5,  14), S(  11,  23),
    S(   2,   5), S(   0,  13), S(  17,  12), S(  21,   9),
    S( -45,  10), S( -32,   8), S(   0,   3), S( -20,   5),
    S( -42,   0), S( -50,   5), S( -92,  13), S( -96,  21),
};

const int RookPSQT32[32] = {
    S(  -6, -30), S( -12, -20), S(   0, -25), S(   8, -30),
    S( -54, -14), S( -15, -30), S(  -9, -31), S(  -2, -33),
    S( -27, -14), S(  -7, -12), S( -17, -15), S(  -4, -24),
    S( -14,   0), S(  -5,   6), S(  -7,   2), S(   4,  -3),
    S(   0,  11), S(  14,   9), S(  24,   4), S(  35,   0),
    S(  -7,  22), S(  28,   9), S(   2,  20), S(  32,   5),
    S(   4,   9), S( -14,  16), S(   5,   8), S(  18,   6),
    S(  37,  20), S(  26,  23), S(   3,  28), S(  14,  24),
};

const int QueenPSQT32[32] = {
    S(   8, -53), S(  -8, -38), S(   0, -51), S(  15, -45),
    S(   9, -39), S(  23, -57), S(  26, -73), S(  16, -25),
    S(   8, -20), S(  24, -18), S(   6,   4), S(   6,   2),
    S(   9,   0), S(  13,  16), S(  -1,  18), S( -17,  64),
    S(  -8,  21), S(  -6,  40), S( -14,  20), S( -31,  71),
    S( -23,  27), S( -14,  19), S( -20,  18), S( -11,  23),
    S(  -7,  26), S( -62,  62), S(  -9,  15), S( -42,  53),
    S( -15,   9), S(  12,   0), S(   5,   1), S( -11,  14),
};

const int KingPSQT32[32] = {
    S(  41, -82), S(  42, -52), S(  -9, -14), S( -26, -22),
    S(  32, -35), S(   0, -25), S( -34,   2), S( -48,   4),
    S(  13, -36), S(  24, -33), S(  21,  -8), S(  -3,   6),
    S(   0, -36), S(  82, -40), S(  40,   0), S(  -5,  18),
    S(   1, -19), S(  90, -26), S(  40,  12), S(  -2,  21),
    S(  46, -18), S( 113, -13), S(  90,  11), S(  33,  10),
    S(   6, -40), S(  46,  -2), S(  32,  12), S(   8,   6),
    S(  12, -92), S(  77, -49), S( -18,  -7), S( -16,  -6),
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
