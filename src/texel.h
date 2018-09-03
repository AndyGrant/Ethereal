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

#define CLEARING    (      1) // Clear hashes between runs
#define RESOLVE     (      1) // Resolve with qsearch
#define NPARTITIONS (     64) // Total thread partitions
#define LEARNING    (    0.1) // Learning rate step size
#define REPORTING   (    100) // How often to report progress

#define NDEPTHS     (      8) // # of search iterations
#define NTERMS      (    340) // # of terms to tune
#define NPOSITIONS  (1364312) // # of FENs in book

#define TunePawnValue                   (0)
#define TuneKnightValue                 (0)
#define TuneBishopValue                 (0)
#define TuneRookValue                   (0)
#define TuneQueenValue                  (0)
#define TuneKingValue                   (0)
#define TunePawnPSQT32                  (1)
#define TuneKnightPSQT32                (1)
#define TuneBishopPSQT32                (1)
#define TuneRookPSQT32                  (1)
#define TuneQueenPSQT32                 (1)
#define TuneKingPSQT32                  (1)
#define TunePawnIsolated                (1)
#define TunePawnStacked                 (1)
#define TunePawnBackwards               (1)
#define TunePawnConnected32             (1)
#define TuneKnightOutpost               (1)
#define TuneKnightBehindPawn            (1)
#define TuneKnightMobility              (1)
#define TuneBishopPair                  (1)
#define TuneBishopRammedPawns           (1)
#define TuneBishopOutpost               (1)
#define TuneBishopBehindPawn            (1)
#define TuneBishopMobility              (1)
#define TuneRookFile                    (1)
#define TuneRookOnSeventh               (1)
#define TuneRookMobility                (1)
#define TuneQueenMobility               (1)
#define TuneKingDefenders               (0)
#define TuneKingShelter                 (0)
#define TuneKingStorm                   (0)
#define TunePassedPawn                  (1)
#define TunePassedFriendlyDistance      (1)
#define TunePassedEnemyDistance         (1)
#define TunePassedSafePromotionPath     (1)
#define TuneThreatWeakPawn              (0)
#define TuneThreatMinorAttackedByPawn   (0)
#define TuneThreatMinorAttackedByMajor  (0)
#define TuneThreatRookAttackedByLesser  (0)
#define TuneThreatQueenAttackedByOne    (0)
#define TuneThreatOverloadedPieces      (0)
#define TuneThreatByPawnPush            (0)

// Size of each allocated chunk
#define STACKSIZE ((int)((double) NPOSITIONS * NTERMS / 16))

struct TexelTuple {
    int index;
    int coeff;
};

struct TexelEntry {
    int ntuples;
    double result;
    double eval, phase;
    double factors[PHASE_NB];
    TexelTuple* tuples;
};

void runTexelTuning(Thread* thread);
void initTexelEntries(TexelEntry* tes, Thread* thread);

void initCoefficients(int coeffs[NTERMS]);
void initCurrentParameters(double cparams[NTERMS][PHASE_NB]);
void printParameters(double params[NTERMS][PHASE_NB], double cparams[NTERMS][PHASE_NB]);

double computeOptimalK(TexelEntry* tes);
double completeEvaluationError(TexelEntry* tes, double K);
double completeLinearError(TexelEntry* tes, double params[NTERMS][PHASE_NB], double K);
double singleLinearError(TexelEntry te, double params[NTERMS][PHASE_NB], double K);
double linearEvaluation(TexelEntry te, double params[NTERMS][PHASE_NB]);
double sigmoid(double K, double S);

void printParameters_0(char *name, int params[NTERMS][PHASE_NB], int i);
void printParameters_1(char *name, int params[NTERMS][PHASE_NB], int i, int A);
void printParameters_2(char *name, int params[NTERMS][PHASE_NB], int i, int A, int B);
void printParameters_3(char *name, int params[NTERMS][PHASE_NB], int i, int A, int B, int C);

// Initalize Parameters of an N dimensional array

#define INIT_PARAM_0(term) do {                                 \
     cparams[i  ][MG] = ScoreMG(term);                          \
     cparams[i++][EG] = ScoreEG(term);                          \
} while (0)

#define INIT_PARAM_1(term, A) do {                              \
    for (int _a = 0; _a < A; _a++)                              \
       {cparams[i  ][MG] = ScoreMG(term[_a]);                   \
        cparams[i++][EG] = ScoreEG(term[_a]);}                  \
} while (0)

#define INIT_PARAM_2(term, A, B) do {                           \
    for (int _b = 0; _b < A; _b++)                              \
        INIT_PARAM_1(term[_b], B);                              \
} while (0)

#define INIT_PARAM_3(term, A, B, C) do {                        \
    for (int _c = 0; _c < A; _c++)                              \
        INIT_PARAM_2(term[_c], B, C);                           \
} while (0)

