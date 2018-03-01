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
    { -18,  -3}, {   4,  -2}, {  -6,   0}, {  -8,  -7},
    { -18,  -4}, {  -5,  -5}, {  -7,  -8}, {  -7, -10},
    { -17,   0}, {  -7,   0}, {  -2,  -9}, {  -1, -17},
    { -12,   5}, {  -1,   1}, {  -3,  -6}, {  -1, -18},
    {  -1,  11}, {   7,  10}, {  14,  -2}, {   7, -19},
    { -17,   2}, {  -7,   3}, {   6, -14}, {   6, -23},
    { -15,   0}, { -10,   0}, {   0,   0}, {   5,   0}
};

const int KnightPSQT32[32][PHASE_NB] = {
    { -36, -60}, { -18, -59}, { -32, -41}, { -20, -40},
    { -14, -64}, { -27, -43}, { -10, -49}, { -10, -36},
    { -19, -44}, {  -2, -45}, { -10, -30}, {  -5, -24},
    { -16, -25}, {  -6, -29}, {  -3, -12}, {   2, -11},
    {  -3, -27}, {   3, -24}, {  -4,  -9}, {   9,  -8},
    { -35, -29}, {   9, -29}, {   7,  -9}, {   9, -11},
    { -50, -43}, { -47, -30}, {  24, -47}, { -11, -32},
    {-112, -49}, { -76, -51}, {-110, -35}, { -49, -45}
};

const int BishopPSQT32[32][PHASE_NB] = {
    { -16, -34}, {  -6, -31}, { -16, -29}, {  -6, -30},
    {  -3, -38}, {   3, -35}, {  -4, -30}, { -12, -22},
    {  -3, -28}, {   3, -28}, {  -1, -20}, {  -4, -16},
    { -14, -22}, {  -9, -20}, {  -9, -11}, {   0,  -8},
    { -21, -12}, {  -5, -19}, { -10, -10}, {   5,  -8},
    { -23, -15}, {  -6, -15}, {   5, -15}, {  -5, -18},
    { -45, -16}, {  -6, -24}, { -16, -26}, { -40, -18},
    { -37, -23}, { -50, -23}, {-105, -17}, { -90, -10}
};

const int RookPSQT32[32][PHASE_NB] = {
    { -17, -30}, { -19, -19}, { -11, -17}, {  -7, -22},
    { -34, -25}, { -21, -25}, { -18, -23}, {  -8, -26},
    { -34, -24}, { -17, -18}, { -14, -24}, { -15, -22},
    { -33, -11}, { -22,  -6}, { -20,  -8}, { -17,  -9},
    { -28,   0}, { -19,  -5}, {  -2,  -4}, {  -2,  -5},
    { -22,   2}, {  -3,  -1}, {   0,   0}, {  -5,  -1},
    { -12,   3}, { -14,   2}, {  10,  -8}, {  -2,  -5},
    { -14,   8}, {   0,   1}, { -24,   6}, {  -3,  10}
};

const int QueenPSQT32[32][PHASE_NB] = {
    { -34, -67}, { -41, -54}, { -37, -49}, { -23, -63},
    { -33, -71}, { -26, -62}, { -19, -70}, { -24, -50},
    { -28, -53}, { -16, -52}, { -30, -40}, { -28, -38},
    { -31, -44}, { -32, -40}, { -31, -35}, { -36, -14},
    { -36, -34}, { -39, -23}, { -34, -30}, { -44, -12},
    { -26, -38}, { -28, -27}, { -25, -31}, { -33, -14},
    { -19, -30}, { -55,  -7}, { -10, -33}, { -30,   5},
    { -34, -62}, { -30, -53}, { -38, -53}, { -32, -33}
};

const int KingPSQT32[32][PHASE_NB] = {
    {  45, -62}, {  53, -48}, {  22, -22}, {  11, -22},
    {  42, -32}, {  40, -27}, {  14,  -4}, {  -4,   0},
    {  -1, -25}, {  30, -17}, {  15,  -1}, {  -3,   8},
    { -32, -23}, {  14, -15}, {   6,   9}, { -29,  20},
    { -24, -14}, {  30,   0}, {   6,  18}, { -22,  22},
    {  22, -11}, {  50,   3}, {  37,  12}, {  -3,  11},
    {  13, -11}, {  44,   0}, {  31,   7}, {  30,  10},
    {   1, -46}, {  71, -31}, {   4, -18}, {  -5, -14}
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
