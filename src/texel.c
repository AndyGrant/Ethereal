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

#ifdef TUNE

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboards.h"
#include "board.h"
#include "evaluate.h"
#include "history.h"
#include "move.h"
#include "piece.h"
#include "psqt.h"
#include "search.h"
#include "texel.h"
#include "thread.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"

// Internal Memory Managment
TexelTuple* TupleStack;
int TupleStackSize = STACKSIZE;

// Tap into evaluate()
extern EvalTrace T, EmptyTrace;

extern const int PawnValue;
extern const int KnightValue;
extern const int BishopValue;
extern const int RookValue;
extern const int QueenValue;
extern const int KingValue;
extern const int PawnPSQT32[32];
extern const int KnightPSQT32[32];
extern const int BishopPSQT32[32];
extern const int RookPSQT32[32];
extern const int QueenPSQT32[32];
extern const int KingPSQT32[32];
extern const int PawnIsolated;
extern const int PawnStacked;
extern const int PawnBackwards[2];
extern const int PawnConnected32[32];
extern const int KnightRammedPawns;
extern const int KnightOutpost[2];
extern const int KnightMobility[9];
extern const int BishopPair;
extern const int BishopRammedPawns;
extern const int BishopOutpost[2];
extern const int BishopMobility[14];
extern const int RookFile[2];
extern const int RookOnSeventh;
extern const int RookMobility[15];
extern const int QueenMobility[28];
extern const int KingDefenders[12];
extern const int KingShelter[2][FILE_NB][RANK_NB];
extern const int PassedPawn[2][2][RANK_NB];
extern const int ThreatPawnAttackedByOne;
extern const int ThreatMinorAttackedByPawn;
extern const int ThreatMinorAttackedByMajor;
extern const int ThreatQueenAttackedByOne;
extern const int ThreatOverloadedPieces;

void runTexelTuning(Thread *thread) {

    TexelEntry *tes;
    int i, j, iteration = -1;
    double K, thisError, bestError = 1e6, baseRate = 10.0;
    double rates[NTERMS][PHASE_NB] = {{0}, {0}};
    double params[NTERMS][PHASE_NB] = {{0}, {0}};
    double cparams[NTERMS][PHASE_NB] = {{0}, {0}};

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\nTuner Will Be Tuning %d Terms...", NTERMS);

    printf("\n\nAllocating Memory for Texel Entries [%dMB]...",
           (int)(NPOSITIONS * sizeof(TexelEntry) / (1024 * 1024)));
    tes = calloc(NPOSITIONS, sizeof(TexelEntry));

    printf("\n\nAllocating Memory for Texel Tuple Stack [%dMB]....  [%7d of %7d]",
           (int)(STACKSIZE * sizeof(TexelTuple) / (1024 * 1024)), 0, NPOSITIONS);
    TupleStack = calloc(STACKSIZE, sizeof(TexelTuple));

    printf("\n\nReading and Initializing Texel Entries from FENS...");
    initTexelEntries(tes, thread);

    printf("\n\nFetching Current Evaluation Terms as a Starting Point...");
    initCurrentParameters(cparams);

    printf("\n\nScaling Params For Phases and Occurance Rates...");
    initLearningRates(tes, rates);

    printf("\n\nComputing Optimal K Value...\n");
    K = computeOptimalK(tes);

    while (1) {

        iteration++;

        if (iteration % 25 == 0) {

            // Check for a regression in the tuning process
            thisError = completeLinearError(tes, params, K);
            if (thisError >= bestError)
                break;

            // Update our best and record the current parameters
            bestError = thisError;
            printParameters(params, cparams);
            printf("\nIteration [%d] Error = %g \n", iteration, bestError);
        }

        double gradients[NTERMS][PHASE_NB] = {{0}, {0}};
        #pragma omp parallel shared(gradients)
        {
            double localgradients[NTERMS][PHASE_NB] = {{0}, {0}};
            #pragma omp for schedule(static, NPOSITIONS / NTHREADS)
            for (i = 0; i < NPOSITIONS; i++) {

                thisError = singleLinearError(tes[i], params, K);

                for (j = 0; j < tes[i].ntuples; j++) {

                    // Update gradients for the j-th tuple for the mid game
                    localgradients[tes[i].tuples[j].index][MG] += thisError
                                                                * tes[i].factors[MG]
                                                                * tes[i].tuples[j].coeff;

                    // Update gradients for the j-th tuple for the end game
                    localgradients[tes[i].tuples[j].index][EG] += thisError
                                                                * tes[i].factors[EG]
                                                                * tes[i].tuples[j].coeff;
                }
            }

            // Collapase all of the local gradients into the main gradient. This is done
            // in order to speed up memory access times when doing the tuning with SMP
            for (i = 0; i < NTERMS; i++) {
                gradients[i][MG] += localgradients[i][MG];
                gradients[i][EG] += localgradients[i][EG];
            }
        }

        // Finally, perform the update step of SGD. If we were to properly compute the gradients
        // each term would be divided by -2 over NPOSITIONS. Instead we avoid those divisions until the
        // final update step. Note that we have also simplified the minus off of the 2.
        for (i = 0; i < NTERMS; i++) {
            params[i][MG] += (2.0 / NPOSITIONS) * baseRate * rates[i][MG] * gradients[i][MG];
            params[i][EG] += (2.0 / NPOSITIONS) * baseRate * rates[i][EG] * gradients[i][EG];
        }
    }
}

