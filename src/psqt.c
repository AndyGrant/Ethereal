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

int PSQTMidgame[32][SQUARE_NB];
int PSQTEndgame[32][SQUARE_NB];

const int PawnPSQT32[32][PHASE_NB] = {
    {   0,   0}, {   0,   0}, {   0,   0}, {   0,   0},
    { -23,   2}, {  12,   3}, {  -5,   6}, {  -6,  -2},
    { -25,   0}, {  -3,  -1}, {  -5,  -6}, {  -3, -10},
    { -21,   7}, {  -6,   6}, {   3,  -9}, {   3, -22},
    { -12,  14}, {   3,   8}, {  -1,  -3}, {   4, -23},
    {   3,  25}, {  14,  23}, {  18,   3}, {  16, -23},
    { -40,   6}, { -32,   9}, {   2, -17}, {   4, -33},
    {   0,   0}, {   0,   0}, {   0,   0}, {   0,   0},
};

const int KnightPSQT32[32][PHASE_NB] = {
    { -33, -46}, {   4, -40}, { -12, -12}, {   9,  -8},
    {  10, -50}, {   2, -12}, {  17, -24}, {  20,  -3},
    {   5, -18}, {  29, -18}, {  16,   5}, {  29,  15},
    {   7,  11}, {  27,   9}, {  31,  34}, {  38,  37},
    {  28,   9}, {  36,  15}, {  35,  41}, {  48,  42},
    { -23,   9}, {  38,   7}, {  42,  39}, {  50,  36},
    { -44, -18}, { -41,   7}, {  60, -26}, {  14,  -1},
    {-163, -29}, {-103, -30}, {-154,  -6}, { -55, -23},
};

const int BishopPSQT32[32][PHASE_NB] = {
    {  14, -21}, {  21, -21}, {   2, -10}, {  19, -14},
    {  31, -28}, {  33, -23}, {  23, -14}, {  10,  -2},
    {  25, -11}, {  32, -12}, {  25,   1}, {  19,   7},
    {  10,  -3}, {  13,   0}, {  11,  15}, {  33,  20},
    {  -9,  14}, {  23,   4}, {  10,  17}, {  35,  21},
    {  -6,  10}, {   8,   9}, {  32,   8}, {  22,   5},
    { -55,   6}, {  13,  -3}, {  -2, -10}, { -36,   3},
    { -35,  -1}, { -55,  -3}, {-144,   7}, {-124,  16},
};

const int RookPSQT32[32][PHASE_NB] = {
    {  -4, -32}, {  -7, -16}, {   5, -13}, {  11, -20},
    { -33, -25}, {  -7, -26}, {   0, -20}, {  10, -26},
    { -22, -21}, {   2, -13}, {   1, -21}, {   1, -20},
    { -23,  -1}, { -12,   5}, {  -7,   3}, {  -1,   2},
    { -15,  14}, {  -8,   9}, {  20,   8}, {  21,   8},
    { -14,  17}, {  18,  12}, {  20,  15}, {  22,  14},
    {   0,  18}, {  -4,  18}, {  40,   3}, {  22,   9},
    {  -1,  25}, {  17,  15}, { -19,  24}, {  11,  30},
};

const int QueenPSQT32[32][PHASE_NB] = {
    {  -3, -45}, { -13, -26}, {  -6, -17}, {  14, -39},
    {   3, -49}, {  12, -36}, {  19, -50}, {  14, -16},
    {   6, -22}, {  23, -19}, {   6,   3}, {   4,   3},
    {   4,  -5}, {   6,   3}, {   0,  11}, {  -5,  45},
    {  -9,  10}, { -12,  31}, {  -5,  20}, { -20,  50},
    {  -3,   3}, {   4,  21}, {   7,  18}, {  -6,  45},
    {   8,  15}, { -53,  55}, {  32,  12}, { -13,  70},
    { -18, -32}, {   5, -18}, {   0, -14}, { -10,  10},
};

