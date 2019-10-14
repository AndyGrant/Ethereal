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

#define KPRECISION  (     10) // Iterations for computing K
#define NPARTITIONS (     64) // Total thread partitions
#define REPORTING   (     25) // How often to report progress
#define NTERMS      (      0) // Total terms in the Tuner (600)

#define LEARNING    (    1.0) // Learning rate
#define LRDROPRATE  (   1.25) // Cut LR by this each failure
#define BATCHSIZE   (  16384) // FENs per mini-batch
#define NPOSITIONS  (7400000) // Total FENS in the book

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
#define TuneKnightMobility              (0)
#define TuneBishopPair                  (0)
#define TuneBishopRammedPawns           (0)
#define TuneBishopOutpost               (0)
#define TuneBishopBehindPawn            (0)
#define TuneBishopMobility              (0)
#define TuneRookFile                    (0)
#define TuneRookOnSeventh               (0)
#define TuneRookMobility                (0)
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
#define TuneThreatQueenAttackedByOne    (0)
#define TuneThreatOverloadedPieces      (0)
#define TuneThreatByPawnPush            (0)
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

void runTexelTuning(Thread *thread);
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
void printParameters_0(char *name, int params[NTERMS][PHASE_NB], int i);
void printParameters_1(char *name, int params[NTERMS][PHASE_NB], int i, int A);
void printParameters_2(char *name, int params[NTERMS][PHASE_NB], int i, int A, int B);
void printParameters_3(char *name, int params[NTERMS][PHASE_NB], int i, int A, int B, int C);

// Initalize the Phase Manger which tracks the Term type

#define INIT_PHASE_0(term, P) do {                              \
    phases[i  ][MG] = (P == NORMAL || P == MGONLY);             \
    phases[i++][EG] = (P == NORMAL || P == EGONLY);             \
} while (0)

#define INIT_PHASE_1(term, A, P) do {                           \
    for (int _a = 0; _a < A; _a++)                              \
       {phases[i  ][MG] = (P == NORMAL || P == MGONLY);         \
        phases[i++][EG] = (P == NORMAL || P == EGONLY);}        \
} while (0)

#define INIT_PHASE_2(term, A, B, P) do {                        \
    for (int _b = 0; _b < A; _b++)                              \
        INIT_PHASE_1(term[_b], B, P);                           \
} while (0)

#define INIT_PHASE_3(term, A, B, C, P) do {                     \
    for (int _c = 0; _c < A; _c++)                              \
        INIT_PHASE_2(term[_c], B, C, P);                        \
} while (0)

// Initalize Parameters of an N dimensional array

#define INIT_PARAM_0(term, P) do {                              \
     cparams[i  ][MG] = ScoreMG(term);                          \
     cparams[i++][EG] = ScoreEG(term);                          \
} while (0)

#define INIT_PARAM_1(term, A, P) do {                           \
    for (int _a = 0; _a < A; _a++)                              \
       {cparams[i  ][MG] = ScoreMG(term[_a]);                   \
        cparams[i++][EG] = ScoreEG(term[_a]);}                  \
} while (0)

#define INIT_PARAM_2(term, A, B, P) do {                        \
    for (int _b = 0; _b < A; _b++)                              \
        INIT_PARAM_1(term[_b], B, P);                           \
} while (0)

#define INIT_PARAM_3(term, A, B, C, P) do {                     \
    for (int _c = 0; _c < A; _c++)                              \
        INIT_PARAM_2(term[_c], B, C, P);                        \
} while (0)

// Initalize Coefficients from an N dimensional array

#define INIT_COEFF_0(term, P) do {                              \
    coeffs[i++] = T.term[WHITE] - T.term[BLACK];                \
} while (0)

#define INIT_COEFF_1(term, A, P) do {                           \
    for (int _a = 0; _a < A; _a++)                              \
        coeffs[i++] = T.term[_a][WHITE] - T.term[_a][BLACK];    \
} while (0)

#define INIT_COEFF_2(term, A, B, P) do {                        \
    for (int _b = 0; _b < A; _b++)                              \
        INIT_COEFF_1(term[_b], B, P);                           \
} while (0)

#define INIT_COEFF_3(term, A, B, C, P) do {                     \
    for (int _c = 0; _c < A; _c++)                              \
        INIT_COEFF_2(term[_c], B, C, P);                        \
} while (0)

// Print Parameters of an N dimensional array

#define PRINT_PARAM_0(term, P) (printParameters_0(#term, tparams, i), i+=1)

#define PRINT_PARAM_1(term, A, P) (printParameters_1(#term, tparams, i, A), i+=A)

#define PRINT_PARAM_2(term, A, B, P) (printParameters_2(#term, tparams, i, A, B), i+=A*B)

#define PRINT_PARAM_3(term, A, B, C, P) (printParameters_3(#term, tparams, i, A, B, C), i+=A*B*C)