// Initalize Coefficients from an N dimensional array

#define INIT_COEFF_0(term) do {                                 \
    coeffs[i++] = T.term[WHITE] - T.term[BLACK];                \
} while (0)

#define INIT_COEFF_1(term, A) do {                              \
    for (int _a = 0; _a < A; _a++)                              \
        coeffs[i++] = T.term[_a][WHITE] - T.term[_a][BLACK];    \
} while (0)

#define INIT_COEFF_2(term, A, B) do {                           \
    for (int _b = 0; _b < A; _b++)                              \
        INIT_COEFF_1(term[_b], B);                              \
} while (0)

#define INIT_COEFF_3(term, A, B, C) do {                        \
    for (int _c = 0; _c < A; _c++)                              \
        INIT_COEFF_2(term[_c], B, C);                           \
} while (0)

// Print Parameters of an N dimensional array

#define PRINT_PARAM_0(term) (printParameters_0(#term, tparams, i), i+=1)

#define PRINT_PARAM_1(term, A) (printParameters_1(#term, tparams, i, A), i+=A)

#define PRINT_PARAM_2(term, A, B) (printParameters_2(#term, tparams, i, A, B), i+=A*B)

#define PRINT_PARAM_3(term, A, B, C) (printParameters_3(#term, tparams, i, A, B, C), i+=A*B*C)

// Wrap all of the above to check for the term being enabled

#define ENABLE_0(fname, term) do {                              \
    if (Tune##term) fname##_0(term);                            \
} while (0)

#define ENABLE_1(fname, term, A) do {                           \
    if (Tune##term) fname##_1(term, A);                         \
} while (0)

#define ENABLE_2(fname, term, A, legnth2) do {                  \
    if (Tune##term) fname##_2(term, A, B);                      \
} while (0)

#define ENABLE_3(fname, term, A, B, C) do {                     \
    if (Tune##term) fname##_3(term, A, B, C);                   \
} while (0)

// Configuration for each aspect of the evaluation terms

#define EXECUTE_ON_TERMS(fname) do {                            \
    ENABLE_0(fname, PawnValue);                                 \
    ENABLE_0(fname, KnightValue);                               \
    ENABLE_0(fname, BishopValue);                               \
    ENABLE_0(fname, RookValue);                                 \
    ENABLE_0(fname, QueenValue);                                \
    ENABLE_0(fname, KingValue);                                 \
    ENABLE_1(fname, PawnPSQT32, 32);                            \
    ENABLE_1(fname, KnightPSQT32, 32);                          \
    ENABLE_1(fname, BishopPSQT32, 32);                          \
    ENABLE_1(fname, RookPSQT32, 32);                            \
    ENABLE_1(fname, QueenPSQT32, 32);                           \
    ENABLE_1(fname, KingPSQT32, 32);                            \
    ENABLE_0(fname, PawnIsolated);                              \
    ENABLE_0(fname, PawnStacked);                               \
    ENABLE_1(fname, PawnBackwards, 2);                          \
    ENABLE_1(fname, PawnConnected32, 32);                       \
    ENABLE_1(fname, KnightOutpost, 2);                          \
    ENABLE_0(fname, KnightBehindPawn);                          \
    ENABLE_1(fname, KnightMobility, 9);                         \
    ENABLE_0(fname, BishopPair);                                \
    ENABLE_0(fname, BishopRammedPawns);                         \
    ENABLE_1(fname, BishopOutpost, 2);                          \
    ENABLE_0(fname, BishopBehindPawn);                          \
    ENABLE_1(fname, BishopMobility, 14);                        \
    ENABLE_1(fname, RookFile, 2);                               \
    ENABLE_0(fname, RookOnSeventh);                             \
    ENABLE_1(fname, RookMobility, 15);                          \
    ENABLE_1(fname, QueenMobility, 28);                         \
    ENABLE_1(fname, KingDefenders, 12);                         \
    ENABLE_3(fname, KingShelter, 2, 8, 8);                      \
    ENABLE_3(fname, KingStorm, 2, 4, 8);                        \
    ENABLE_3(fname, PassedPawn, 2, 2, 8);                       \
    ENABLE_0(fname, PassedFriendlyDistance);                    \
    ENABLE_0(fname, PassedEnemyDistance);                       \
    ENABLE_0(fname, PassedSafePromotionPath);                   \
    ENABLE_0(fname, ThreatWeakPawn);                            \
    ENABLE_0(fname, ThreatMinorAttackedByPawn);                 \
    ENABLE_0(fname, ThreatMinorAttackedByMajor);                \
    ENABLE_0(fname, ThreatRookAttackedByLesser);                \
    ENABLE_0(fname, ThreatQueenAttackedByOne);                  \
    ENABLE_0(fname, ThreatOverloadedPieces);                    \
    ENABLE_0(fname, ThreatByPawnPush);                          \
} while (0)

#endif
