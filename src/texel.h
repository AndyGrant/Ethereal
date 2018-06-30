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

#if defined(TUNE) && !defined(_TEXEL_H)
#define _TEXEL_H

#include "types.h"

#define NTHREADS   (      4) // # of Threads to use
#define NTERMS     (      0) // # of Terms to tune
#define NPOSITIONS (1491000) // # of FENs in book

// Each Eval Term (Total = 487)
#define TunePawnValue                  (0)
#define TuneKnightValue                (0)
#define TuneBishopValue                (0)
#define TuneRookValue                  (0)
#define TuneQueenValue                 (0)
#define TuneKingValue                  (0)
#define TunePawnPSQT32                 (0)
#define TuneKnightPSQT32               (0)
#define TuneBishopPSQT32               (0)
#define TuneRookPSQT32                 (0)
#define TuneQueenPSQT32                (0)
#define TuneKingPSQT32                 (0)
#define TunePawnIsolated               (0)
#define TunePawnStacked                (0)
#define TunePawnBackwards              (0)
#define TunePawnConnected32            (0)
#define TuneKnightOutpost              (0)
#define TuneKnightMobility             (0)
#define TuneBishopPair                 (0)
#define TuneBishopRammedPawns          (0)
#define TuneBishopOutpost              (0)
#define TuneBishopMobility             (0)
#define TuneRookFile                   (0)
#define TuneRookOnSeventh              (0)
#define TuneRookMobility               (0)
#define TuneQueenMobility              (0)
#define TuneKingDefenders              (0)
#define TuneKingShelter                (0)
#define TunePassedPawn                 (0)
#define TuneThreatPawnAttackedByOne    (0)
#define TuneThreatMinorAttackedByPawn  (0)
#define TuneThreatMinorAttackedByMajor (0)
#define TuneThreatQueenAttackedByOne   (0)
#define TuneThreatOverloadedPieces     (0)
#define TuneThreatByPawnPush           (0)

// Size of each allocated chunk
#define STACKSIZE ((int)((double) NPOSITIONS * NTERMS / 64))

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
void initLearningRates(TexelEntry* tes, double rates[NTERMS][PHASE_NB]);

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

// Initalize Parameters of an N dimensional Array

#define INIT_PARAM_0(term) do {                                     \
     cparams[i  ][MG] = ScoreMG(term);                              \
     cparams[i++][EG] = ScoreEG(term);                              \
} while (0)

#define INIT_PARAM_1(term, length1) do {                            \
    for (int _a = 0; _a < length1; _a++)                            \
       {cparams[i  ][MG] = ScoreMG(term[_a]);                       \
        cparams[i++][EG] = ScoreEG(term[_a]);}                      \
} while (0)

#define INIT_PARAM_2(term, length1, length2) do {                   \
    for (int _b = 0; _b < length1; _b++)                            \
        INIT_PARAM_1(term[_b], length2);                            \
} while (0)

#define INIT_PARAM_3(term, length1, length2, length3) do {          \
    for (int _c = 0; _c < length1; _c++)                            \
        INIT_PARAM_2(term[_c], length2, length3);                   \
} while (0)


// Initalize Coefficients from an N dimensional Array

#define INIT_COEFF_0(term) do {                                     \
    coeffs[i++] = T.term[WHITE] - T.term[BLACK];                    \
} while (0)

#define INIT_COEFF_1(term, length1) do {                            \
    for (int _a = 0; _a < length1; _a++)                            \
        coeffs[i++] = T.term[_a][WHITE] - T.term[_a][BLACK];        \
} while (0)

#define INIT_COEFF_2(term, length1, length2) do {                   \
    for (int _b = 0; _b < length1; _b++)                            \
        INIT_COEFF_1(term[_b], length2);                            \
} while (0)

#define INIT_COEFF_3(term, length1, length2, length3) do {          \
    for (int _c = 0; _c < length1; _c++)                            \
        INIT_COEFF_2(term[_c], length2, length3);                   \
} while (0)

// Print Parameters of an N dimensional Array

#define PRINT_PARAM_0(term) (printParameters_0(#term, tparams, i), i+=1)
#define PRINT_PARAM_1(term, A) (printParameters_1(#term, tparams, i, A), i+=A)
#define PRINT_PARAM_2(term, A, B) (printParameters_2(#term, tparams, i, A, B), i+=A*B)
#define PRINT_PARAM_3(term, A, B, C) (printParameters_3(#term, tparams, i, A, B, C), i+=A*B*C)

#endif