// Generic wrapper for all of the above functions

#define ENABLE_0(fname, term, P) do {                           \
    if (Tune##term) fname##_0(term, P);                         \
} while (0)

#define ENABLE_1(fname, term, A, P) do {                        \
    if (Tune##term) fname##_1(term, A, P);                      \
} while (0)

#define ENABLE_2(fname, term, A, B, P) do {                     \
    if (Tune##term) fname##_2(term, A, B, P);                   \
} while (0)

#define ENABLE_3(fname, term, A, B, C, P) do {                  \
    if (Tune##term) fname##_3(term, A, B, C, P);                \
} while (0)

// Configuration for each aspect of the evaluation terms

#define EXECUTE_ON_TERMS(fname) do {                            \
    ENABLE_0(fname, PawnValue, NORMAL);                         \
    ENABLE_0(fname, KnightValue, NORMAL);                       \
    ENABLE_0(fname, BishopValue, NORMAL);                       \
    ENABLE_0(fname, RookValue, NORMAL);                         \
    ENABLE_0(fname, QueenValue, NORMAL);                        \
    ENABLE_0(fname, KingValue, NORMAL);                         \
    ENABLE_1(fname, PawnPSQT32, 32, NORMAL);                    \
    ENABLE_1(fname, KnightPSQT32, 32, NORMAL);                  \
    ENABLE_1(fname, BishopPSQT32, 32, NORMAL);                  \
    ENABLE_1(fname, RookPSQT32, 32, NORMAL);                    \
    ENABLE_1(fname, QueenPSQT32, 32, NORMAL);                   \
    ENABLE_1(fname, KingPSQT32, 32, NORMAL);                    \
    ENABLE_2(fname, PawnCandidatePasser, 2, 8, NORMAL);         \
    ENABLE_0(fname, PawnIsolated, NORMAL);                      \
    ENABLE_0(fname, PawnStacked, NORMAL);                       \
    ENABLE_1(fname, PawnBackwards, 2, NORMAL);                  \
    ENABLE_1(fname, PawnConnected32, 32, NORMAL);               \
    ENABLE_1(fname, KnightOutpost, 2, NORMAL);                  \
    ENABLE_0(fname, KnightBehindPawn, NORMAL);                  \
    ENABLE_1(fname, KnightMobility, 9, NORMAL);                 \
    ENABLE_0(fname, BishopPair, NORMAL);                        \
    ENABLE_0(fname, BishopRammedPawns, NORMAL);                 \
    ENABLE_1(fname, BishopOutpost, 2, NORMAL);                  \
    ENABLE_0(fname, BishopBehindPawn, NORMAL);                  \
    ENABLE_1(fname, BishopMobility, 14, NORMAL);                \
    ENABLE_1(fname, RookFile, 2, NORMAL);                       \
    ENABLE_0(fname, RookOnSeventh, NORMAL);                     \
    ENABLE_1(fname, RookMobility, 15, NORMAL);                  \
    ENABLE_1(fname, QueenMobility, 28, NORMAL);                 \
    ENABLE_1(fname, KingDefenders, 12, NORMAL);                 \
    ENABLE_1(fname, KingPawnFileProximity, 8, NORMAL);          \
    ENABLE_3(fname, KingShelter, 2, 8, 8, NORMAL);              \
    ENABLE_3(fname, KingStorm, 2, 4, 8, NORMAL);                \
    ENABLE_3(fname, PassedPawn, 2, 2, 8, NORMAL);               \
    ENABLE_1(fname, PassedFriendlyDistance, 8, NORMAL);         \
    ENABLE_1(fname, PassedEnemyDistance, 8, NORMAL);            \
    ENABLE_0(fname, PassedSafePromotionPath, NORMAL);           \
    ENABLE_0(fname, ThreatWeakPawn, NORMAL);                    \
    ENABLE_0(fname, ThreatMinorAttackedByPawn, NORMAL);         \
    ENABLE_0(fname, ThreatMinorAttackedByMinor, NORMAL);        \
    ENABLE_0(fname, ThreatMinorAttackedByMajor, NORMAL);        \
    ENABLE_0(fname, ThreatRookAttackedByLesser, NORMAL);        \
    ENABLE_0(fname, ThreatQueenAttackedByOne, NORMAL);          \
    ENABLE_0(fname, ThreatOverloadedPieces, NORMAL);            \
    ENABLE_0(fname, ThreatByPawnPush, NORMAL);                  \
    ENABLE_0(fname, ComplexityTotalPawns, EGONLY);              \
    ENABLE_0(fname, ComplexityPawnFlanks, EGONLY);              \
    ENABLE_0(fname, ComplexityPawnEndgame, EGONLY);             \
    ENABLE_0(fname, ComplexityAdjustment, EGONLY);              \
} while (0)

#endif
