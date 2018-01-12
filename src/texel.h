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

#define NP (2000000)

#define NT (386)

typedef struct TexelEntry {
    double result;
    double coeffs[NT];
    double eval, phase;
    double factors[PHASE_NB];
} TexelEntry;

void runTexelTuning(Thread* thread);

void initializeTexelEntries(TexelEntry* tes, Thread* thread);

void initializeCoefficients(TexelEntry* te);

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
