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

#pragma once

#include <stdint.h>

#include "evalcache.h"
#include "types.h"

#ifdef TUNE
    #define TRACE (1)
#else
    #define TRACE (0)
#endif

enum {
    SCALE_DRAW             =   0,
    SCALE_OCB_BISHOPS_ONLY =  64,
    SCALE_OCB_ONE_KNIGHT   = 106,
    SCALE_OCB_ONE_ROOK     =  96,
    SCALE_LONE_QUEEN       =  88,
    SCALE_NORMAL           = 128,
    SCALE_LARGE_PAWN_ADV   = 144,
};

struct EvalTrace {
    int PawnValue[COLOUR_NB];
    int KnightValue[COLOUR_NB];
    int BishopValue[COLOUR_NB];
    int RookValue[COLOUR_NB];
    int QueenValue[COLOUR_NB];
    int KingValue[COLOUR_NB];
    int PawnPSQT[SQUARE_NB][COLOUR_NB];
    int KnightPSQT[SQUARE_NB][COLOUR_NB];
    int BishopPSQT[SQUARE_NB][COLOUR_NB];
    int RookPSQT[SQUARE_NB][COLOUR_NB];
    int QueenPSQT[SQUARE_NB][COLOUR_NB];
    int KingPSQT[SQUARE_NB][COLOUR_NB];
    int PawnCandidatePasser[2][8][COLOUR_NB];
    int PawnIsolated[8][COLOUR_NB];
    int PawnStacked[2][8][COLOUR_NB];
    int PawnBackwards[2][8][COLOUR_NB];
    int PawnConnected32[32][COLOUR_NB];
    int KnightOutpost[2][2][COLOUR_NB];
    int KnightBehindPawn[COLOUR_NB];
    int KnightInSiberia[4][COLOUR_NB];
    int KnightMobility[9][COLOUR_NB];
    int BishopPair[COLOUR_NB];
    int BishopRammedPawns[COLOUR_NB];
    int BishopOutpost[2][2][COLOUR_NB];
    int BishopBehindPawn[COLOUR_NB];
    int BishopLongDiagonal[COLOUR_NB];
    int BishopMobility[14][COLOUR_NB];
    int RookFile[2][COLOUR_NB];
    int RookOnSeventh[COLOUR_NB];
    int RookMobility[15][COLOUR_NB];
    int QueenRelativePin[COLOUR_NB];
    int QueenMobility[28][COLOUR_NB];
    int KingPawnFileProximity[8][COLOUR_NB];
    int KingDefenders[12][COLOUR_NB];
    int KingShelter[2][8][8][COLOUR_NB];
    int KingStorm[2][4][8][COLOUR_NB];
    int SafetyKnightWeight[COLOUR_NB];
    int SafetyBishopWeight[COLOUR_NB];
    int SafetyRookWeight[COLOUR_NB];
    int SafetyQueenWeight[COLOUR_NB];
    int SafetyAttackValue[COLOUR_NB];
    int SafetyWeakSquares[COLOUR_NB];
    int SafetyNoEnemyQueens[COLOUR_NB];
    int SafetySafeQueenCheck[COLOUR_NB];
    int SafetySafeRookCheck[COLOUR_NB];
    int SafetySafeBishopCheck[COLOUR_NB];
    int SafetySafeKnightCheck[COLOUR_NB];
    int SafetyAdjustment[COLOUR_NB];
    int SafetyShelter[2][8][COLOUR_NB];
    int SafetyStorm[2][8][COLOUR_NB];
    int PassedPawn[2][2][8][COLOUR_NB];
    int PassedFriendlyDistance[8][COLOUR_NB];
    int PassedEnemyDistance[8][COLOUR_NB];
    int PassedSafePromotionPath[COLOUR_NB];
    int ThreatWeakPawn[COLOUR_NB];
    int ThreatMinorAttackedByPawn[COLOUR_NB];
    int ThreatMinorAttackedByMinor[COLOUR_NB];
    int ThreatMinorAttackedByMajor[COLOUR_NB];
    int ThreatRookAttackedByLesser[COLOUR_NB];
    int ThreatMinorAttackedByKing[COLOUR_NB];
    int ThreatRookAttackedByKing[COLOUR_NB];
    int ThreatQueenAttackedByOne[COLOUR_NB];
    int ThreatOverloadedPieces[COLOUR_NB];
    int ThreatByPawnPush[COLOUR_NB];
    int SpaceRestrictPiece[COLOUR_NB];
    int SpaceRestrictEmpty[COLOUR_NB];
    int SpaceCenterControl[COLOUR_NB];
    int ClosednessKnightAdjustment[9][COLOUR_NB];
    int ClosednessRookAdjustment[9][COLOUR_NB];
    int ComplexityTotalPawns[COLOUR_NB];
    int ComplexityPawnFlanks[COLOUR_NB];
    int ComplexityPawnEndgame[COLOUR_NB];
    int ComplexityAdjustment[COLOUR_NB];
    int eval, complexity, factor, safety[COLOUR_NB];
};

struct EvalInfo {
    uint64_t pawnAttacks[COLOUR_NB];
    uint64_t pawnAttacksBy2[COLOUR_NB];
    uint64_t rammedPawns[COLOUR_NB];
    uint64_t blockedPawns[COLOUR_NB];
    uint64_t kingAreas[COLOUR_NB];
    uint64_t mobilityAreas[COLOUR_NB];
    uint64_t attacked[COLOUR_NB];
    uint64_t attackedBy2[COLOUR_NB];
    uint64_t attackedBy[COLOUR_NB][PIECE_NB];
    uint64_t occupiedMinusBishops[COLOUR_NB];
    uint64_t occupiedMinusRooks[COLOUR_NB];
    uint64_t passedPawns;
    int kingSquare[COLOUR_NB];
    int kingAttacksCount[COLOUR_NB];
    int kingAttackersCount[COLOUR_NB];
    int kingAttackersWeight[COLOUR_NB];
    int pkeval[COLOUR_NB];
    int pksafety[COLOUR_NB];
    PKEntry *pkentry;
};

int evaluateBoard(Thread *thread, Board *board);
int evaluatePieces(EvalInfo *ei, Board *board);
int evaluatePawns(EvalInfo *ei, Board *board, int colour);
int evaluateKnights(EvalInfo *ei, Board *board, int colour);
int evaluateBishops(EvalInfo *ei, Board *board, int colour);
int evaluateRooks(EvalInfo *ei, Board *board, int colour);
int evaluateQueens(EvalInfo *ei, Board *board, int colour);
int evaluateKingsPawns(EvalInfo *ei, Board *board, int colour);
int evaluateKings(EvalInfo *ei, Board *board, int colour);
int evaluatePassed(EvalInfo *ei, Board *board, int colour);
int evaluateThreats(EvalInfo *ei, Board *board, int colour);
int evaluateSpace(EvalInfo *ei, Board *board, int colour);
int evaluateClosedness(EvalInfo *ei, Board *board);
int evaluateComplexity(EvalInfo *ei, Board *board, int eval);
int evaluateScaleFactor(Board *board, int eval);
void initEvalInfo(Thread *thread, Board *board, EvalInfo *ei);
void initEval();

#define MakeScore(mg, eg) ((int)((unsigned int)(eg) << 16) + (mg))
#define ScoreMG(s) ((int16_t)((uint16_t)((unsigned)((s)))))
#define ScoreEG(s) ((int16_t)((uint16_t)((unsigned)((s) + 0x8000) >> 16)))

extern int PSQT[32][SQUARE_NB];
extern const int Tempo;