void initTexelEntries(TexelEntry *tes, Thread *thread) {

    int i, j, k;
    Undo undo[1];
    Limits limits;
    int coeffs[NTERMS];
    char line[128];

    // Initialize limits for the search
    limits.limitedByNone  = 0;
    limits.limitedByTime  = 0;
    limits.limitedByDepth = 1;
    limits.limitedBySelf  = 0;
    limits.timeLimit      = 0;
    limits.depthLimit     = 1;

    // Initialize the thread for the search
    thread->limits = &limits;
    thread->depth  = 1;

    FILE * fin = fopen("FENS", "r");

    for (i = 0; i < NPOSITIONS; i++) {

        if ((i + 1) % 100000 == 0 || i == NPOSITIONS - 1)
            printf("\rReading and Initializing Texel Entries from FENS...  [%7d of %7d]", i + 1, NPOSITIONS);

        fgets(line, 128, fin);

        // Determine the result of the game
        if      (strstr(line, "1-0")) tes[i].result = 1.0;
        else if (strstr(line, "0-1")) tes[i].result = 0.0;
        else if (strstr(line, "1/2")) tes[i].result = 0.5;
        else    {printf("Unable to Parse Result: %s\n", line); exit(0);}

        // Setup the board with the FEN from the FENS file
        boardFromFEN(&thread->board, line);

        // Resolve FEN to a quiet position
        qsearch(thread, &thread->pv, -MATE, MATE, 0);
        for (j = 0; j < thread->pv.length; j++)
            applyMove(&thread->board, thread->pv.line[j], undo);

        // Prepare coefficients and get a WHITE POV eval
        T = EmptyTrace;
        tes[i].eval = evaluateBoard(&thread->board, NULL);
        if (thread->board.turn == BLACK) tes[i].eval *= -1;

        // Determine the game phase based on remaining material
        tes[i].phase = 24 - 4 * popcount(thread->board.pieces[QUEEN ])
                          - 2 * popcount(thread->board.pieces[ROOK  ])
                          - 1 * popcount(thread->board.pieces[KNIGHT])
                          - 1 * popcount(thread->board.pieces[BISHOP]);

        // Compute phase factors for updating the gradients
        tes[i].factors[MG] = 1 - tes[i].phase / 24.0;
        tes[i].factors[EG] = 0 + tes[i].phase / 24.0;

        // Finish with the usual phase for the evaluation
        tes[i].phase = (tes[i].phase * 256 + 12) / 24.0;

        // Vectorize the evaluation coefficients into coeffs
        initCoefficients(coeffs);

        // Determine how many TexelTuples will be needed
        for (k = 0, j = 0; j < NTERMS; j++)
            k += coeffs[j] != 0;

        // Determine if we need to allocate more Texel Tuples
        if (k > TupleStackSize) {
            TupleStackSize = STACKSIZE;
            TupleStack = calloc(STACKSIZE, sizeof(TexelTuple));

            printf("\rAllocating Memory for Texel Tuple Stack [%dMB]...\n\n",
                    (int)(STACKSIZE * sizeof(TexelTuple) / (1024 * 1024)));
        }

        // Tell the Texel Entry where its Texel Tuples are
        tes[i].tuples = TupleStack;
        tes[i].ntuples = k;
        TupleStack += k;
        TupleStackSize -= k;

        // Finally, initialize the Texel Tuples
        for (k = 0, j = 0; j < NTERMS; j++) {
            if (coeffs[j] != 0){
                tes[i].tuples[k].index = j;
                tes[i].tuples[k++].coeff = coeffs[j];
            }
        }
    }

    fclose(fin);
}

