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
    { -13,  -2}, {   3,   0}, {   2,   0}, {  -2,  -3},
    { -16,  -4}, {  -4,  -3}, {  -6,  -5}, {  -7, -10},
    { -18,   1}, { -16,   0}, {  -3,  -8}, {   5, -14},
    { -11,   7}, { -12,   2}, {  -3,  -5}, {   7, -14},
    {  -3,  12}, {  -1,   8}, {   5,  -5}, {   3, -17},
    {  -4,  22}, {  -9,  22}, {  -4,   0}, {  14,   2},
    { -15,   0}, { -10,   0}, {   0,   0}, {   5,   0},
};

const int KnightPSQT32[32][PHASE_NB] = {
    { -40, -47}, { -28, -42}, { -24, -24}, { -19, -24},
    { -22, -37}, { -30, -31}, { -18, -28}, {  -7, -19},
    { -22, -28}, { -11, -21}, {  -1, -12}, {   6,  -8},
    { -16, -21}, { -10, -24}, {  -1, -13}, {   2,  -4},
    {  -2, -19}, {   2, -15}, {   5,  -8}, {  19,  -1},
    { -17, -29}, {   4, -22}, {   7,  -9}, {  21,  -8},
    { -24, -36}, { -19, -28}, {  10, -33}, {  -8, -22},
    {-129, -50}, { -54, -54}, { -79, -60}, { -41, -42},
};

const int BishopPSQT32[32][PHASE_NB] = {
    { -14, -17}, {   1, -14}, { -14, -15}, { -13, -14},
    {   9, -22}, {   7, -18}, {   9, -16}, { -12, -11},
    {  -4, -15}, {  11,  -9}, {  -3, -14}, {   2,  -3},
    {  -3, -16}, { -17, -15}, {  -7,  -8}, {   4,  -7},
    { -21, -11}, {  -8,  -9}, {  -5,  -9}, {   2,  -5},
    {  -4,  -9}, {  -3, -13}, {  -2, -12}, {  -3, -11},
    { -41, -19}, { -22, -17}, { -18, -20}, { -34, -20},
    { -43, -36}, { -42, -26}, { -59, -42}, { -53, -23},
};

const int RookPSQT32[32][PHASE_NB] = {
    { -21, -21}, { -17, -16}, {  -6,  -3}, {   0, -11},
    { -31, -17}, { -19, -20}, { -23, -20}, {  -8, -18},
    { -18, -14}, { -11, -13}, { -18, -17}, {  -9, -16},
    { -17,  -4}, { -13,  -4}, { -16, -10}, { -13,  -8},
    { -10,   3}, { -11,  -2}, {  -1,   1}, {  -2,  -1},
    {  -8,   3}, {  -7,   2}, {   0,   0}, {   2,   0},
    { -10,  -1}, {  -8,   0}, {   2, -10}, {  -7,  -6},
    {  -8,  11}, {  -2,   7}, { -28,  -6}, {  -6,   6},
};

const int QueenPSQT32[32][PHASE_NB] = {
    { -27, -46}, { -34, -42}, { -30, -29}, { -20, -43},
    { -19, -38}, { -19, -40}, { -20, -59}, { -16, -28},
    { -15, -30}, {  -8, -22}, { -19, -23}, { -19, -15},
    { -19, -22}, { -17, -20}, { -24, -20}, { -23,  -4},
    { -20, -13}, { -20,  -8}, { -15, -11}, { -17,   1},
    {  -1, -16}, { -10, -14}, {  -5,  -8}, { -11,   1},
    {  -8, -17}, { -42, -11}, {  -7, -20}, { -22,  -3},
    { -32, -40}, { -17, -27}, { -41, -45}, { -25, -24},
};

const int KingPSQT32[32][PHASE_NB] = {
    {  45, -73}, {  61, -57}, {  30, -28}, {  11, -34},
    {  44, -33}, {  47, -20}, {  15,  -6}, {  -6,   0},
    {   7, -27}, {  23, -11}, {   6,   3}, { -12,  12},
    { -17, -25}, {   9,  -6}, {   0,  11}, { -28,  18},
    { -11, -13}, {  13,   3}, {  -5,  13}, { -29,  17},
    {  12,  -2}, {  27,  12}, {   1,  10}, { -31,   7},
    {   7,  -7}, {  13,   5}, { -15,  -1}, { -32,   0},
    {  -2, -34}, {  10, -10}, { -31, -30}, { -43, -13},
};

void initalizePSQT(){
    
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
