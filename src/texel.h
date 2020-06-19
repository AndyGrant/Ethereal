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

#if defined(TUNE)

#pragma once

#include "types.h"

#define NPARTITIONS  (     64) // Total thread partitions
#define KPRECISION   (     10) // Iterations for computing K
#define REPORTING    (     50) // How often to report progress
#define NTERMS       (      0) // Total terms in the Tuner (648)

#define LEARNING     (    5.0) // Learning rate
#define LRDROPRATE   (   1.25) // Cut LR by this each failure
#define BATCHSIZE    (  16384) // FENs per mini-batch
#define NPOSITIONS   (5832688) // Total FENS in the book

#define STACKSIZE ((int)((double) NPOSITIONS * NTERMS / 32))

#define TunePawnValue                   (0)
#define TuneKnightValue                 (0)
#define TuneBishopValue                 (0)
#define TuneRookValue                   (0)
#define TuneQueenValue                  (0)
#define TuneKingValue                   (0)
#define TunePawnPSQT32                  (0)
#define TuneKnightPSQT32                (0)
#define TuneBishopPSQT32                (0)
#define TuneRookPSQT32                  (0)
#define TuneQueenPSQT32                 (0)
#define TuneKingPSQT32                  (0)
#define TunePawnCandidatePasser         (0)
#define TunePawnIsolated                (0)
#define TunePawnStacked                 (0)
#define TunePawnBackwards               (0)
#define TunePawnConnected32             (0)
#define TuneKnightOutpost               (0)
#define TuneKnightBehindPawn            (0)
#define TuneKnightInSiberia             (0)
#define TuneKnightMobility              (0)
#define TuneBishopPair                  (0)
#define TuneBishopRammedPawns           (0)
#define TuneBishopOutpost               (0)
#define TuneBishopBehindPawn            (0)
#define TuneBishopLongDiagonal          (0)
#define TuneBishopMobility              (0)
#define TuneRookFile                    (0)
#define TuneRookOnSeventh               (0)
#define TuneRookMobility                (0)
#define TuneQueenRelativePin            (0)
#define TuneQueenMobility               (0)
#define TuneKingDefenders               (0)
#define TuneKingPawnFileProximity       (0)
#define TuneKingShelter                 (0)
#define TuneKingStorm                   (0)
#define TunePassedPawn                  (0)
#define TunePassedFriendlyDistance      (0)
#define TunePassedEnemyDistance         (0)
#define TunePassedSafePromotionPath     (0)
#define TuneThreatWeakPawn              (0)
#define TuneThreatMinorAttackedByPawn   (0)
#define TuneThreatMinorAttackedByMinor  (0)
#define TuneThreatMinorAttackedByMajor  (0)
#define TuneThreatRookAttackedByLesser  (0)
#define TuneThreatMinorAttackedByKing   (0)
#define TuneThreatRookAttackedByKing    (0)
#define TuneThreatQueenAttackedByOne    (0)
#define TuneThreatOverloadedPieces      (0)
#define TuneThreatByPawnPush            (0)
#define TuneSpaceRestrictPiece          (0)
#define TuneSpaceRestrictEmpty          (0)
#define TuneSpaceCenterControl          (0)
#define TuneClosednessKnightAdjustment  (0)
#define TuneClosednessRookAdjustment    (0)
#define TuneComplexityTotalPawns        (0)
#define TuneComplexityPawnFlanks        (0)
#define TuneComplexityPawnEndgame       (0)
#define TuneComplexityAdjustment        (0)

enum { NORMAL, MGONLY, EGONLY };

typedef struct TexelTuple {
    int index;
    int coeff;
} TexelTuple;

typedef struct TexelEntry {
    int ntuples;
    double result;
    double eval, phase;
    double factors[PHASE_NB];
    TexelTuple *tuples;
} TexelEntry;

typedef double TexelVector[NTERMS][PHASE_NB];

void runTexelTuning();
void initTexelEntries(TexelEntry *tes, Thread *thread);
void initCoefficients(int coeffs[NTERMS]);
void initCurrentParameters(TexelVector cparams);
void initPhaseManager(TexelVector phases);

void updateMemory(TexelEntry *te, int size);
void updateGradient(TexelEntry *tes, TexelVector gradient, TexelVector params, TexelVector phases, double K, int batch);
void shuffleTexelEntries(TexelEntry *tes);

double computeOptimalK(TexelEntry *tes);
double completeEvaluationError(TexelEntry *tes, double K);
double completeLinearError(TexelEntry *tes, TexelVector params, double K);
double singleLinearError(TexelEntry *te, TexelVector params, double K);
double linearEvaluation(TexelEntry *te, TexelVector params);
double sigmoid(double K, double S);

void printParameters(TexelVector params, TexelVector cparams);
void print_0(char *name, int params[NTERMS][PHASE_NB], int i, char *S);
void print_1(char *name, int params[NTERMS][PHASE_NB], int i, int A, char *S);
void print_2(char *name, int params[NTERMS][PHASE_NB], int i, int A, int B, char *S);
void print_3(char *name, int params[NTERMS][PHASE_NB], int i, int A, int B, int C, char *S);

