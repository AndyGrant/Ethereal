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

#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

typedef struct EvalInfo {
    uint64_t pawnAttacks[COLOUR_NB];
    uint64_t blockedPawns[COLOUR_NB];
    uint64_t kingAreas[COLOUR_NB];
    uint64_t mobilityAreas[COLOUR_NB];
    uint64_t attacked[COLOUR_NB];
    uint64_t occupiedMinusBishops[COLOUR_NB];
    uint64_t occupiedMinusRooks[COLOUR_NB];
    uint64_t passedPawns;
    int attackCounts[COLOUR_NB];
    int attackerCounts[COLOUR_NB];
    int midgame[COLOUR_NB];
    int endgame[COLOUR_NB];
    int pawnMidgame[COLOUR_NB];
    int pawnEndgame[COLOUR_NB];
} EvalInfo;

int evaluateBoard(Board * board);
void evaluatePawns(EvalInfo * ei, Board * board, int colour, PawnEntry * pentry);
void evaluateKnights(EvalInfo * ei, Board * board, int colour);
void evaluateBishops(EvalInfo * ei, Board * board, int colour);
void evaluateRooks(EvalInfo * ei, Board * board, int colour);
void evaluateQueens(EvalInfo * ei, Board * board, int colour);
void evaluateKings(EvalInfo * ei, Board * board, int colour);
void evaluatePassedPawns(EvalInfo * ei, Board * board, int colour);

#define PawnValue   ( 100)
#define KnightValue ( 325)
#define BishopValue ( 325)
#define RookValue   ( 505)
#define QueenValue  (1000)
#define KingValue   ( 100)

extern const int PieceValues[8];

#endif