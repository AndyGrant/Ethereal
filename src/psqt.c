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
    { -15,  -4}, {   5,  -3}, {  -7,   0}, {  -6,  -9},
    { -17,  -3}, {  -5,  -4}, { -10,  -9}, {  -7, -12},
    { -18,   0}, {  -5,   0}, {  -2, -10}, {   0, -19},
    { -13,   5}, {  -1,   2}, {  -1,  -5}, {   1, -18},
    {  -5,   9}, {   7,  10}, {  14,  -2}, {   3, -17},
    { -14,  -1}, {  -4,   4}, {   7, -16}, {  12, -24},
    { -15,   0}, { -10,   0}, {   0,   0}, {   5,   0},
};

const int KnightPSQT32[32][PHASE_NB] = {
    { -29, -61}, { -24, -55}, { -29, -38}, { -17, -39},
    { -17, -58}, { -24, -42}, { -14, -47}, {  -9, -39},
    { -19, -40}, {  -6, -40}, { -10, -32}, {   1, -25},
    { -16, -28}, {  -9, -31}, {  -3, -18}, {   0, -12},
    {  -7, -25}, {   0, -26}, {   0,  -9}, {   9,  -8},
    { -29, -30}, {  -1, -28}, {   8,  -9}, {  12, -14},
    { -55, -49}, { -42, -33}, {  17, -55}, { -14, -39},
    {-115, -59}, { -85, -49}, {-114, -56}, { -51, -45},
};

const int BishopPSQT32[32][PHASE_NB] = {
    {  -9, -31}, {  -8, -30}, { -18, -20}, {  -6, -27},
    {   9, -39}, {   4, -34}, {  -5, -32}, { -11, -21},
    {  -1, -28}, {   5, -29}, {  -3, -21}, {  -3, -16},
    {  -8, -26}, { -11, -21}, {  -9, -12}, {   3, -10},
    { -30, -13}, {  -8, -17}, { -10, -13}, {   4,  -9},
    { -23, -14}, { -10, -21}, {   3, -14}, {  -4, -17},
    { -60, -18}, { -16, -23}, { -16, -29}, { -44, -26},
    { -38, -33}, { -46, -29}, {-109, -33}, { -91, -27},
};

const int RookPSQT32[32][PHASE_NB] = {
    { -19, -30}, { -25, -26}, { -11, -15}, {  -7, -25},
    { -40, -26}, { -24, -30}, { -23, -26}, { -11, -30},
    { -31, -20}, { -18, -21}, { -19, -24}, { -17, -21},
    { -23,  -8}, { -17,  -4}, { -17,  -9}, { -14,  -9},
    { -15,   0}, { -15,  -2}, {   5,  -2}, {   2,  -2},
    { -16,   0}, {  -1,  -3}, {   8,  -5}, {   4,  -1},
    {  -7,   1}, { -11,  -2}, {  15, -14}, {   6,  -4},
    { -13,   8}, {   0,   2}, { -25,  -1}, {  -2,   7},
};

const int QueenPSQT32[32][PHASE_NB] = {
    { -38, -66}, { -42, -62}, { -42, -40}, { -25, -62},
    { -32, -70}, { -24, -68}, { -21, -81}, { -26, -50},
    { -26, -52}, { -16, -51}, { -29, -45}, { -33, -38},
    { -30, -39}, { -30, -42}, { -36, -38}, { -36, -19},
    { -38, -27}, { -42, -26}, { -30, -33}, { -39, -18},
    { -32, -44}, { -27, -38}, { -18, -36}, { -31, -19},
    { -24, -40}, { -58, -24}, { -11, -51}, { -39, -13},
    { -41, -60}, { -32, -53}, { -42, -59}, { -41, -44},
};

const int KingPSQT32[32][PHASE_NB] = {
    {  42, -66}, {  55, -56}, {  29, -21}, {  11, -24},
    {  43, -30}, {  39, -25}, {  12,  -3}, {  -7,   1},
    {   0, -22}, {  26, -14}, {  10,   0}, {  -4,  12},
    { -28, -27}, {  17,  -9}, {   6,   8}, { -26,  21},
    { -13, -17}, {  35,   0}, {   7,  14}, { -20,  19},
    {  30,  -8}, {  51,   3}, {  27,   8}, {   2,  11},
    {  10, -14}, {  37,  -5}, {  27,   0}, {  22,  -2},
    { -11, -50}, {  61, -24}, {   2, -21}, {   9, -13},
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
