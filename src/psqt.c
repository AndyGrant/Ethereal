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
    { -17,  -3}, {   7,  -2}, {  -9,   0}, {  -7,  -7},
    { -18,  -4}, {  -4,  -4}, { -11,  -9}, {  -8, -12},
    { -18,   0}, {  -5,   0}, {   0, -12}, {   1, -20},
    { -10,   6}, {  -1,   3}, {  -1,  -6}, {   2, -20},
    {  -3,  13}, {   7,  14}, {  11,  -1}, {   8, -15},
    { -25,   8}, { -18,   8}, {  -1, -12}, {   4, -22},
    { -15,   0}, { -10,   0}, {   0,   0}, {   5,   0}
};

const int KnightPSQT32[32][PHASE_NB] = {
    { -41, -64}, { -23, -52}, { -23, -36}, { -12, -39},
    { -15, -57}, { -10, -38}, { -13, -47}, {  -8, -37},
    { -16, -41}, {  -4, -42}, { -10, -32}, {   0, -25},
    { -13, -30}, {  -5, -28}, {  -2, -18}, {   1, -13},
    {  -2, -24}, {  -1, -24}, {   4,  -9}, {   7,  -7},
    { -38, -26}, {  -3, -27}, {   7, -10}, {  12, -12},
    { -35, -45}, { -30, -29}, {  38, -55}, {  -9, -39},
    {-101, -59}, { -77, -50}, {-100, -47}, { -73, -45}
};

const int BishopPSQT32[32][PHASE_NB] = {
    {  -1, -32}, {  -3, -36}, { -19, -18}, {  -6, -28},
    {  14, -42}, {   6, -33}, {  -1, -31}, {  -9, -23},
    {   2, -31}, {   5, -29}, {  -2, -20}, {  -5, -15},
    {  -6, -29}, { -11, -24}, {  -9, -13}, {   6, -12},
    { -29, -15}, {  -7, -18}, { -12, -13}, {   0,  -7},
    { -24, -18}, { -16, -17}, {   0, -15}, {  -2, -16},
    { -65, -17}, {  -4, -23}, { -14, -31}, { -41, -26},
    { -32, -31}, { -63, -26}, { -96, -31}, {-102, -21}
};

const int RookPSQT32[32][PHASE_NB] = {
    { -21, -29}, { -23, -24}, {  -7, -11}, {  -5, -22},
    { -37, -20}, { -19, -27}, { -18, -21}, {  -8, -26},
    { -27, -17}, { -12, -19}, { -22, -21}, { -14, -23},
    { -27, -10}, { -26,  -9}, { -23, -14}, { -15, -10},
    { -24,  -2}, { -23,  -5}, {  -5,  -8}, {  -8,  -7},
    { -25,  -3}, { -15,  -3}, {  -5,  -6}, {  -3,  -7},
    { -14,  -1}, { -16,   0}, {  17, -13}, {   0,  -7},
    { -31,   3}, { -21,  -1}, { -42,  -5}, { -23,   0}
};

const int QueenPSQT32[32][PHASE_NB] = {
    { -39, -64}, { -44, -60}, { -37, -41}, { -25, -64},
    { -22, -64}, { -23, -68}, { -21, -81}, { -25, -48},
    { -26, -52}, { -16, -52}, { -30, -43}, { -33, -37},
    { -29, -39}, { -30, -38}, { -37, -36}, { -35, -19},
    { -41, -24}, { -39, -24}, { -35, -29}, { -40, -13},
    { -33, -39}, { -28, -32}, { -14, -34}, { -28, -17},
    { -32, -37}, { -43, -23}, {  -8, -44}, { -44, -15},
    { -44, -48}, { -30, -40}, { -29, -45}, { -59, -40}
};

const int KingPSQT32[32][PHASE_NB] = {
    {  43, -66}, {  51, -56}, {  28, -21}, {  14, -25},
    {  43, -33}, {  40, -26}, {  10,  -3}, {  -9,   1},
    {   4, -22}, {  28, -16}, {  13,   1}, {  -4,  11},
    { -11, -21}, {  26,  -8}, {   4,   9}, { -26,  23},
    {   3, -13}, {  43,   0}, {  10,  16}, { -13,  23},
    {  32,  -6}, {  56,   4}, {  40,  11}, {  19,  14},
    {   4,  -7}, {  42,   2}, {  32,  -3}, {  19,  -2},
    {  18, -35}, {  57, -36}, {   7, -23}, {  -5, -21}
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