void initLearningRates(TexelEntry* tes, double rates[NTERMS][PHASE_NB]) {

    int index, coeff;
    double avgByPhase[PHASE_NB] = {0};
    double occurances[NTERMS][PHASE_NB] = {{0}, {0}};

    for (int i = 0; i < NPOSITIONS; i++) {
        for (int j = 0; j < tes[i].ntuples; j++) {

            index = tes[i].tuples[j].index;
            coeff = tes[i].tuples[j].coeff;

            occurances[index][MG] += abs(coeff) * tes[i].factors[MG];
            occurances[index][EG] += abs(coeff) * tes[i].factors[EG];

            avgByPhase[MG] += abs(coeff) * tes[i].factors[MG];
            avgByPhase[EG] += abs(coeff) * tes[i].factors[EG];
        }
    }

    avgByPhase[MG] /= NTERMS;
    avgByPhase[EG] /= NTERMS;

    for (int i = 0; i < NTERMS; i++){
        if (occurances[i][MG] >= 1.0)
            rates[i][MG] = avgByPhase[MG] / occurances[i][MG];
        if (occurances[i][EG] >= 1.0)
            rates[i][EG] = avgByPhase[EG] / occurances[i][EG];
    }
}

void initCoefficients(int coeffs[NTERMS]) {

    int i = 0; // INIT_COEFF_N will update i accordingly

    if (TunePawnValue                 ) INIT_COEFF_0(PawnValue)                 ;
    if (TuneKnightValue               ) INIT_COEFF_0(KnightValue)               ;
    if (TuneBishopValue               ) INIT_COEFF_0(BishopValue)               ;
    if (TuneRookValue                 ) INIT_COEFF_0(RookValue)                 ;
    if (TuneQueenValue                ) INIT_COEFF_0(QueenValue)                ;
    if (TuneKingValue                 ) INIT_COEFF_0(KingValue)                 ;
    if (TunePawnPSQT32                ) INIT_COEFF_1(PawnPSQT32, 32)            ;
    if (TuneKnightPSQT32              ) INIT_COEFF_1(KnightPSQT32, 32)          ;
    if (TuneBishopPSQT32              ) INIT_COEFF_1(BishopPSQT32, 32)          ;
    if (TuneRookPSQT32                ) INIT_COEFF_1(RookPSQT32, 32)            ;
    if (TuneQueenPSQT32               ) INIT_COEFF_1(QueenPSQT32, 32)           ;
    if (TuneKingPSQT32                ) INIT_COEFF_1(KingPSQT32, 32)            ;
    if (TunePawnIsolated              ) INIT_COEFF_0(PawnIsolated)              ;
    if (TunePawnStacked               ) INIT_COEFF_0(PawnStacked)               ;
    if (TunePawnBackwards             ) INIT_COEFF_1(PawnBackwards, 2)          ;
    if (TunePawnConnected32           ) INIT_COEFF_1(PawnConnected32, 32)       ;
    if (TuneKnightRammedPawns         ) INIT_COEFF_0(KnightRammedPawns)         ;
    if (TuneKnightOutpost             ) INIT_COEFF_1(KnightOutpost, 2)          ;
    if (TuneKnightMobility            ) INIT_COEFF_1(KnightMobility, 9)         ;
    if (TuneBishopPair                ) INIT_COEFF_0(BishopPair)                ;
    if (TuneBishopRammedPawns         ) INIT_COEFF_0(BishopRammedPawns)         ;
    if (TuneBishopOutpost             ) INIT_COEFF_1(BishopOutpost, 2)          ;
    if (TuneBishopMobility            ) INIT_COEFF_1(BishopMobility, 14)        ;
    if (TuneRookFile                  ) INIT_COEFF_1(RookFile, 2)               ;
    if (TuneRookOnSeventh             ) INIT_COEFF_0(RookOnSeventh)             ;
    if (TuneRookMobility              ) INIT_COEFF_1(RookMobility, 15)          ;
    if (TuneQueenMobility             ) INIT_COEFF_1(QueenMobility, 28)         ;
    if (TuneKingDefenders             ) INIT_COEFF_1(KingDefenders, 12)         ;
    if (TuneKingShelter               ) INIT_COEFF_3(KingShelter, 2, 8, 8)      ;
    if (TunePassedPawn                ) INIT_COEFF_3(PassedPawn, 2, 2, 8)       ;
    if (TuneThreatPawnAttackedByOne   ) INIT_COEFF_0(ThreatPawnAttackedByOne)   ;
    if (TuneThreatMinorAttackedByPawn ) INIT_COEFF_0(ThreatMinorAttackedByPawn) ;
    if (TuneThreatMinorAttackedByMajor) INIT_COEFF_0(ThreatMinorAttackedByMajor);
    if (TuneThreatQueenAttackedByOne  ) INIT_COEFF_0(ThreatQueenAttackedByOne)  ;
    if (TuneThreatOverloadedPieces    ) INIT_COEFF_0(ThreatOverloadedPieces)    ;

    if (i != NTERMS){
        printf("Error in initCoefficients(): i = %d ; NTERMS = %d\n", i, NTERMS);
        exit(EXIT_FAILURE);
    }
}

