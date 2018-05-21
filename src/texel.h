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

// Depth for evaluating each position
#define TEXEL_DEPTH (0)

// Number of Positions in the data set
#define NP (1470000)

// Every tunable component of the evaluation includes a definition of
// TuneParamName, which is equal to an ON/OFF flag, multipled by the
// number of terms associated with the evaluation component.

// Define the Piece Value Terms
#define TunePawnValue   (0 * 1)
#define TuneKnightValue (0 * 1)
#define TuneBishopValue (0 * 1)
#define TuneRookValue   (0 * 1)
#define TuneQueenValue  (0 * 1)
#define TuneKingValue   (0 * 1)

// Define the Piece Square Table Terms
#define TunePawnPSQT   (0 * 32)
#define TuneKnightPSQT (0 * 32)
#define TuneBishopPSQT (0 * 32)
#define TuneRookPSQT   (0 * 32)
#define TuneQueenPSQT  (0 * 32)
#define TuneKingPSQT   (0 * 32)

// Define the Pawn Terms
#define TunePawnIsolated  (0 *  1)
#define TunePawnStacked   (0 *  1)
#define TunePawnBackwards (0 *  2)
#define TunePawnConnected (0 * 32)

// Define the Knight Terms
#define TuneKnightRammedPawns (0 * 1)
#define TuneKnightOutpost     (0 * 2)
#define TuneKnightMobility    (0 * 9)

// Define the Bishop Terms
#define TuneBishopPair        (0 *  1)
#define TuneBishopRammedPawns (0 *  1)
#define TuneBishopOutpost     (0 *  2)
#define TuneBishopMobility    (0 * 14)

// Define the Rook Terms
#define TuneRookFile      (0 *  2)
#define TuneRookOnSeventh (0 *  1)
#define TuneRookMobility  (0 * 15)

// Define the Queen Terms
#define TuneQueenMobility (0 * 28)

// Define the King Terms
#define TuneKingDefenders (0 *  12)
#define TuneKingShelter   (0 * 128)

// Define the Passed Pawn Terms
#define TunePassedPawn (1 * 32)

// Define the Threat Terms
#define TuneThreatPawnAttackedByOne     (0 * 1)
#define TuneThreatMinorAttackedByPawn   (0 * 1)
#define TuneThreatMinorAttackedByMajor  (0 * 1)
#define TuneThreatQueenAttackedByOne    (0 * 1)

// Compute Number Of Terms (NT) based on what is turned on and off
#define NT (                                                                                           \
    TunePawnValue                  + TuneKnightValue                + TuneBishopValue                + \
    TuneRookValue                  + TuneQueenValue                 + TuneKingValue                  + \
    TunePawnPSQT                   + TuneKnightPSQT                 + TuneBishopPSQT                 + \
    TuneRookPSQT                   + TuneQueenPSQT                  + TuneKingPSQT                   + \
    TunePawnIsolated               + TunePawnStacked                + TunePawnBackwards              + \
    TunePawnConnected              + TuneKnightRammedPawns          + TuneKnightOutpost              + \
    TuneKnightMobility             + TuneBishopPair                 + TuneBishopRammedPawns          + \
    TuneBishopOutpost              + TuneBishopMobility             + TuneRookFile                   + \
    TuneRookOnSeventh              + TuneRookMobility               + TuneQueenMobility              + \
    TuneKingDefenders              + TuneKingShelter                + TunePassedPawn                 + \
    TuneThreatPawnAttackedByOne    + TuneThreatMinorAttackedByPawn  + TuneThreatMinorAttackedByMajor + \
    TuneThreatQueenAttackedByOne                                                                       \
)

// Try to figure out how much we should allocate for the tuner
#define STACKSIZE ((int)((double) NP * NT / 64))

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

void initializeTexelEntries(TexelEntry* tes, Thread* thread);

void initializeCoefficients(int coeffs[NT]);

void initializeCurrentParameters(double cparams[NT][PHASE_NB]);

void calculateLearningRates(TexelEntry* tes, double rates[NT][PHASE_NB]);

void printParameters(double params[NT][PHASE_NB], double cparams[NT][PHASE_NB]);

double computeOptimalK(TexelEntry* tes);

double completeEvaluationError(TexelEntry* tes, double K);

double completeLinearError(TexelEntry* tes, double params[NT][PHASE_NB], double K);

double singleLinearError(TexelEntry te, double params[NT][PHASE_NB], double K);

double linearEvaluation(TexelEntry te, double params[NT][PHASE_NB]);

double sigmoid(double K, double S);

#endif
