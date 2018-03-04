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
    { -23,   2}, {  11,   3}, {  -4,   6}, {  -7,  -4},
    { -23,   0}, {  -2,  -1}, {  -5,  -6}, {  -5,  -9},
    { -21,   6}, {  -5,   6}, {   2,  -8}, {   4, -20},
    { -13,  13}, {   4,   7}, {   1,  -3}, {   4, -22},
    {   4,  23}, {  15,  21}, {  26,   3}, {  15, -23},
    { -21,   9}, {  -5,  10}, {  14, -15}, {  14, -29},
    {   0,   0}, {   0,   0}, {   0,   0}, {   0,   0},
};

const int KnightPSQT32[32][PHASE_NB] = {
    { -23, -40}, {   5, -39}, { -17, -11}, {   2,  -9},  
    {  12, -47}, {  -9, -14}, {  18, -23}, {  18,  -3},  
    {   4, -15}, {  30, -17}, {  18,   7}, {  26,  16},  
    {   8,  14}, {  24,   8}, {  29,  35}, {  36,  36},  
    {  29,  11}, {  37,  16}, {  27,  39}, {  47,  41},  
    { -21,   8}, {  47,   8}, {  43,  39}, {  47,  36},  
    { -45, -14}, { -40,   7}, {  70, -20}, {  16,   3},  
    {-142, -23}, { -85, -26}, {-138,  -1}, { -43, -17},  
};

const int BishopPSQT32[32][PHASE_NB] = {
    {   2, -20}, {  18, -15}, {   2, -12}, {  18, -13},  
    {  23, -26}, {  31, -21}, {  21, -13}, {   9,  -1},  
    {  23, -10}, {  31, -10}, {  26,   2}, {  21,   8},  
    {   6,  -1}, {  13,   2}, {  13,  16}, {  27,  21},  
    {  -5,  15}, {  20,   4}, {  12,  18}, {  34,  21},  
    {  -8,  10}, {  18,  10}, {  34,  10}, {  20,   5},  
    { -43,   8}, {  18,  -4}, {   2,  -7}, { -35,   5},  
    { -30,  -2}, { -51,  -2}, {-137,   7}, {-113,  18},  
};

const int RookPSQT32[32][PHASE_NB] = {
    {  -4, -31}, {  -7, -14}, {   5, -11}, {  12, -19},  
    { -31, -24}, { -10, -24}, {  -6, -20}, {  10, -25},  
    { -31, -22}, {  -4, -13}, {   1, -22}, {  -1, -19},  
    { -29,  -2}, { -12,   6}, {  -9,   3}, {  -4,   1},  
    { -21,  15}, {  -7,   8}, {  19,   9}, {  19,   8},  
    { -12,  18}, {  18,  14}, {  22,  15}, {  15,  14},  
    {   4,  19}, {   1,  18}, {  37,   3}, {  19,   8},  
    {   1,  27}, {  22,  16}, { -15,  24}, {  18,  30},  
};

const int QueenPSQT32[32][PHASE_NB] = {
    {  -5, -42}, { -16, -22}, {  -9, -14}, {  13, -36},  
    {  -3, -48}, {   8, -34}, {  19, -47}, {  11, -16},  
    {   5, -20}, {  23, -19}, {   2,   0}, {   5,   3},  
    {   0,  -6}, {  -2,   0}, {   0,   8}, {  -8,  41},  
    {  -8,   9}, { -12,  27}, {  -5,  16}, { -20,  44},  
    {   8,   3}, {   5,  20}, {   9,  14}, {  -3,  41},  
    {  19,  16}, { -37,  52}, {  33,  11}, {   2,  69},  
    {  -5, -34}, {   2, -20}, { -11, -20}, {  -2,  11},  
};

const int KingPSQT32[32][PHASE_NB] = {
    {  70, -96}, {  82, -75}, {  34, -34}, {  17, -34},
    {  65, -50}, {  62, -42}, {  21,  -6}, {  -6,   0},
    {  -1, -39}, {  46, -26}, {  23,  -1}, {  -4,  12},
    { -50, -35}, {  21, -23}, {   9,  14}, { -45,  31},
    { -37, -21}, {  46,   0}, {   9,  28}, { -34,  34},
    {  34, -17}, {  78,   4}, {  57,  18}, {  -4,  17},
    {  20, -17}, {  68,   0}, {  48,  10}, {  46,  15},
    {   1, -71}, { 110, -48}, {   6, -28}, {  -7, -21},
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