void initCurrentParameters(double cparams[NTERMS][PHASE_NB]) {

    int i = 0; // INIT_PARAM_N will update i accordingly

    if (TunePawnValue                 ) INIT_PARAM_0(PawnValue)                 ;
    if (TuneKnightValue               ) INIT_PARAM_0(KnightValue)               ;
    if (TuneBishopValue               ) INIT_PARAM_0(BishopValue)               ;
    if (TuneRookValue                 ) INIT_PARAM_0(RookValue)                 ;
    if (TuneQueenValue                ) INIT_PARAM_0(QueenValue)                ;
    if (TuneKingValue                 ) INIT_PARAM_0(KingValue)                 ;
    if (TunePawnPSQT32                ) INIT_PARAM_1(PawnPSQT32, 32)            ;
    if (TuneKnightPSQT32              ) INIT_PARAM_1(KnightPSQT32, 32)          ;
    if (TuneBishopPSQT32              ) INIT_PARAM_1(BishopPSQT32, 32)          ;
    if (TuneRookPSQT32                ) INIT_PARAM_1(RookPSQT32, 32)            ;
    if (TuneQueenPSQT32               ) INIT_PARAM_1(QueenPSQT32, 32)           ;
    if (TuneKingPSQT32                ) INIT_PARAM_1(KingPSQT32, 32)            ;
    if (TunePawnIsolated              ) INIT_PARAM_0(PawnIsolated)              ;
    if (TunePawnStacked               ) INIT_PARAM_0(PawnStacked)               ;
    if (TunePawnBackwards             ) INIT_PARAM_1(PawnBackwards, 2)          ;
    if (TunePawnConnected32           ) INIT_PARAM_1(PawnConnected32, 32)       ;
    if (TuneKnightRammedPawns         ) INIT_PARAM_0(KnightRammedPawns)         ;
    if (TuneKnightOutpost             ) INIT_PARAM_1(KnightOutpost, 2)          ;
    if (TuneKnightMobility            ) INIT_PARAM_1(KnightMobility, 9)         ;
    if (TuneBishopPair                ) INIT_PARAM_0(BishopPair)                ;
    if (TuneBishopRammedPawns         ) INIT_PARAM_0(BishopRammedPawns)         ;
    if (TuneBishopOutpost             ) INIT_PARAM_1(BishopOutpost, 2)          ;
    if (TuneBishopMobility            ) INIT_PARAM_1(BishopMobility, 14)        ;
    if (TuneRookFile                  ) INIT_PARAM_1(RookFile, 2)               ;
    if (TuneRookOnSeventh             ) INIT_PARAM_0(RookOnSeventh)             ;
    if (TuneRookMobility              ) INIT_PARAM_1(RookMobility, 15)          ;
    if (TuneQueenMobility             ) INIT_PARAM_1(QueenMobility, 28)         ;
    if (TuneKingDefenders             ) INIT_PARAM_1(KingDefenders, 12)         ;
    if (TuneKingShelter               ) INIT_PARAM_3(KingShelter, 2, 8, 8)      ;
    if (TunePassedPawn                ) INIT_PARAM_3(PassedPawn, 2, 2, 8)       ;
    if (TuneThreatPawnAttackedByOne   ) INIT_PARAM_0(ThreatPawnAttackedByOne)   ;
    if (TuneThreatMinorAttackedByPawn ) INIT_PARAM_0(ThreatMinorAttackedByPawn) ;
    if (TuneThreatMinorAttackedByMajor) INIT_PARAM_0(ThreatMinorAttackedByMajor);
    if (TuneThreatQueenAttackedByOne  ) INIT_PARAM_0(ThreatQueenAttackedByOne)  ;
    if (TuneThreatOverloadedPieces    ) INIT_PARAM_0(ThreatOverloadedPieces)    ;

    if (i != NTERMS){
        printf("Error in initCurrentParameters(): i = %d ; NTERMS = %d\n", i, NTERMS);
        exit(EXIT_FAILURE);
    }
}

