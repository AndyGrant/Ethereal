/******************************************************************************/
/*                                                                            */
/*    Ethereal is a UCI chess playing engine authored by Andrew Grant.        */
/*    <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>        */
/*                                                                            */
/*    Ethereal is free software: you can redistribute it and/or modify        */
/*    it under the terms of the GNU General Public License as published by    */
/*    the Free Software Foundation, either version 3 of the License, or       */
/*    (at your option) any later version.                                     */
/*                                                                            */
/*    Ethereal is distributed in the hope that it will be useful,             */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*    GNU General Public License for more details.                            */
/*                                                                            */
/*    You should have received a copy of the GNU General Public License       */
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>    */
/*                                                                            */
/******************************************************************************/

#pragma once

#include <stdlib.h>

#include "../types.h"

#if defined(USE_AVX2)
    #include "archs/avx2.h"
#elif defined(USE_AVX)
    #include "archs/avx.h"
#elif defined(USE_SSSE3)
    #include "archs/ssse3.h"
#endif

#define INSIZE  20480
#define KPSIZE  768
#define L1SIZE  1536
#define L2SIZE  8
#define L3SIZE  32
#define OUTSIZE 1

#define NUM_REGS 16

typedef struct NNUEDelta {
    int piece, from, to;
} NNUEDelta;

typedef struct NNUEAccumulator {
    int changes, accurate[COLOUR_NB];
    NNUEDelta deltas[3];
    ALIGN64 int16_t values[COLOUR_NB][KPSIZE];
} NNUEAccumulator;

typedef struct NNUEAccumulatorTableEntry {
    NNUEAccumulator accumulator;
    uint64_t occupancy[COLOUR_NB][COLOUR_NB][PIECE_NB-1];
} NNUEAccumulatorTableEntry;

typedef struct NNUEEvaluator {
    NNUEAccumulator stack[MAX_PLY + 4];         // Each ply of search
    NNUEAccumulator *current;                   // Pointer of the current stack location
    NNUEAccumulatorTableEntry table[SQUARE_NB]; // Finny table with Accumulators for each square
} NNUEEvaluator;
