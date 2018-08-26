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
extern const int KnightOutpost[2];
extern const int KnightBehindPawn;
extern const int KnightMobility[9];
extern const int BishopPair;
extern const int BishopRammedPawns;
extern const int BishopOutpost[2];
extern const int BishopBehindPawn;
extern const int BishopMobility[14];
extern const int RookFile[2];
extern const int RookOnSeventh;
extern const int RookMobility[15];
extern const int QueenMobility[28];
extern const int KingDefenders[12];
extern const int KingShelter[2][8][8];
extern const int KingStorm[2][4][8];
extern const int PassedPawn[2][2][8];
extern const int PassedFriendlyDistance;
extern const int PassedEnemyDistance;
extern const int PassedSafePromotionPath;
extern const int ThreatWeakPawn;
extern const int ThreatMinorAttackedByPawn;
extern const int ThreatMinorAttackedByMajor;
extern const int ThreatRookAttackedByLesser;
extern const int ThreatQueenAttackedByOne;
extern const int ThreatOverloadedPieces;
extern const int ThreatByPawnPush;

void runTexelTuning(Thread *thread) {

    TexelEntry *tes;
    int i, j, iteration = -1;
    double K, thisError, bestError = 1e6;
    double params[NTERMS][PHASE_NB] = {0};
    double cparams[NTERMS][PHASE_NB] = {0};

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\nTuner Will Be Tuning %d Terms...", NTERMS);

    printf("\n\nSetting Table size to 1MB for speed...");
    initTT(1);

    printf("\n\nAllocating Memory for Texel Entries [%dKB]...",
           (int)(NPOSITIONS * sizeof(TexelEntry) / 1024));
    tes = calloc(NPOSITIONS, sizeof(TexelEntry));

    printf("\n\nAllocating Memory for Texel Tuple Stack [%dKB]...",
           (int)(STACKSIZE * sizeof(TexelTuple) / 1024));
    TupleStack = calloc(STACKSIZE, sizeof(TexelTuple));

    printf("\n\nInitializing Texel Entries from FENS...");
    initTexelEntries(tes, thread);

    printf("\n\nFetching Current Evaluation Terms as a Starting Point...");
    initCurrentParameters(cparams);

    printf("\n\nComputing Optimal K Value...\n");
    K = computeOptimalK(tes);

    while (1) {

        if (++iteration % REPORTING == 0) {

            // Check for a regression in the tuning process
            thisError = completeLinearError(tes, params, K);
            if (thisError >= bestError)
                break;

            // Update our best and record the current parameters
            bestError = thisError;
            printf("\nIteration [%d] Error = %g \n", iteration, bestError);
            printParameters(params, cparams);
        }

        double gradients[NTERMS][PHASE_NB] = {0};
        #pragma omp parallel shared(gradients)
        {
            double localgradients[NTERMS][PHASE_NB] = {0};
            #pragma omp for schedule(static, NPOSITIONS / NPARTITIONS)
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
            params[i][MG] += (2.0 / NPOSITIONS) * LEARNING * gradients[i][MG];
            params[i][EG] += (2.0 / NPOSITIONS) * LEARNING * gradients[i][EG];
        }
    }
}