void printParameters(double params[NTERMS][PHASE_NB], double cparams[NTERMS][PHASE_NB]) {

    int i = 0; // PRINT_PARAM_N will update i accordingly
    int tparams[NTERMS][PHASE_NB];
    int pvalue = ScoreMG(PawnValue) + (TunePawnValue ? params[0][MG] : 0);

    // Combine original and updated, scale so PawnValue[MG] = 100
    for (int j = 0; j < NTERMS; j++) {
        tparams[j][MG] = (int)((100.0 / pvalue) * (params[j][MG] + cparams[j][MG]));
        tparams[j][EG] = (int)((100.0 / pvalue) * (params[j][EG] + cparams[j][EG]));
    }

    if (TunePawnValue                 ) PRINT_PARAM_0(PawnValue)                 ;
    if (TuneKnightValue               ) PRINT_PARAM_0(KnightValue)               ;
    if (TuneBishopValue               ) PRINT_PARAM_0(BishopValue)               ;
    if (TuneRookValue                 ) PRINT_PARAM_0(RookValue)                 ;
    if (TuneQueenValue                ) PRINT_PARAM_0(QueenValue)                ;
    if (TuneKingValue                 ) PRINT_PARAM_0(KingValue)                 ;
    if (TunePawnPSQT32                ) PRINT_PARAM_1(PawnPSQT32, 32)            ;
    if (TuneKnightPSQT32              ) PRINT_PARAM_1(KnightPSQT32, 32)          ;
    if (TuneBishopPSQT32              ) PRINT_PARAM_1(BishopPSQT32, 32)          ;
    if (TuneRookPSQT32                ) PRINT_PARAM_1(RookPSQT32, 32)            ;
    if (TuneQueenPSQT32               ) PRINT_PARAM_1(QueenPSQT32, 32)           ;
    if (TuneKingPSQT32                ) PRINT_PARAM_1(KingPSQT32, 32)            ;
    if (TunePawnIsolated              ) PRINT_PARAM_0(PawnIsolated)              ;
    if (TunePawnStacked               ) PRINT_PARAM_0(PawnStacked)               ;
    if (TunePawnBackwards             ) PRINT_PARAM_1(PawnBackwards, 2)          ;
    if (TunePawnConnected32           ) PRINT_PARAM_1(PawnConnected32, 32)       ;
    if (TuneKnightRammedPawns         ) PRINT_PARAM_0(KnightRammedPawns)         ;
    if (TuneKnightOutpost             ) PRINT_PARAM_1(KnightOutpost, 2)          ;
    if (TuneKnightMobility            ) PRINT_PARAM_1(KnightMobility, 9)         ;
    if (TuneBishopPair                ) PRINT_PARAM_0(BishopPair)                ;
    if (TuneBishopRammedPawns         ) PRINT_PARAM_0(BishopRammedPawns)         ;
    if (TuneBishopOutpost             ) PRINT_PARAM_1(BishopOutpost, 2)          ;
    if (TuneBishopMobility            ) PRINT_PARAM_1(BishopMobility, 14)        ;
    if (TuneRookFile                  ) PRINT_PARAM_1(RookFile, 2)               ;
    if (TuneRookOnSeventh             ) PRINT_PARAM_0(RookOnSeventh)             ;
    if (TuneRookMobility              ) PRINT_PARAM_1(RookMobility, 15)          ;
    if (TuneQueenMobility             ) PRINT_PARAM_1(QueenMobility, 28)         ;
    if (TuneKingDefenders             ) PRINT_PARAM_1(KingDefenders, 12)         ;
    if (TuneKingShelter               ) PRINT_PARAM_3(KingShelter, 2, 8, 8)      ;
    if (TunePassedPawn                ) PRINT_PARAM_3(PassedPawn, 2, 2, 8)       ;
    if (TuneThreatPawnAttackedByOne   ) PRINT_PARAM_0(ThreatPawnAttackedByOne)   ;
    if (TuneThreatMinorAttackedByPawn ) PRINT_PARAM_0(ThreatMinorAttackedByPawn) ;
    if (TuneThreatMinorAttackedByMajor) PRINT_PARAM_0(ThreatMinorAttackedByMajor);
    if (TuneThreatQueenAttackedByOne  ) PRINT_PARAM_0(ThreatQueenAttackedByOne)  ;
    if (TuneThreatOverloadedPieces    ) PRINT_PARAM_0(ThreatOverloadedPieces)    ;

    if (i != NTERMS){
        printf("Error in printParameters(): i = %d ; NTERMS = %d\n", i, NTERMS);
        exit(EXIT_FAILURE);
    }
}