// Initalize the Phase Manger for an N dimensional array

#define INIT_PHASE_0(term, P, S) do {                           \
    phases[i  ][MG] = (P == NORMAL || P == MGONLY);             \
    phases[i++][EG] = (P == NORMAL || P == EGONLY);             \
} while (0)

#define INIT_PHASE_1(term, A, P, S) do {                        \
    for (int _a = 0; _a < A; _a++)                              \
       {phases[i  ][MG] = (P == NORMAL || P == MGONLY);         \
        phases[i++][EG] = (P == NORMAL || P == EGONLY);}        \
} while (0)

#define INIT_PHASE_2(term, A, B, P, S) do {                     \
    for (int _b = 0; _b < A; _b++)                              \
        INIT_PHASE_1(term[_b], B, P, S);                        \
} while (0)

#define INIT_PHASE_3(term, A, B, C, P, S) do {                  \
    for (int _c = 0; _c < A; _c++)                              \
        INIT_PHASE_2(term[_c], B, C, P, S);                     \
} while (0)

// Initalize Parameters of an N dimensional array

#define INIT_PARAM_0(term, P, S) do {                           \
     cparams[i  ][MG] = ScoreMG(term);                          \
     cparams[i++][EG] = ScoreEG(term);                          \
} while (0)

#define INIT_PARAM_1(term, A, P, S) do {                        \
    for (int _a = 0; _a < A; _a++)                              \
       {cparams[i  ][MG] = ScoreMG(term[_a]);                   \
        cparams[i++][EG] = ScoreEG(term[_a]);}                  \
} while (0)

#define INIT_PARAM_2(term, A, B, P, S) do {                     \
    for (int _b = 0; _b < A; _b++)                              \
        INIT_PARAM_1(term[_b], B, P, S);                        \
} while (0)

#define INIT_PARAM_3(term, A, B, C, P, S) do {                  \
    for (int _c = 0; _c < A; _c++)                              \
        INIT_PARAM_2(term[_c], B, C, P, S);                     \
} while (0)

// Initalize Coefficients from an N dimensional array

#define INIT_COEFF_0(term, P, S) do {                           \
    coeffs[i++] = T.term[WHITE] - T.term[BLACK];                \
} while (0)

#define INIT_COEFF_1(term, A, P, S) do {                        \
    for (int _a = 0; _a < A; _a++)                              \
        coeffs[i++] = T.term[_a][WHITE] - T.term[_a][BLACK];    \
} while (0)

#define INIT_COEFF_2(term, A, B, P, S) do {                     \
    for (int _b = 0; _b < A; _b++)                              \
        INIT_COEFF_1(term[_b], B, P, S);                        \
} while (0)

#define INIT_COEFF_3(term, A, B, C, P, S) do {                  \
    for (int _c = 0; _c < A; _c++)                              \
        INIT_COEFF_2(term[_c], B, C, P, S);                     \
} while (0)

// Print Parameters of an N dimensional array

#define PRINT_0(term, P, S) (print_0(#term, tparams, i, S), i+=1)

#define PRINT_1(term, A, P, S) (print_1(#term, tparams, i, A, S), i+=A)

#define PRINT_2(term, A, B, P, S) (print_2(#term, tparams, i, A, B, S), i+=A*B)

#define PRINT_3(term, A, B, C, P, S) (print_3(#term, tparams, i, A, B, C, S), i+=A*B*C)

// Generic wrapper for all of the above functions

#define ENABLE_0(F, term, P, S) do {                            \
    if (Tune##term) F##_0(term, P, S);                          \
} while (0)

#define ENABLE_1(F, term, A, P, S) do {                         \
    if (Tune##term) F##_1(term, A, P, S);                       \
} while (0)

#define ENABLE_2(F, term, A, B, P, S) do {                      \
    if (Tune##term) F##_2(term, A, B, P, S);                    \
} while (0)

#define ENABLE_3(F, term, A, B, C, P, S) do {                   \
    if (Tune##term) F##_3(term, A, B, C, P, S);                 \
} while (0)

// Configuration for each aspect of the evaluation terms