void initTexelEntries(TexelEntry *tes, Thread *thread) {

    int i, j, k, eval;
    Undo undo[1];
    Limits limits;
    int coeffs[NTERMS];
    char line[128];

    // Initialize limits for the search
    limits.limitedByNone  = 1;
    limits.limitedByTime  = 0;
    limits.limitedByDepth = 0;
    limits.limitedBySelf  = 0;
    limits.timeLimit      = 0;
    limits.depthLimit     = 0;

    // Initialize the thread for the search
    thread->limits = &limits;
    thread->depth  = 0;

    FILE * fin = fopen("FENS", "r");

    for (i = 0; i < NPOSITIONS; i++) {

        // Read next position from the FEN file
        if (fgets(line, 128, fin) == NULL) {
            printf("Unable to read line #%d\n", i);
            exit(EXIT_FAILURE);
        }

        // Occasional reporting for total completion
        if ((i + 1) % 10000 == 0 || i == NPOSITIONS - 1)
            printf("\rInitializing Texel Entries from FENS...  [%7d of %7d]", i + 1, NPOSITIONS);

        // Determine the result of the game
        if      (strstr(line, "1-0")) tes[i].result = 1.0;
        else if (strstr(line, "0-1")) tes[i].result = 0.0;
        else if (strstr(line, "1/2")) tes[i].result = 0.5;
        else    {printf("Cannot Parse %s\n", line); exit(EXIT_FAILURE);}

        // Clear out all of the hash and history tables. This is extemely slow!
        // For correctness this must be done, but you can likely get away with it.
        if (CLEARING && NDEPTHS) resetThreadPool(thread), clearTT();

        // Setup the board with the FEN from the FENS file
        boardFromFEN(&thread->board, line);

        // Determine the game phase based on remaining material
        tes[i].phase = 24 - 4 * popcount(thread->board.pieces[QUEEN ])
                          - 2 * popcount(thread->board.pieces[ROOK  ])
                          - 1 * popcount(thread->board.pieces[BISHOP])
                          - 1 * popcount(thread->board.pieces[KNIGHT]);

        // Compute phase factors for updating the gradients
        tes[i].factors[MG] = 1 - tes[i].phase / 24.0;
        tes[i].factors[EG] = 0 + tes[i].phase / 24.0;

        // Finish the phase calculation for the evaluation
        tes[i].phase = (tes[i].phase * 256 + 12) / 24.0;

        // Use a iterative deepening to get a predictive evaluation
        for (int depth = 1; depth <= NDEPTHS; depth++)
            tes[i].eval = search(thread, &thread->pv, -MATE, MATE, depth, 0);
        if (thread->board.turn == BLACK) tes[i].eval *= -1;

        // Resolve FEN to a quiet position
        if (RESOLVE) {
            qsearch(thread, &thread->pv, -MATE, MATE, 0);
            for (j = 0; j < thread->pv.length; j++)
                applyMove(&thread->board, thread->pv.line[j], undo);
        }

        // Vectorize the evaluation coefficients
        T = EmptyTrace;
        eval = evaluateBoard(&thread->board, NULL);
        initCoefficients(coeffs);

        // When using NDEPTHS=0, use the proper evaluation
        if (NDEPTHS == 0)
            tes[i].eval = thread->board.turn == WHITE ? eval : -eval;

        // Count up the non zero evaluation terms
        for (k = 0, j = 0; j < NTERMS; j++)
            k += coeffs[j] != 0;

        // Determine if we need to allocate more Texel Tuples
        if (k > TupleStackSize) {
            printf("\n\nAllocating Memory for Texel Tuple Stack [%dKB]...\n\n",
                    (int)(STACKSIZE * sizeof(TexelTuple) / 1024));
            TupleStackSize = STACKSIZE;
            TupleStack = calloc(STACKSIZE, sizeof(TexelTuple));
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

void initCoefficients(int coeffs[NTERMS]) {

    int i = 0; // EXECUTE_ON_TERMS will update i accordingly

    EXECUTE_ON_TERMS(INIT_COEFF);

    if (i != NTERMS){
        printf("Error in initCoefficients(): i = %d ; NTERMS = %d\n", i, NTERMS);
        exit(EXIT_FAILURE);
    }
}

void initCurrentParameters(double cparams[NTERMS][PHASE_NB]) {

    int i = 0; // EXECUTE_ON_TERMS will update i accordingly

    EXECUTE_ON_TERMS(INIT_PARAM);

    if (i != NTERMS){
        printf("Error in initCurrentParameters(): i = %d ; NTERMS = %d\n", i, NTERMS);
        exit(EXIT_FAILURE);
    }
}

void printParameters(double params[NTERMS][PHASE_NB], double cparams[NTERMS][PHASE_NB]) {

    int tparams[NTERMS][PHASE_NB];

    // Combine updated and current parameters
    for (int j = 0; j < NTERMS; j++) {
        tparams[j][MG] = params[j][MG] + cparams[j][MG];
        tparams[j][EG] = params[j][EG] + cparams[j][EG];
    }

    int i = 0; // EXECUTE_ON_TERMS will update i accordingly

    EXECUTE_ON_TERMS(PRINT_PARAM);

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
        #pragma omp for schedule(static, NPOSITIONS / NPARTITIONS) reduction(+:total)
        for (int i = 0; i < NPOSITIONS; i++)
            total += pow(tes[i].result - sigmoid(K, tes[i].eval), 2);
    }

    return total / (double)NPOSITIONS;
}

double completeLinearError(TexelEntry* tes, double params[NTERMS][PHASE_NB], double K) {

    double total = 0.0;

    #pragma omp parallel shared(total)
    {
        #pragma omp for schedule(static, NPOSITIONS / NPARTITIONS) reduction(+:total)
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
