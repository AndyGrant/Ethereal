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

// Number of Positions in the data set
#define NP (147000)

// Every tunable component of the evaluation includes a definition of
// TuneParamName, which is equal to an ON/OFF flag, multipled by the
// number of terms associated with the evaluation component.

// Define the Piece Value Terms
#define TunePawnValue            (1 * 1  )
#define TuneKnightValue          (1 * 1  )
#define TuneBishopValue          (1 * 1  )
#define TuneRookValue            (1 * 1  )
#define TuneQueenValue           (1 * 1  )
#define TuneKingValue            (1 * 1  )

// Define the Piece Square Table Terms
#define TunePawnPSQT             (1 * 32 )
#define TuneKnightPSQT           (1 * 32 )
#define TuneBishopPSQT           (1 * 32 )
#define TuneRookPSQT             (1 * 32 )
#define TuneQueenPSQT            (1 * 32 )
#define TuneKingPSQT             (1 * 32 )

// Define the Pawn Terms
#define TunePawnIsolated         (1 * 1  )
#define TunePawnStacked          (1 * 1  )
#define TunePawnBackwards        (1 * 2  )
#define TunePawnConnected        (1 * 32 )

// Define the Knight Terms
#define TuneKnightAttackedByPawn (1 * 1  )
#define TuneKnightOutpost        (1 * 2  )
#define TuneKnightMobility       (1 * 9  )

// Define the Bishop Terms
#define TuneBishopPair           (1 * 1  )
#define TuneBishopAttackedByPawn (1 * 1  )
#define TuneBishopOutpost        (1 * 2  )
#define TuneBishopMobility       (1 * 14 )

// Define the Rook Terms
#define TuneRookFile             (1 * 2  )
#define TuneRookOnSeventh        (1 * 1  )
#define TuneRookMobility         (1 * 15 )

// Define the Queen Terms
#define TuneQueenChecked         (1 * 1  )
#define TuneQueenCheckedByPawn   (1 * 1  )
#define TuneQueenMobility        (1 * 28 )

// Define the King Terms
#define TuneKingDefenders        (1 * 12 )
#define TuneKingShelter          (1 * 128)

// Define the Passed Pawn Terms
#define TunePassedPawn           (1 * 32 )

// Compute Number Of Terms (NT) based on what is turned on and off
#define NT (                                                                   \
    TunePawnValue            + TuneKnightValue          + TuneBishopValue    + \
    TuneRookValue            + TuneQueenValue           + TuneKingValue      + \
    TunePawnPSQT             + TuneKnightPSQT           + TuneBishopPSQT     + \
    TuneRookPSQT             + TuneQueenPSQT            + TuneKingPSQT       + \
    TunePawnIsolated         + TunePawnStacked          + TunePawnBackwards  + \
    TunePawnConnected        + TuneKnightAttackedByPawn + TuneKnightOutpost  + \
    TuneKnightMobility                                  + TuneBishopPair     + \
    TuneBishopAttackedByPawn + TuneBishopOutpost        + TuneBishopMobility + \
    TuneRookFile             + TuneRookOnSeventh        + TuneRookMobility   + \
    TuneQueenChecked         + TuneQueenCheckedByPawn   + TuneQueenMobility  + \
    TuneKingDefenders        + TuneKingShelter          + TunePassedPawn       \
)

// Try to figure out how much we should allocate for the tuner
#define STACKSIZE ((int)((double) NP * NT / 64))

typedef struct TexelTuple {
    int index;
    int coeff;
} TexelTuple;

typedef struct TexelEntry {
    int ntuples;
    double result;
    double eval, phase;
    double factors[PHASE_NB];
    TexelTuple* tuples;
} TexelEntry;

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