#define EXECUTE_ON_TERMS(F) do {                                            \
    ENABLE_0(F, PawnValue, NORMAL, "");                                     \
    ENABLE_0(F, KnightValue, NORMAL, "");                                   \
    ENABLE_0(F, BishopValue, NORMAL, "");                                   \
    ENABLE_0(F, RookValue, NORMAL, "");                                     \
    ENABLE_0(F, QueenValue, NORMAL, "");                                    \
    ENABLE_0(F, KingValue, NORMAL, "");                                     \
    ENABLE_1(F, PawnPSQT32, 32, NORMAL, "[32]");                            \
    ENABLE_1(F, KnightPSQT32, 32, NORMAL, "[32]");                          \
    ENABLE_1(F, BishopPSQT32, 32, NORMAL, "[32]");                          \
    ENABLE_1(F, RookPSQT32, 32, NORMAL, "[32]");                            \
    ENABLE_1(F, QueenPSQT32, 32, NORMAL, "[32]");                           \
    ENABLE_1(F, KingPSQT32, 32, NORMAL, "[32]");                            \
    ENABLE_2(F, PawnCandidatePasser, 2, 8, NORMAL, "[2][RANK_NB]");         \
    ENABLE_0(F, PawnIsolated, NORMAL, "");                                  \
    ENABLE_1(F, PawnStacked, 2, NORMAL, "[2]");                             \
    ENABLE_2(F, PawnBackwards, 2, 8, NORMAL, "[2][RANK_NB]");               \
    ENABLE_1(F, PawnConnected32, 32, NORMAL, "[32]");                       \
    ENABLE_2(F, KnightOutpost, 2, 2, NORMAL, "[2][2]");                     \
    ENABLE_0(F, KnightBehindPawn, NORMAL, "");                              \
    ENABLE_1(F, KnightInSiberia, 4, NORMAL, "[4]");                         \
    ENABLE_1(F, KnightMobility, 9, NORMAL, "[9]");                          \
    ENABLE_0(F, BishopPair, NORMAL, "");                                    \
    ENABLE_0(F, BishopRammedPawns, NORMAL, "");                             \
    ENABLE_2(F, BishopOutpost, 2, 2, NORMAL, "[2][2]");                     \
    ENABLE_0(F, BishopBehindPawn, NORMAL, "");                              \
    ENABLE_0(F, BishopLongDiagonal, NORMAL, "");                            \
    ENABLE_1(F, BishopMobility, 14, NORMAL, "[14]");                        \
    ENABLE_1(F, RookFile, 2, NORMAL, "[2]");                                \
    ENABLE_0(F, RookOnSeventh, NORMAL, "");                                 \
    ENABLE_1(F, RookMobility, 15, NORMAL, "[15]");                          \
    ENABLE_0(F, QueenRelativePin, NORMAL, "");                              \
    ENABLE_1(F, QueenMobility, 28, NORMAL, "[28]");                         \
    ENABLE_1(F, KingDefenders, 12, NORMAL, "[12]");                         \
    ENABLE_1(F, KingPawnFileProximity, 8, NORMAL, "[FILE_NB]");             \
    ENABLE_3(F, KingShelter, 2, 8, 8, NORMAL, "[2][FILE_NB][RANK_NB]");     \
    ENABLE_3(F, KingStorm, 2, 4, 8, NORMAL, "[2][FILE_NB/2][RANK_NB]");     \
    ENABLE_3(F, PassedPawn, 2, 2, 8, NORMAL, "[2][2][RANK_NB]");            \
    ENABLE_1(F, PassedFriendlyDistance, 8, NORMAL, "[FILE_NB]");            \
    ENABLE_1(F, PassedEnemyDistance, 8, NORMAL, "[FILE_NB]");               \
    ENABLE_0(F, PassedSafePromotionPath, NORMAL, "");                       \
    ENABLE_0(F, ThreatWeakPawn, NORMAL, "");                                \
    ENABLE_0(F, ThreatMinorAttackedByPawn, NORMAL, "");                     \
    ENABLE_0(F, ThreatMinorAttackedByMinor, NORMAL, "");                    \
    ENABLE_0(F, ThreatMinorAttackedByMajor, NORMAL, "");                    \
    ENABLE_0(F, ThreatRookAttackedByLesser, NORMAL, "");                    \
    ENABLE_0(F, ThreatMinorAttackedByKing, NORMAL, "");                     \
    ENABLE_0(F, ThreatRookAttackedByKing, NORMAL, "");                      \
    ENABLE_0(F, ThreatQueenAttackedByOne, NORMAL, "");                      \
    ENABLE_0(F, ThreatOverloadedPieces, NORMAL, "");                        \
    ENABLE_0(F, ThreatByPawnPush, NORMAL, "");                              \
    ENABLE_0(F, SpaceRestrictPiece, NORMAL, "");                            \
    ENABLE_0(F, SpaceRestrictEmpty, NORMAL, "");                            \
    ENABLE_0(F, SpaceCenterControl, NORMAL, "");                            \
    ENABLE_1(F, ClosednessKnightAdjustment, 9, NORMAL, "[9]");              \
    ENABLE_1(F, ClosednessRookAdjustment, 9, NORMAL, "[9]");                \
    ENABLE_0(F, ComplexityTotalPawns, EGONLY, "");                          \
    ENABLE_0(F, ComplexityPawnFlanks, EGONLY, "");                          \
    ENABLE_0(F, ComplexityPawnEndgame, EGONLY, "");                         \
    ENABLE_0(F, ComplexityAdjustment, EGONLY, "");                          \
} while (0)

#endif
