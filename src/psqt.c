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

int PawnPSQT32[32][PHASE_NB] = {
    { -15,    0}, { -10,    0}, {   0,    0}, {   5,    0},
    { -14,   -1}, {  -3,    0}, {   8,    0}, {   3,    2},
    { -13,   -1}, {  -4,    0}, {   1,   -2}, {   9,    4},
    { -18,    0}, { -14,    0}, {  -1,   -5}, {  11,   -8},
    { -14,    2}, { -11,    0}, {  -5,   -8}, {   1,  -17},
    { -10,    6}, {  -8,    2}, {  -5,   -6}, {  -5,  -15},
    { -11,   16}, {  -9,   10}, {  -1,    4}, {   3,    2},
    { -15,    0}, { -10,    0}, {   0,    0}, {   5,    0},
};

int KnightPSQT32[32][PHASE_NB] = {
    { -51,  -41}, { -44,  -33}, { -31,  -22}, { -26,  -17},
    { -38,  -32}, { -27,  -21}, { -10,  -10}, {  -8,   -4},
    { -26,  -23}, {  -8,   -9}, {   4,   -1}, {  11,    8},
    { -22,  -21}, {   0,   -6}, {  14,    5}, {  17,   10},
    {  -6,  -16}, {   8,   -4}, {  17,    4}, {  28,   10},
    {  -5,  -20}, {   4,  -11}, {  14,    0}, {  23,    5},
    { -19,  -30}, { -10,  -21}, {   0,  -11}, {   4,   -5},
    {-136,  -42}, { -25,  -31}, { -15,  -21}, { -11,  -17},
};

int BishopPSQT32[32][PHASE_NB] = {
    { -15,  -14}, { -14,  -10}, { -12,  -11}, { -14,   -6},
    {  -6,  -13}, {   6,   -3}, {   2,   -6}, {  -9,    0},
    {  -5,   -8}, {   0,   -4}, {   3,   -1}, {   4,   -1},
    {  -2,   -7}, {  -3,   -2}, {  -6,   -3}, {  10,    2},
    {  -5,   -4}, {  -2,   -1}, {  -1,   -1}, {   9,    3},
    {  -1,   -6}, {  -2,   -4}, {   2,   -2}, {   0,    0},
    {  -9,  -12}, {  -1,   -7}, {  -5,   -6}, {  -2,   -3},
    { -10,  -20}, {  -9,  -14}, {  -6,  -10}, {  -5,   -7},
};

int RookPSQT32[32][PHASE_NB] = {
    { -12,   -8}, {   0,   -2}, {  -7,  -11}, {  -2,   -6},
    { -15,  -10}, {  -7,   -4}, {  -2,   -5}, {  -7,   -8},
    {  -7,   -8}, {  -1,   -2}, {  -3,   -3}, {  -4,   -5},
    {  -4,   -5}, {  -1,    0}, {  -1,   -1}, {  -2,   -1},
    {   0,    0}, {   0,    0}, {   0,    0}, {   2,    2},
    {   0,    0}, {   1,    2}, {   0,    0}, {   3,    2},
    {  -1,   -3}, {   0,    0}, {  -2,   -3}, {   0,    0},
    {  -1,   -1}, {  -1,    0}, {   2,    4}, {   0,    0},
};

int QueenPSQT32[32][PHASE_NB] = {
    {  -6,  -25}, { -11,  -18}, { -19,  -20}, { -19,  -16},
    {  -5,  -18}, {  -6,  -12}, {  -4,  -14}, { -11,  -12},
    {  -4,  -15}, {  -9,   -9}, {  -6,   -5}, {  -8,   -4},
    {  -6,  -10}, {  -1,   -3}, {  -3,    0}, {   1,    4},
    {   4,   -7}, {   1,    0}, {   1,    0}, {   6,    8},
    {   7,  -10}, {   0,   -5}, {   3,    0}, {   5,    4},
    {   0,  -16}, {  -7,  -10}, {  -2,   -6}, {   0,    0},
    {  -2,  -25}, {  -1,  -17}, {   0,  -11}, {  -1,   -9},
};

