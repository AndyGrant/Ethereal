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
    { -16,  -3}, {   5,  -3}, {  -7,   0}, {  -6,  -8},
    { -17,  -4}, {  -4,  -4}, { -10,  -9}, {  -8, -12},
    { -19,   0}, {  -7,   0}, {  -2, -10}, {   1, -19},
    { -12,   5}, {  -1,   2}, {  -1,  -6}, {   3, -19},
    {  -2,   9}, {   7,   9}, {  17,  -5}, {   6, -19},
    {  -1,  -1}, {   4,   4}, {  11, -20}, {  14, -25},
    { -15,   0}, { -10,   0}, {   0,   0}, {   5,   0}
};

const int KnightPSQT32[32][PHASE_NB] = {
    { -27, -57}, { -24, -53}, { -29, -36}, { -20, -40},
    { -17, -58}, { -32, -42}, { -14, -43}, {  -8, -38},
    { -21, -38}, {  -7, -39}, {  -9, -31}, {   0, -24},
    { -16, -25}, {  -9, -30}, {  -4, -16}, {  -1, -12},
    {  -6, -23}, {  -1, -25}, {   0, -10}, {  11,  -7},
    { -32, -30}, {   2, -27}, {   5,  -8}, {  11, -14},
    { -48, -45}, { -39, -31}, {  20, -54}, { -17, -35},
    {-106, -58}, { -66, -50}, { -97, -51}, { -44, -42}
};

const int BishopPSQT32[32][PHASE_NB] = {
    { -15, -30}, { -14, -28}, { -20, -20}, {  -9, -26},
    {   6, -37}, {   7, -32}, {  -7, -30}, { -12, -21},
    {  -2, -27}, {   7, -27}, {  -3, -20}, {  -2, -15},
    {  -5, -24}, { -12, -21}, {  -9, -11}, {   3, -10},
    { -32, -13}, { -10, -17}, {  -9, -11}, {   6,  -7},
    { -23, -15}, {  -8, -20}, {   3, -14}, {  -5, -17},
    { -58, -19}, { -14, -22}, { -17, -26}, { -44, -24},
    { -39, -33}, { -45, -27}, { -86, -35}, { -81, -23}
};

const int RookPSQT32[32][PHASE_NB] = {
    { -21, -29}, { -24, -22}, {  -8, -10}, {  -5, -22},
    { -42, -20}, { -22, -27}, { -23, -21}, {  -9, -26},
    { -32, -19}, { -19, -18}, { -21, -24}, { -18, -21},
    { -27, -10}, { -23,  -8}, { -23, -15}, { -19, -12},
    { -23,  -4}, { -23,  -8}, {  -3,  -6}, {  -7,  -9},
    { -22,  -5}, {  -9,  -5}, {  -1, -10}, {  -5,  -8},
    { -11,  -2}, { -10,  -3}, {   8, -17}, {  -3, -10},
    { -23,   5}, {  -7,   0}, { -36,  -5}, { -12,   4}
};

const int QueenPSQT32[32][PHASE_NB] = {
    { -37, -62}, { -41, -56}, { -41, -38}, { -22, -60},
    { -29, -62}, { -25, -66}, { -19, -78}, { -25, -48},
    { -22, -47}, { -14, -48}, { -28, -43}, { -32, -35},
    { -29, -34}, { -29, -39}, { -34, -35}, { -36, -17},
    { -35, -22}, { -40, -25}, { -29, -30}, { -38, -14},
    { -29, -39}, { -27, -33}, { -17, -33}, { -27, -15},
    { -27, -36}, { -60, -24}, { -13, -49}, { -36, -11},
    { -48, -63}, { -37, -51}, { -57, -67}, { -51, -44}
};

const int KingPSQT32[32][PHASE_NB] = {
    {  42, -66}, {  56, -55}, {  31, -22}, {  11, -24},
    {  42, -31}, {  39, -25}, {  12,  -3}, {  -7,   1},
    {   2, -21}, {  26, -14}, {  11,   1}, {  -6,  11},
    { -14, -25}, {  16, -11}, {   5,   8}, { -30,  19},
    {  -5, -17}, {  35,  -2}, {   5,  13}, { -27,  17},
    {  22, -11}, {  54,   3}, {  31,   8}, {  -5,   6},
    {   5, -15}, {  35,  -5}, {  31,   1}, {  39,   0},
    { -19, -46}, {  54, -27}, {   2, -26}, {   9, -14}
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