double computeOptimalK(TexelEntry* tes) {

    int i;
    double start = -10.0, end = 10.0, delta = 1.0;
    double curr = start, thisError, bestError = completeEvaluationError(tes, start);

    for (i = 0; i < 10; i++) {
        printf("Computing K Iteration [%d] ", i);

        // Find the best value if K within the range [start, end],
        // with a step size based on the current iteration
        curr = start - delta;
        while (curr < end) {
            curr = curr + delta;
            thisError = completeEvaluationError(tes, curr);
            if (thisError <= bestError)
                bestError = thisError, start = curr;
        }

        printf("K = %f E = %f\n", start, bestError);

        // Narrow our search to [best - delta, best + delta]
        end = start + delta;
        start = start - delta;
        delta = delta / 10.0;
    }

    return start;
}

double completeEvaluationError(TexelEntry* tes, double K) {

    double total = 0.0;

    #pragma omp parallel shared(total)
    {
        #pragma omp for schedule(static, NPOSITIONS / NTHREADS) reduction(+:total)
        for (int i = 0; i < NPOSITIONS; i++)
            total += pow(tes[i].result - sigmoid(K, tes[i].eval), 2);
    }

    return total / (double)NPOSITIONS;
}

double completeLinearError(TexelEntry* tes, double params[NTERMS][PHASE_NB], double K) {

    double total = 0.0;

    #pragma omp parallel shared(total)
    {
        #pragma omp for schedule(static, NPOSITIONS / NTHREADS) reduction(+:total)
        for (int i = 0; i < NPOSITIONS; i++)
            total += pow(tes[i].result - sigmoid(K, linearEvaluation(tes[i], params)), 2);
    }

    return total / (double)NPOSITIONS;
}

double singleLinearError(TexelEntry te, double params[NTERMS][PHASE_NB], double K) {
    return te.result - sigmoid(K, linearEvaluation(te, params));
}

double linearEvaluation(TexelEntry te, double params[NTERMS][PHASE_NB]) {

    double mg = 0, eg = 0;

    for (int i = 0; i < te.ntuples; i++) {
        mg += te.tuples[i].coeff * params[te.tuples[i].index][MG];
        eg += te.tuples[i].coeff * params[te.tuples[i].index][EG];
    }

    return te.eval + ((mg * (256 - te.phase) + eg * te.phase) / 256.0);
}

double sigmoid(double K, double S) {
    return 1.0 / (1.0 + pow(10.0, -K * S / 400.0));
}

void printParameters_0(char *name, int params[NTERMS][PHASE_NB], int i) {
    printf("const int %s = S(%4d,%4d);\n", name, params[i][MG], params[i][EG]);
    i++;
}

void printParameters_1(char *name, int params[NTERMS][PHASE_NB], int i, int A) {

    printf("const int %s[%d] = {", name, A);

    for (int a = 0; a < A; a++, i++) {
        if (a % 4 == 0) printf("\n    ");
        printf("S(%4d,%4d), ", params[i][MG], params[i][EG]);
    }

    printf("\n};\n");
}

void printParameters_2(char *name, int params[NTERMS][PHASE_NB], int i, int A, int B) {

    (void)name, (void)params, (void)i, (void)A, (void)B;

    printf("PRINT_PARAM_2 IS NOT ENABLED!\n");
    exit(EXIT_FAILURE);

}

void printParameters_3(char *name, int params[NTERMS][PHASE_NB], int i, int A, int B, int C) {

    printf("const int %s[%d][%d][%d] = {\n", name, A, B, C);

    for (int a = 0; a < A; a++) {

        for (int b = 0; b < B; b++) {

            printf("%s", b ? "   {" : "  {{");;

            for (int c = 0; c < C; c++, i++) {
                printf("S(%4d,%4d)", params[i][MG], params[i][EG]);
                printf("%s", c == C - 1 ? "" : ", ");
            }

            printf("%s", b == B - 1 ? "}},\n" : "},\n");
        }

    }

    printf("};\n");
}

#endif
