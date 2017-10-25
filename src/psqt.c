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
    { -14,  -2}, {   2,   0}, {   3,   0}, {  -2,  -2},
    { -16,  -4}, {  -5,  -3}, {  -6,  -5}, {  -6,  -9},
    { -19,   1}, { -17,   0}, {  -4,  -8}, {   6, -14},
    { -11,   8}, { -11,   3}, {  -3,  -5}, {   8, -14},
    {  -2,  16}, {   0,  12}, {   6,  -2}, {   4, -14},
    {   4,  35}, {   0,  33}, {   4,  10}, {  20,  14},
    { -15,   0}, { -10,   0}, {   0,   0}, {   5,   0},
};

const int KnightPSQT32[32][PHASE_NB] = {
    { -45, -47}, { -29, -39}, { -25, -22}, { -21, -23},
    { -23, -35}, { -29, -28}, { -17, -24}, {  -6, -15},
    { -23, -26}, { -10, -18}, {   0,  -9}, {   7,  -6},
    { -16, -21}, {  -8, -21}, {   0, -11}, {   5,  -2},
    {  -2, -19}, {   4, -13}, {   7,  -6}, {  20,   0},
    { -16, -29}, {   4, -20}, {   8,  -9}, {  21,  -7},
    { -22, -34}, { -15, -26}, {  10, -28}, {  -6, -18},
    {-131, -48}, { -49, -49}, { -66, -53}, { -35, -36},
};

const int BishopPSQT32[32][PHASE_NB] = {
    { -15, -17}, {   0, -14}, { -14, -14}, { -14, -13},
    {   9, -20}, {   8, -15}, {  10, -14}, { -11,  -8},
    {  -4, -13}, {  10,  -8}, {  -2, -11}, {   3,  -2},
    {  -2, -15}, { -15, -14}, {  -7,  -7}, {   5,  -6},
    { -19, -11}, {  -6,  -8}, {  -5,  -8}, {   2,  -4},
    {  -6,  -9}, {  -4, -12}, {  -2, -11}, {  -4, -11},
    { -40, -20}, { -18, -15}, { -17, -18}, { -29, -19},
    { -40, -36}, { -37, -24}, { -49, -37}, { -45, -22},
};

const int RookPSQT32[32][PHASE_NB] = {
    { -19, -17}, { -15, -13}, {  -4,  -2}, {   1,  -9},
    { -29, -15}, { -18, -17}, { -20, -17}, {  -8, -16},
    { -17, -13}, {  -9, -11}, { -15, -14}, {  -9, -14},
    { -15,  -4}, { -11,  -4}, { -15,  -9}, { -12,  -7},
    {  -9,   3}, { -11,  -2}, {  -1,   2}, {  -2,  -1},
    {  -7,   3}, {  -6,   3}, {  -1,   0}, {   1,   1},
    {  -9,  -1}, {  -7,   0}, {   2,  -9}, {  -6,  -5},
    {  -7,  10}, {  -3,   6}, { -25,  -6}, {  -8,   3},
};

const int QueenPSQT32[32][PHASE_NB] = {
    { -23, -41}, { -31, -36}, { -28, -26}, { -17, -35},
    { -16, -33}, { -16, -33}, { -18, -49}, { -13, -23},
    { -13, -26}, {  -8, -18}, { -16, -18}, { -16, -11},
    { -17, -18}, { -14, -16}, { -20, -14}, { -19,   0},
    { -15, -11}, { -18,  -6}, { -12,  -7}, { -13,   5},
    {   0, -14}, {  -9, -13}, {  -2,  -4}, {  -7,   5},
    { -12, -18}, { -42, -12}, {  -4, -15}, { -13,   2},
    { -26, -32}, { -10, -19}, { -31, -33}, { -18, -16},
};

const int KingPSQT32[32][PHASE_NB] = {
    {  48, -71}, {  64, -55}, {  30, -30}, {   8, -34},
    {  44, -34}, {  46, -20}, {  15,  -8}, {  -8,  -1},
    {   8, -28}, {  22, -11}, {   5,   2}, { -15,  11},
    { -13, -25}, {   8,  -7}, {  -2,  10}, { -31,  16},
    {  -8, -12}, {  11,   3}, {  -8,  13}, { -33,  15},
    {  12,  -4}, {  24,  11}, {  -3,  10}, { -36,   7},
    {   8,  -7}, {  12,   5}, { -17,   0}, { -34,   3},
    {   1, -31}, {  13,  -9}, { -27, -25}, { -45, -11},
};

void initalizePSQT(){
    
    int sq, w32, b32;
    
    for (sq = 0; sq < SQUARE_NB; sq++){

        w32 = RelativeSquare32(sq, WHITE);
        b32 = RelativeSquare32(sq, BLACK);

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
