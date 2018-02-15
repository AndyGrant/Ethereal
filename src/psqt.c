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
    { -15,   0}, { -10,   0}, {   0,   0}, {   5,   0},
    { -15,  -2}, {   4,   0}, {  -5,   0}, {  -7, -11},
    { -17,  -3}, {  -2,  -2}, {  -8,  -5}, {  -8, -12},
    { -15,   1}, {  -5,   1}, {   0,  -7}, {   2, -18},
    {  -8,   5}, {  -1,   4}, {   3,  -5}, {   6, -16},
    {   4,   3}, {  12,   3}, {  22,  -3}, {  15, -16},
    {  -4, -37}, {   3, -12}, {  18, -17}, {  30, -18},
    { -15,   0}, { -10,   0}, {   0,   0}, {   5,   0},
};


const int KnightPSQT32[32][PHASE_NB] = {
    { -50, -88}, { -25, -57}, { -36, -48}, { -22, -42},
    { -22, -50}, { -30, -43}, { -22, -46}, {  -7, -37},
    { -26, -44}, { -11, -37}, { -13, -30}, {   1, -23},
    { -15, -28}, {  -9, -27}, {  -4, -19}, {   0, -10},
    {  -4, -20}, {   0, -19}, {   6,  -8}, {  11,  -6},
    { -17, -17}, {   2, -22}, {   7,  -7}, {  23,  -8},
    { -25, -33}, { -23, -26}, {  20, -39}, {  -5, -28},
    {-100, -53}, { -47, -31}, { -84, -50}, { -42, -31},
};

const int BishopPSQT32[32][PHASE_NB] = {
    {  -3, -21}, {  -7, -21}, { -20, -22}, { -14, -30},
    {   9, -39}, {   4, -31}, {   0, -27}, { -13, -20},
    {  -3, -25}, {  10, -24}, {  -5, -20}, {  -2, -18},
    {   0, -19}, { -14, -16}, {  -9, -11}, {   3, -13},
    { -31, -13}, { -11,  -9}, {  -6, -12}, {   5,  -9},
    { -15, -11}, {  -5, -15}, {   3,  -9}, {  -1, -14},
    { -45, -17}, { -18, -18}, { -14, -22}, { -32, -15},
    { -42, -27}, { -39, -27}, { -70, -27}, { -73, -25},
};

const int RookPSQT32[32][PHASE_NB] = {
    { -22, -29}, { -25, -26}, { -10, -12}, {  -7, -24},
    { -44, -29}, { -25, -31}, { -26, -27}, { -14, -30},
    { -29, -20}, { -19, -20}, { -20, -22}, { -19, -22},
    { -23,  -5}, { -14,  -1}, { -15,  -7}, { -17,  -7},
    { -11,   5}, { -13,   0}, {   5,   1}, {   4,   0},
    { -10,   3}, {  -4,  -1}, {   6,  -1}, {   8,   2},
    {  -9,   0}, { -12,  -3}, {  11, -12}, {   2,   0},
    {  -9,  17}, {  -2,   6}, { -27,   2}, {  -9,   8},
};

const int QueenPSQT32[32][PHASE_NB] = {
    { -36, -60}, { -43, -59}, { -43, -40}, { -25, -65},
    { -28, -56}, { -24, -61}, { -21, -83}, { -25, -46},
    { -23, -44}, { -13, -40}, { -26, -37}, { -31, -30},
    { -28, -30}, { -25, -32}, { -33, -29}, { -34, -11},
    { -32, -12}, { -34, -13}, { -24, -16}, { -33,  -5},
    { -20, -20}, { -22, -21}, { -12, -18}, { -25,  -4},
    { -20, -19}, { -58, -12}, { -11, -31}, { -39,  -7},
    { -40, -54}, { -33, -43}, { -50, -55}, { -48, -41},
};

const int KingPSQT32[32][PHASE_NB] = {
    {  44, -70}, {  58, -57}, {  33, -24}, {  11, -34},
    {  42, -31}, {  40, -22}, {  13,  -1}, {  -8,   1},
    {   1, -19}, {  25, -11}, {  12,   5}, {  -6,  13},
    { -14, -22}, {  19,  -4}, {   6,  10}, { -29,  18},
    {  -1, -12}, {  37,   2}, {  -2,   7}, { -39,   4},
    {  27,  -3}, {  42,  -5}, {  23,   1}, { -12,  -2},
    {  11, -14}, {  32,  -8}, {  41,   9}, {  39,   5},
    { -52, -80}, {  49, -13}, {  13, -10}, {  28,   3},
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
