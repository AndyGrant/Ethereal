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

typedef struct EvalTrace {
    
    int pawnCounts[COLOUR_NB];
    int pawnPSQT[COLOUR_NB][SQUARE_NB];
    int pawnIsolated[COLOUR_NB];
    int pawnStacked[COLOUR_NB];
    int pawnBackwards[COLOUR_NB][2];
    int pawnConnected[COLOUR_NB][SQUARE_NB];
    
    int knightCounts[COLOUR_NB];
    int knightPSQT[COLOUR_NB][SQUARE_NB];
    int knightAttackedByPawn[COLOUR_NB];
    int knightOutpost[COLOUR_NB][2];
    int knightMobility[COLOUR_NB][9];
    
    int bishopCounts[COLOUR_NB];
    int bishopPSQT[COLOUR_NB][SQUARE_NB];
    int bishopAttackedByPawn[COLOUR_NB];
    int bishopWings[COLOUR_NB];
    int bishopPair[COLOUR_NB];
    int bishopOutpost[COLOUR_NB][2];
    int bishopMobility[COLOUR_NB][14];
    
    int rookCounts[COLOUR_NB];
    int rookPSQT[COLOUR_NB][SQUARE_NB];
    int rookFile[COLOUR_NB][2];
    int rookOnSeventh[COLOUR_NB];
    int rookMobility[COLOUR_NB][15];
    
    int queenCounts[COLOUR_NB];
    int queenChecked[COLOUR_NB];
    int queenCheckedByPawn[COLOUR_NB];
    int queenPSQT[COLOUR_NB][SQUARE_NB];
    int queenMobility[COLOUR_NB][28];
    
    int kingPSQT[COLOUR_NB][SQUARE_NB];
    int kingDefenders[COLOUR_NB][12];
    int kingShelter[COLOUR_NB][2][2][RANK_NB];
    
    int passedPawn[COLOUR_NB][2][2][RANK_NB];
    
} EvalTrace;

typedef struct EvalInfo {
    
    uint64_t pawnAttacks[COLOUR_NB];
    uint64_t blockedPawns[COLOUR_NB];
    uint64_t kingAreas[COLOUR_NB];
    uint64_t mobilityAreas[COLOUR_NB];
    uint64_t attacked[COLOUR_NB];
    uint64_t attackedBy2[COLOUR_NB];
    uint64_t attackedNoQueen[COLOUR_NB];
    uint64_t occupiedMinusBishops[COLOUR_NB];
    uint64_t occupiedMinusRooks[COLOUR_NB];
    uint64_t passedPawns;
    int attackCounts[COLOUR_NB];
    int attackerCounts[COLOUR_NB];
    int midgame[COLOUR_NB];
    int endgame[COLOUR_NB];
    int pawnMidgame[COLOUR_NB];
    int pawnEndgame[COLOUR_NB];
    int positionIsDrawn;
    PawnEntry* pentry;
    
} EvalInfo;

int evaluateBoard(Board* board, EvalInfo* ei, PawnTable* ptable);
int evaluateDraws(Board* board);
void evaluatePieces(EvalInfo* ei, Board* board, PawnTable* ptable);
void evaluatePawns(EvalInfo* ei, Board* board, int colour);
void evaluateKnights(EvalInfo* ei, Board* board, int colour);
void evaluateBishops(EvalInfo* ei, Board* board, int colour);
void evaluateRooks(EvalInfo* ei, Board* board, int colour);
void evaluateQueens(EvalInfo* ei, Board* board, int colour);
void evaluateKings(EvalInfo* ei, Board* board, int colour);
void evaluatePassedPawns(EvalInfo* ei, Board * board, int colour);
void initializeEvalInfo(EvalInfo* ei, Board * board, PawnTable* ptable);

extern const int* PieceValues[8];

#endif