const int KingPSQT32[32][PHASE_NB] = {
    {  78,-103}, {  88, -79}, {  37, -35}, {  20, -37},
    {  69, -53}, {  61, -45}, {  14,  -6}, { -16,   1},
    {  -1, -41}, {  46, -29}, {  20,  -1}, { -10,  14},
    { -52, -35}, {  29, -21}, {   7,  15}, { -46,  35},
    { -27, -19}, {  53,   1}, {   7,  30}, { -34,  37},
    {  38, -17}, {  82,   2}, {  63,  19}, {  -3,  17},
    {  25, -15}, {  60,  -2}, {  39,   4}, {  23,   7},
    {   1, -80}, {  97, -59}, { -12, -34}, { -21, -31},
};


void initializePSQT(){
    
    int sq, w32, b32;
    
    for (sq = 0; sq < SQUARE_NB; sq++){

        w32 = relativeSquare32(sq, WHITE);
        b32 = relativeSquare32(sq, BLACK);

        PSQTMidgame[WHITE_PAWN  ][sq] = + PieceValues[PAWN  ][MG] +   PawnPSQT32[w32][MG];
        PSQTEndgame[WHITE_PAWN  ][sq] = + PieceValues[PAWN  ][EG] +   PawnPSQT32[w32][EG];
        PSQTMidgame[WHITE_KNIGHT][sq] = + PieceValues[KNIGHT][MG] + KnightPSQT32[w32][MG];
        PSQTEndgame[WHITE_KNIGHT][sq] = + PieceValues[KNIGHT][EG] + KnightPSQT32[w32][EG];
        PSQTMidgame[WHITE_BISHOP][sq] = + PieceValues[BISHOP][MG] + BishopPSQT32[w32][MG];
        PSQTEndgame[WHITE_BISHOP][sq] = + PieceValues[BISHOP][EG] + BishopPSQT32[w32][EG];
        PSQTMidgame[WHITE_ROOK  ][sq] = + PieceValues[ROOK  ][MG] +   RookPSQT32[w32][MG];
        PSQTEndgame[WHITE_ROOK  ][sq] = + PieceValues[ROOK  ][EG] +   RookPSQT32[w32][EG];
        PSQTMidgame[WHITE_QUEEN ][sq] = + PieceValues[QUEEN ][MG] +  QueenPSQT32[w32][MG];
        PSQTEndgame[WHITE_QUEEN ][sq] = + PieceValues[QUEEN ][EG] +  QueenPSQT32[w32][EG];
        PSQTMidgame[WHITE_KING  ][sq] = + PieceValues[KING  ][MG] +   KingPSQT32[w32][MG];
        PSQTEndgame[WHITE_KING  ][sq] = + PieceValues[KING  ][EG] +   KingPSQT32[w32][EG];
        
        PSQTMidgame[BLACK_PAWN  ][sq] = - PieceValues[PAWN  ][MG] -   PawnPSQT32[b32][MG];
        PSQTEndgame[BLACK_PAWN  ][sq] = - PieceValues[PAWN  ][EG] -   PawnPSQT32[b32][EG];
        PSQTMidgame[BLACK_KNIGHT][sq] = - PieceValues[KNIGHT][MG] - KnightPSQT32[b32][MG];
        PSQTEndgame[BLACK_KNIGHT][sq] = - PieceValues[KNIGHT][EG] - KnightPSQT32[b32][EG];
        PSQTMidgame[BLACK_BISHOP][sq] = - PieceValues[BISHOP][MG] - BishopPSQT32[b32][MG];
        PSQTEndgame[BLACK_BISHOP][sq] = - PieceValues[BISHOP][EG] - BishopPSQT32[b32][EG];
        PSQTMidgame[BLACK_ROOK  ][sq] = - PieceValues[ROOK  ][MG] -   RookPSQT32[b32][MG];
        PSQTEndgame[BLACK_ROOK  ][sq] = - PieceValues[ROOK  ][EG] -   RookPSQT32[b32][EG];
        PSQTMidgame[BLACK_QUEEN ][sq] = - PieceValues[QUEEN ][MG] -  QueenPSQT32[b32][MG];
        PSQTEndgame[BLACK_QUEEN ][sq] = - PieceValues[QUEEN ][EG] -  QueenPSQT32[b32][EG];
        PSQTMidgame[BLACK_KING  ][sq] = - PieceValues[KING  ][MG] -   KingPSQT32[b32][MG];
        PSQTEndgame[BLACK_KING  ][sq] = - PieceValues[KING  ][EG] -   KingPSQT32[b32][EG];
    }
}