int KingPSQT32[32][PHASE_NB] = {
    {  52,  -67}, {  81,  -38}, {  14,  -41}, {   1,  -28},
    {  36,  -37}, {  38,  -19}, {  10,  -17}, {  -9,   -5},
    {   8,  -33}, {  18,  -12}, {  -3,   -5}, { -22,    8},
    {  -1,  -24}, {   8,   -4}, { -10,    7}, { -31,   16},
    {  -9,  -20}, {   0,    0}, { -20,    9}, { -40,   16},
    { -18,  -30}, {  -9,   -8}, { -29,    1}, { -50,    8},
    { -29,  -43}, { -19,  -20}, { -39,  -10}, { -59,    0},
    { -39,  -69}, { -29,  -45}, { -49,  -34}, { -69,  -22},
};

void initalizePSQT(){
    
    int sq, i, j;
    
    static const int table[8] = {0, 1, 2, 3, 3, 2, 1, 0};
    
    static const int inv[SQUARE_NB] = {
        56,  57,  58,  59,  60,  61,  62,  63,
        48,  49,  50,  51,  52,  53,  54,  55,
        40,  41,  42,  43,  44,  45,  46,  47,
        32,  33,  34,  35,  36,  37,  38,  39,
        24,  25,  26,  27,  28,  29,  30,  31,
        16,  17,  18,  19,  20,  21,  22,  23,
        8,   9,  10,  11,  12,  13,  14,  15,
        0,   1,   2,   3,   4,   5,   6,   7
    };
    
    for (sq = 0; sq < SQUARE_NB; sq++){
        
        i = 4 * (sq / 8) + table[sq % 8];
        j = 4 * (inv[sq] / 8) + table[inv[sq] % 8];
        
        PSQTMidgame[WHITE_PAWN  ][sq] = + PawnValue   +   PawnPSQT32[i][MG];
        PSQTEndgame[WHITE_PAWN  ][sq] = + PawnValue   +   PawnPSQT32[i][EG];
        PSQTMidgame[WHITE_KNIGHT][sq] = + KnightValue + KnightPSQT32[i][MG];
        PSQTEndgame[WHITE_KNIGHT][sq] = + KnightValue + KnightPSQT32[i][EG];
        PSQTMidgame[WHITE_BISHOP][sq] = + BishopValue + BishopPSQT32[i][MG];
        PSQTEndgame[WHITE_BISHOP][sq] = + BishopValue + BishopPSQT32[i][EG];
        PSQTMidgame[WHITE_ROOK  ][sq] = + RookValue   +   RookPSQT32[i][MG];
        PSQTEndgame[WHITE_ROOK  ][sq] = + RookValue   +   RookPSQT32[i][EG];
        PSQTMidgame[WHITE_QUEEN ][sq] = + QueenValue  +  QueenPSQT32[i][MG];
        PSQTEndgame[WHITE_QUEEN ][sq] = + QueenValue  +  QueenPSQT32[i][EG];
        PSQTMidgame[WHITE_KING  ][sq] = + KingValue   +   KingPSQT32[i][MG];
        PSQTEndgame[WHITE_KING  ][sq] = + KingValue   +   KingPSQT32[i][EG];
        
        PSQTMidgame[BLACK_PAWN  ][sq] = - PawnValue   -   PawnPSQT32[j][MG];
        PSQTEndgame[BLACK_PAWN  ][sq] = - PawnValue   -   PawnPSQT32[j][EG];
        PSQTMidgame[BLACK_KNIGHT][sq] = - KnightValue - KnightPSQT32[j][MG];
        PSQTEndgame[BLACK_KNIGHT][sq] = - KnightValue - KnightPSQT32[j][EG];
        PSQTMidgame[BLACK_BISHOP][sq] = - BishopValue - BishopPSQT32[j][MG];
        PSQTEndgame[BLACK_BISHOP][sq] = - BishopValue - BishopPSQT32[j][EG];
        PSQTMidgame[BLACK_ROOK  ][sq] = - RookValue   -   RookPSQT32[j][MG];
        PSQTEndgame[BLACK_ROOK  ][sq] = - RookValue   -   RookPSQT32[j][EG];
        PSQTMidgame[BLACK_QUEEN ][sq] = - QueenValue  -  QueenPSQT32[j][MG];
        PSQTEndgame[BLACK_QUEEN ][sq] = - QueenValue  -  QueenPSQT32[j][EG];
        PSQTMidgame[BLACK_KING  ][sq] = - KingValue   -   KingPSQT32[j][MG];
        PSQTEndgame[BLACK_KING  ][sq] = - KingValue   -   KingPSQT32[j][EG];
    }
}