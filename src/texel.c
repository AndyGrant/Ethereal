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

// Our own memory managment system, so that that all of the texel
// tuple structs are, for the most part, storied sequentially in memory
TexelTuple* TupleStack;
int TupleStackSize = STACKSIZE;

// Need these in order to get the coefficients
// for each of the evaluation terms
extern EvalTrace T, EmptyTrace;

// To determine the starting values for the Piece Values
extern const int PawnValue;
extern const int KnightValue;
extern const int BishopValue;
extern const int RookValue;
extern const int QueenValue;
extern const int KingValue;

// To determine the starting values for Piece Square Table Values
extern const int PawnPSQT32[32];
extern const int KnightPSQT32[32];
extern const int BishopPSQT32[32];
extern const int RookPSQT32[32];
extern const int QueenPSQT32[32];
extern const int KingPSQT32[32];

// To determine the starting values for the Pawn terms
extern const int PawnIsolated;
extern const int PawnStacked;
extern const int PawnBackwards[2];
extern const int PawnConnected32[32];

// To determine the starting values for the Knight terms
extern const int KnightRammedPawns;
extern const int KnightOutpost[2];
extern const int KnightMobility[9];

// To determine the starting values for the Bishop terms
extern const int BishopPair;
extern const int BishopRammedPawns;
extern const int BishopOutpost[2];
extern const int BishopMobility[14];

// To determine the starting values for the Rook terms
extern const int RookFile[2];
extern const int RookOnSeventh;
extern const int RookMobility[15];

// To determine the starting values for the Queen terms
extern const int QueenMobility[28];

// To determine the starting values for the King terms
extern const int KingDefenders[12];
extern const int KingShelter[2][FILE_NB][RANK_NB];

// To determine the starting values for the Passed Pawn terms
extern const int PassedPawn[2][2][RANK_NB];

// To determine the starting values for the Threat terms
extern const int ThreatPawnAttackedByOne;
extern const int ThreatMinorAttackedByPawn;
extern const int ThreatMinorAttackedByMajor;
extern const int ThreatQueenAttackedByMinor;
extern const int ThreatQueenAttackedByOne;


void runTexelTuning(Thread* thread){

    TexelEntry* tes;
    int i, j, iteration = -1;
    double K, thisError, bestError = 1e6, baseRate = 10.0;
    double rates[NT][PHASE_NB] = {{0}, {0}};
    double params[NT][PHASE_NB] = {{0}, {0}};
    double cparams[NT][PHASE_NB] = {{0}, {0}};

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\nTuner Will Be Tuning %d Terms...", NT);

    printf("\n\nSetting Transposition Table to 1MB...");
    initTT(1);

    printf("\n\nAllocating Memory for Texel Entries [%dMB]...",
           (int)(NP * sizeof(TexelEntry) / (1024 * 1024)));
    tes = calloc(NP, sizeof(TexelEntry));

    printf("\n\nAllocating Memory for Texel Tuple Stack [%dMB]...",
           (int)(STACKSIZE * sizeof(TexelTuple) / (1024 * 1024)));
    TupleStack = calloc(STACKSIZE, sizeof(TexelTuple));

    printf("\n\nReading and Initializing Texel Entries from FENS...");
    initializeTexelEntries(tes, thread);

    printf("\n\nFetching Current Evaluation Terms as a Starting Point...");
    initializeCurrentParameters(cparams);

    printf("\n\nScaling Params For Phases and Occurance Rates...");
    calculateLearningRates(tes, rates);

    printf("\n\nComputing Optimal K Value...\n");
    K = computeOptimalK(tes);

    while (1){

        iteration++;

        if (iteration % 25 == 0){

            // Check for a regression in the tuning process
            thisError = completeLinearError(tes, params, K);
            if (thisError >= bestError)
                break;

            // Update our best and record the current parameters
            bestError = thisError;
            printParameters(params, cparams);
            printf("\nIteration [%d] Error = %g \n", iteration, bestError);
        }

        double gradients[NT][PHASE_NB] = {{0}, {0}};
        #pragma omp parallel shared(gradients)
        {
            double localgradients[NT][PHASE_NB] = {{0}, {0}};
            #pragma omp for schedule(static, NP / 48)
            for (i = 0; i < NP; i++){

                thisError = singleLinearError(tes[i], params, K);

                for (j = 0; j < tes[i].ntuples; j++){

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
            for (i = 0; i < NT; i++){
                gradients[i][MG] += localgradients[i][MG];
                gradients[i][EG] += localgradients[i][EG];
            }
        }

        // Finally, perform the update step of SGD. If we were to properly compute the gradients
        // each term would be divided by -2 over NP. Instead we avoid those divisions until the
        // final update step. Note that we have also simplified the minus off of the 2.
        for (i = 0; i < NT; i++){
            params[i][MG] += (2.0 / NP) * baseRate * rates[i][MG] * gradients[i][MG];
            params[i][EG] += (2.0 / NP) * baseRate * rates[i][EG] * gradients[i][EG];
        }
    }
}

void initializeTexelEntries(TexelEntry* tes, Thread* thread){

    int i, j, k;
    Undo undo[1];
    Limits limits;
    int coeffs[NT];
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

    for (i = 0; i < NP; i++){

        if ((i + 1) % 100000 == 0 || i == NP - 1)
            printf("\rReading and Initializing Texel Entries from FENS...  [%7d of %7d]", i + 1, NP);

        fgets(line, 128, fin);

        // Determine the result of the game
        if      (strstr(line, "1-0")) tes[i].result = 1.0;
        else if (strstr(line, "0-1")) tes[i].result = 0.0;
        else if (strstr(line, "1/2")) tes[i].result = 0.5;
        else    {printf("Unable to Parse Result: %s\n", line); exit(0);}

        // Setup the board with the FEN from the FENS file
        boardFromFEN(&thread->board, line);

        // Determine the game phase based on remaining material
        tes[i].phase = 24 - 4 * popcount(thread->board.pieces[QUEEN ])
                          - 2 * popcount(thread->board.pieces[ROOK  ])
                          - 1 * popcount(thread->board.pieces[KNIGHT])
                          - 1 * popcount(thread->board.pieces[BISHOP]);

        // Use the search value as the evaluation, to provide a better
        // understanding the potential of a position's eval terms
        tes[i].eval = search(thread, &thread->pv, -MATE, MATE, TEXEL_DEPTH, 0);
        if (thread->board.turn == BLACK) tes[i].eval *= -1;

        // Now collect an evaluation from a quiet position
        qsearch(thread, &thread->pv, -MATE, MATE, 0);
        for (j = 0; j < thread->pv.length; j++)
            applyMove(&thread->board, thread->pv.line[j], undo);
        T = EmptyTrace;
        evaluateBoard(&thread->board, NULL);

        // Compute phase factors for updating the gradients
        tes[i].factors[MG] = 1 - tes[i].phase / 24.0;
        tes[i].factors[EG] =     tes[i].phase / 24.0;

        // Finish with the usual phase for the evaluation
        tes[i].phase = (tes[i].phase * 256 + 12) / 24.0;

        // Vectorize the evaluation coefficients into coeffs
        initializeCoefficients(coeffs);

        // Determine how many TexelTuples will be needed
        for (k = 0, j = 0; j < NT; j++)
            k += coeffs[j] != 0;

        // Determine if we need to allocate more Texel Tuples
        if (k > TupleStackSize){
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
        for (k = 0, j = 0; j < NT; j++){
            if (coeffs[j] != 0){
                tes[i].tuples[k].index = j;
                tes[i].tuples[k++].coeff = coeffs[j];
            }
        }
    }

    fclose(fin);
}

void initializeCoefficients(int coeffs[NT]){

    int i = 0, a, b, c;

    // Zero out the coefficients since not all parts of the process
    // for vectorization of the evaluation initializes the vector
    memset(coeffs, 0, NT * sizeof(int));

    // Initialize coefficients for the Piece Values. We include
    // the King as well, just to get a nice output to copy paste

    if (TunePawnValue)
        coeffs[i++] = T.pawnCounts[WHITE] - T.pawnCounts[BLACK];

    if (TuneKnightValue)
        coeffs[i++] = T.knightCounts[WHITE] - T.knightCounts[BLACK];

    if (TuneBishopValue)
        coeffs[i++] = T.bishopCounts[WHITE] - T.bishopCounts[BLACK];

    if (TuneRookValue)
        coeffs[i++] = T.rookCounts[WHITE] - T.rookCounts[BLACK];

    if (TuneQueenValue)
        coeffs[i++] = T.queenCounts[WHITE] - T.queenCounts[BLACK];

    if (TuneKingValue)
        coeffs[i++] = 0; // Both sides always have a King ...

    // Initialize coefficients for the Piece Square Tables

    if (TunePawnPSQT){
        for (a = 0; a < 64; a++){
            coeffs[i + relativeSquare32(a, WHITE)] += T.pawnPSQT[WHITE][a];
            coeffs[i + relativeSquare32(a, BLACK)] -= T.pawnPSQT[BLACK][a];
        } i += 32;
    }

    if (TuneKnightPSQT){
        for (a = 0; a < 64; a++){
            coeffs[i + relativeSquare32(a, WHITE)] += T.knightPSQT[WHITE][a];
            coeffs[i + relativeSquare32(a, BLACK)] -= T.knightPSQT[BLACK][a];
        } i += 32;
    }

    if (TuneBishopPSQT){
        for (a = 0; a < 64; a++){
            coeffs[i + relativeSquare32(a, WHITE)] += T.bishopPSQT[WHITE][a];
            coeffs[i + relativeSquare32(a, BLACK)] -= T.bishopPSQT[BLACK][a];
        } i += 32;
    }

    if (TuneRookPSQT){
        for (a = 0; a < 64; a++){
            coeffs[i + relativeSquare32(a, WHITE)] += T.rookPSQT[WHITE][a];
            coeffs[i + relativeSquare32(a, BLACK)] -= T.rookPSQT[BLACK][a];
        } i += 32;
    }

    if (TuneQueenPSQT){
        for (a = 0; a < 64; a++){
            coeffs[i + relativeSquare32(a, WHITE)] += T.queenPSQT[WHITE][a];
            coeffs[i + relativeSquare32(a, BLACK)] -= T.queenPSQT[BLACK][a];
        } i += 32;
    }

    if (TuneKingPSQT){
        for (a = 0; a < 64; a++){
            coeffs[i + relativeSquare32(a, WHITE)] += T.kingPSQT[WHITE][a];
            coeffs[i + relativeSquare32(a, BLACK)] -= T.kingPSQT[BLACK][a];
        } i += 32;
    }

    // Initialize coefficients for Pawn evaluation terms

    if (TunePawnIsolated)
        coeffs[i++] = T.pawnIsolated[WHITE] - T.pawnIsolated[BLACK];

    if (TunePawnStacked)
        coeffs[i++] = T.pawnStacked[WHITE] - T.pawnStacked[BLACK];

    if (TunePawnBackwards)
        for (a = 0; a < 2; a++)
            coeffs[i++] = T.pawnBackwards[WHITE][a] - T.pawnBackwards[BLACK][a];

    if (TunePawnConnected){
        for (a = 0; a < 64; a++){
            coeffs[i + relativeSquare32(a, WHITE)] += T.pawnConnected[WHITE][a];
            coeffs[i + relativeSquare32(a, BLACK)] -= T.pawnConnected[BLACK][a];
        } i += 32;
    }


    // Initialize coefficients for the Knight evaluation terms

    if (TuneKnightRammedPawns)
        coeffs[i++] = T.knightRammedPawns[WHITE] - T.knightRammedPawns[BLACK];

    if (TuneKnightOutpost)
        for (a = 0; a < 2; a++)
            coeffs[i++] = T.knightOutpost[WHITE][a] - T.knightOutpost[BLACK][a];

    if (TuneKnightMobility)
        for (a = 0; a < 9; a++)
            coeffs[i++] = T.knightMobility[WHITE][a] - T.knightMobility[BLACK][a];


    // Initialize coefficients for the Bishop evaluation terms

    if (TuneBishopPair)
        coeffs[i++] = T.bishopPair[WHITE] - T.bishopPair[BLACK];

    if (TuneBishopRammedPawns)
        coeffs[i++] = T.bishopRammedPawns[WHITE] - T.bishopRammedPawns[BLACK];

    if (TuneBishopOutpost)
        for (a = 0; a < 2; a++)
            coeffs[i++] = T.bishopOutpost[WHITE][a] - T.bishopOutpost[BLACK][a];

    if (TuneBishopMobility)
        for (a = 0; a < 14; a++)
            coeffs[i++] = T.bishopMobility[WHITE][a] - T.bishopMobility[BLACK][a];


    // Initialize coefficients for the Rook evaluation terms

    if (TuneRookFile)
        for (a = 0; a < 2; a++)
            coeffs[i++] = T.rookFile[WHITE][a] - T.rookFile[BLACK][a];

    if (TuneRookOnSeventh)
        coeffs[i++] = T.rookOnSeventh[WHITE] - T.rookOnSeventh[BLACK];

    if (TuneRookMobility)
        for (a = 0; a < 15; a++)
            coeffs[i++] = T.rookMobility[WHITE][a] - T.rookMobility[BLACK][a];


    // Initialize coefficients for the Queen evaluation terms

    if (TuneQueenMobility)
        for (a = 0; a < 28; a++)
            coeffs[i++] = T.queenMobility[WHITE][a] - T.queenMobility[BLACK][a];


    // Intitialize coefficients for the King evaluation terms

    if (TuneKingDefenders)
        for (a = 0; a < 12; a++)
            coeffs[i++] = T.kingDefenders[WHITE][a] - T.kingDefenders[BLACK][a];

    if (TuneKingShelter)
        for (a = 0; a < 2; a++)
            for (b = 0; b < FILE_NB; b++)
                for (c = 0; c < RANK_NB; c++)
                    coeffs[i++] = T.kingShelter[WHITE][a][b][c] - T.kingShelter[BLACK][a][b][c];


    // Initialize coefficients for the Passed Pawn evaluation terms

    if (TunePassedPawn)
        for (a = 0; a < 2; a++)
            for (b = 0; b < 2; b++)
                for (c = 0; c < RANK_NB; c++)
                    coeffs[i++] = T.passedPawn[WHITE][a][b][c] - T.passedPawn[BLACK][a][b][c];


    // Initialize coefficients for the Threat evaluation terms

    if (TuneThreatPawnAttackedByOne)
        coeffs[i++] = T.threatPawnAttackedByOne[WHITE] - T.threatPawnAttackedByOne[BLACK];

    if (TuneThreatMinorAttackedByPawn)
        coeffs[i++] = T.threatMinorAttackedByPawn[WHITE] - T.threatMinorAttackedByPawn[BLACK];

    if (TuneThreatMinorAttackedByMajor)
        coeffs[i++] = T.threatMinorAttackedByMajor[WHITE] - T.threatMinorAttackedByMajor[BLACK];

    if (TuneThreatQueenAttackedByMinor)
        coeffs[i++] = T.threatQueenAttackedByMinor[WHITE] - T.threatQueenAttackedByMinor[BLACK];

    if (TuneThreatQueenAttackedByOne)
        coeffs[i++] = T.threatQueenAttackedByOne[WHITE] - T.threatQueenAttackedByOne[BLACK];
}

void initializeCurrentParameters(double cparams[NT][PHASE_NB]){

    int i = 0, a, b, c;

    // Grab the current parameters for each Piece Value

    if (TunePawnValue){
        cparams[i  ][MG] = ScoreMG(PawnValue);
        cparams[i++][EG] = ScoreEG(PawnValue);
    }

    if (TuneKnightValue){
        cparams[i  ][MG] = ScoreMG(KnightValue);
        cparams[i++][EG] = ScoreEG(KnightValue);
    }

    if (TuneBishopValue){
        cparams[i  ][MG] = ScoreMG(BishopValue);
        cparams[i++][EG] = ScoreEG(BishopValue);
    }

    if (TuneRookValue){
        cparams[i  ][MG] = ScoreMG(RookValue);
        cparams[i++][EG] = ScoreEG(RookValue);
    }

    if (TuneQueenValue){
        cparams[i  ][MG] = ScoreMG(QueenValue);
        cparams[i++][EG] = ScoreEG(QueenValue);
    }

    if (TuneKingValue){
        cparams[i  ][MG] = ScoreMG(KingValue);
        cparams[i++][EG] = ScoreEG(KingValue);
    }


    // Grab the current parameters for each Piece Square Table Value

    if (TunePawnPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = ScoreMG(PawnPSQT32[a]);
            cparams[i][EG] = ScoreEG(PawnPSQT32[a]);
        }
    }

    if (TuneKnightPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = ScoreMG(KnightPSQT32[a]);
            cparams[i][EG] = ScoreEG(KnightPSQT32[a]);
        }
    }

    if (TuneBishopPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = ScoreMG(BishopPSQT32[a]);
            cparams[i][EG] = ScoreEG(BishopPSQT32[a]);
        }
    }

    if (TuneRookPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = ScoreMG(RookPSQT32[a]);
            cparams[i][EG] = ScoreEG(RookPSQT32[a]);
        }
    }

    if (TuneQueenPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = ScoreMG(QueenPSQT32[a]);
            cparams[i][EG] = ScoreEG(QueenPSQT32[a]);
        }
    }

    if (TuneKingPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = ScoreMG(KingPSQT32[a]);
            cparams[i][EG] = ScoreEG(KingPSQT32[a]);
        }
    }


    // Grab the current parameters for the Pawn evaluation terms

    if (TunePawnIsolated){
        cparams[i  ][MG] = ScoreMG(PawnIsolated);
        cparams[i++][EG] = ScoreEG(PawnIsolated);
    }

    if (TunePawnStacked){
        cparams[i  ][MG] = ScoreMG(PawnStacked);
        cparams[i++][EG] = ScoreEG(PawnStacked);
    }

    if (TunePawnBackwards){
        for (a = 0; a < 2; a++, i++){
            cparams[i][MG] = ScoreMG(PawnBackwards[a]);
            cparams[i][EG] = ScoreEG(PawnBackwards[a]);
        }
    }

    if (TunePawnConnected){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = ScoreMG(PawnConnected32[a]);
            cparams[i][EG] = ScoreEG(PawnConnected32[a]);
        }
    }


    // Grab the current parameters for the Knight evaluation terms

    if (TuneKnightRammedPawns){
        cparams[i  ][MG] = ScoreMG(KnightRammedPawns);
        cparams[i++][EG] = ScoreEG(KnightRammedPawns);
    }

    if (TuneKnightOutpost){
        for (a = 0; a < 2; a++, i++){
            cparams[i][MG] = ScoreMG(KnightOutpost[a]);
            cparams[i][EG] = ScoreEG(KnightOutpost[a]);
        }
    }

    if (TuneKnightMobility){
        for (a = 0; a < 9; a++, i++){
            cparams[i][MG] = ScoreMG(KnightMobility[a]);
            cparams[i][EG] = ScoreEG(KnightMobility[a]);
        }
    }


    // Grab the current parameters for the Bishop evaluation terms

    if (TuneBishopPair){
        cparams[i  ][MG] = ScoreMG(BishopPair);
        cparams[i++][EG] = ScoreEG(BishopPair);
    }

    if (TuneBishopRammedPawns){
        cparams[i  ][MG] = ScoreMG(BishopRammedPawns);
        cparams[i++][EG] = ScoreEG(BishopRammedPawns);
    }

    if (TuneBishopOutpost){
        for (a = 0; a < 2; a++, i++){
            cparams[i][MG] = ScoreMG(BishopOutpost[a]);
            cparams[i][EG] = ScoreEG(BishopOutpost[a]);
        }
    }

    if (TuneKnightMobility){
        for (a = 0; a < 14; a++, i++){
            cparams[i][MG] = ScoreMG(BishopMobility[a]);
            cparams[i][EG] = ScoreEG(BishopMobility[a]);
        }
    }


    // Grab the current parameters for the Rook evaluation terms

    if (TuneRookFile){
        for (a = 0; a < 2; a++, i++){
            cparams[i][MG] = ScoreMG(RookFile[a]);
            cparams[i][EG] = ScoreEG(RookFile[a]);
        }
    }

    if (TuneRookOnSeventh){
        cparams[i  ][MG] = ScoreMG(RookOnSeventh);
        cparams[i++][EG] = ScoreEG(RookOnSeventh);
    }

    if (TuneRookMobility){
        for (a = 0; a < 15; a++, i++){
            cparams[i][MG] = ScoreMG(RookMobility[a]);
            cparams[i][EG] = ScoreEG(RookMobility[a]);
        }
    }


    // Grab the current parameters for the Queen evaluation terms

    if (TuneQueenMobility){
        for (a = 0; a < 28; a++, i++){
            cparams[i][MG] = ScoreMG(QueenMobility[a]);
            cparams[i][EG] = ScoreEG(QueenMobility[a]);
        }
    }


    // Grab the current parameters for the King evaluation terms

    if (TuneKingDefenders){
        for (a = 0; a < 12; a++, i++){
            cparams[i][MG] = ScoreMG(KingDefenders[a]);
            cparams[i][EG] = ScoreEG(KingDefenders[a]);
        }
    }

    if (TuneKingShelter){
        for (a = 0; a < 2; a++){
            for (b = 0; b < FILE_NB; b++){
                for (c = 0; c < RANK_NB; c++, i++){
                    cparams[i][MG] = ScoreMG(KingShelter[a][b][c]);
                    cparams[i][EG] = ScoreEG(KingShelter[a][b][c]);
                }
            }
        }
    }


    // Grab the current parameters for the Passed Pawn evaluation terms

    if (TunePassedPawn){
        for (a = 0; a < 2; a++){
            for (b = 0; b < 2; b++){
                for (c = 0; c < RANK_NB; c++, i++){
                    cparams[i][MG] = ScoreMG(PassedPawn[a][b][c]);
                    cparams[i][EG] = ScoreEG(PassedPawn[a][b][c]);
                }
            }
        }
    }


    // Grab the current parameters for the Threat evaluation terms

    if (TuneThreatPawnAttackedByOne){
        cparams[i  ][MG] = ScoreMG(ThreatPawnAttackedByOne);
        cparams[i++][EG] = ScoreEG(ThreatPawnAttackedByOne);
    }

    if (TuneThreatMinorAttackedByPawn){
        cparams[i  ][MG] = ScoreMG(ThreatMinorAttackedByPawn);
        cparams[i++][EG] = ScoreEG(ThreatMinorAttackedByPawn);
    }

    if (TuneThreatMinorAttackedByMajor){
        cparams[i  ][MG] = ScoreMG(ThreatMinorAttackedByMajor);
        cparams[i++][EG] = ScoreEG(ThreatMinorAttackedByMajor);
    }

    if (TuneThreatQueenAttackedByMinor){
        cparams[i  ][MG] = ScoreMG(ThreatQueenAttackedByMinor);
        cparams[i++][EG] = ScoreEG(ThreatQueenAttackedByMinor);
    }

    if (TuneThreatQueenAttackedByOne){
        cparams[i  ][MG] = ScoreMG(ThreatQueenAttackedByOne);
        cparams[i++][EG] = ScoreEG(ThreatQueenAttackedByOne);
    }
}

void calculateLearningRates(TexelEntry* tes, double rates[NT][PHASE_NB]){

    int i, j, index, coeff;
    double avgByPhase[PHASE_NB] = {0};
    double occurances[NT][PHASE_NB] = {{0}, {0}};

    for (i = 0; i < NP; i++){
        for (j = 0; j < tes[i].ntuples; j++){

            index = tes[i].tuples[j].index;
            coeff = tes[i].tuples[j].coeff;

            occurances[index][MG] += abs(coeff) * tes[i].factors[MG];
            occurances[index][EG] += abs(coeff) * tes[i].factors[EG];

            avgByPhase[MG] += abs(coeff) * tes[i].factors[MG];
            avgByPhase[EG] += abs(coeff) * tes[i].factors[EG];
        }
    }

    avgByPhase[MG] /= NT;
    avgByPhase[EG] /= NT;

    for (i = 0; i < NT; i++){
        if (occurances[i][MG] >= 1.0)
            rates[i][MG] = avgByPhase[MG] / occurances[i][MG];
        if (occurances[i][EG] >= 1.0)
            rates[i][EG] = avgByPhase[EG] / occurances[i][EG];
    }
}

void printParameters(double params[NT][PHASE_NB], double cparams[NT][PHASE_NB]){

    int i = 0, x, y;

    int tparams[NT][PHASE_NB];

    int pvalue = ScoreMG(PawnValue) + (TunePawnValue ? params[0][MG] : 0);

    // Combine the original params and the param deltas. Scale the params so
    // that the mid game value of a pawn is always 100 centipawns
    for (x = 0; x < NT; x++){
        tparams[x][MG] = (int)((100.0 / pvalue) * (params[x][MG] + cparams[x][MG]));
        tparams[x][EG] = (int)((100.0 / pvalue) * (params[x][EG] + cparams[x][EG]));
    }

    // Print Piece Values

    printf("\n\n// Definition of Values for each Piece type\n");

    if (TunePawnValue){
        printf("\nconst int PawnValue   = S(%4d,%4d);", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneKnightValue){
        printf("\nconst int KnightValue = S(%4d,%4d);", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneBishopValue){
        printf("\nconst int BishopValue = S(%4d,%4d);", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneRookValue){
        printf("\nconst int RookValue   = S(%4d,%4d);", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneQueenValue){
        printf("\nconst int QueenValue  = S(%4d,%4d);", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneKingValue){
        printf("\nconst int KingValue   = S(%4d,%4d);", tparams[i][MG], tparams[i][EG]); i++;
    }


    // Print Piece Square Table Values

    if (TunePawnPSQT){
        printf("\nconst int PawnPSQT32[32] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }

    if (TuneKnightPSQT){
        printf("\nconst int KnightPSQT32[32] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }

    if (TuneBishopPSQT){
        printf("\nconst int BishopPSQT32[32] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }

    if (TuneRookPSQT){
        printf("\nconst int RookPSQT32[32] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }

    if (TuneQueenPSQT){
        printf("\nconst int QueenPSQT32[32] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }

    if (TuneKingPSQT){
        printf("\nconst int KingPSQT32[32] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }


    // Print Pawn Values

    printf("\n\n// Definition of evaluation terms related to Pawns\n");

    if (TunePawnIsolated){
        printf("\nconst int PawnIsolated = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TunePawnStacked){
        printf("\nconst int PawnStacked = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TunePawnBackwards){
        printf("\nconst int PawnBackwards[2] = { S(%4d,%4d), S(%4d,%4d) };\n",
                tparams[i  ][MG], tparams[i  ][EG],
                tparams[i+1][MG], tparams[i+1][EG]); i += 2;
    }

    if (TunePawnConnected){
        printf("\nconst int PawnConnected32[32] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }


    // Print Knight Values

    printf("\n\n// Definition of evaluation terms related to Knights\n");

    if (TuneKnightRammedPawns){
        printf("\nconst int KnightRammedPawns = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneKnightOutpost){
        printf("\nconst int KnightOutpost[2] = { S(%4d,%4d), S(%4d,%4d) };\n",
                tparams[i  ][MG], tparams[i  ][EG],
                tparams[i+1][MG], tparams[i+1][EG]); i += 2;
    }

    if (TuneKnightMobility){
        printf("\nconst int KnightMobility[9] = {");
        for (x = 0; x < 3; x++){
            printf("\n   ");
            for (y = 0; y < 3; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }


    // Print Bishop Values

    printf("\n\n// Definition of evaluation terms related to Bishops\n");

    if (TuneBishopPair){
        printf("\nconst int BishopPair = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneBishopRammedPawns){
        printf("\nconst int BishopRammedPawns = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneBishopOutpost){
        printf("\nconst int BishopOutpost[2] = { S(%4d,%4d), S(%4d,%4d) };\n",
                tparams[i  ][MG], tparams[i  ][EG],
                tparams[i+1][MG], tparams[i+1][EG]); i += 2;
    }

    if (TuneBishopMobility){
        printf("\nconst int BishopMobility[14] = {");
        for (x = 0; x < 4; x++){
            printf("\n   ");
            for (y = 0; y < 4 && 4 * x + y < 14; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }


    // Print Rook Values

    printf("\n\n// Definition of evaluation terms related to Rooks\n");

    if (TuneRookFile){
        printf("\nconst int RookFile[2] = { S(%4d,%4d), S(%4d,%4d) };\n",
                tparams[i  ][MG], tparams[i  ][EG],
                tparams[i+1][MG], tparams[i+1][EG]); i += 2;
    }

    if (TuneRookOnSeventh){
        printf("\nconst int RookOnSeventh = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneRookMobility){
        printf("\nconst int RookMobility[15] = {");
        for (x = 0; x < 4; x++){
            printf("\n   ");
            for (y = 0; y < 4 && x * 4 + y < 15; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }


    // Print Queen Values

    printf("\n\n// Definition of evaluation terms related to Queens\n");

    if (TuneQueenMobility){
        printf("\nconst int QueenMobility[28] = {");
        for (x = 0; x < 7; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }


    // Print King Values

    printf("\n\n// Definition of evaluation terms related to Kings\n\n");
    printf("int KingSafety[64]; // Defined by the Polynomial below\n\n");
    printf("const double KingPolynomial[6] = {\n");
    printf("    0.00000011, -0.00009948,  0.00797308,\n");
    printf("    0.03141319,  2.18429452, -3.33669140,\n");
    printf("};");

    if (TuneKingDefenders){
        printf("\nconst int KingDefenders[12] = {");
        for (x = 0; x < 3; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" S(%4d,%4d),", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }

    if (TuneKingShelter){
        printf("\nconst int KingShelter[2][FILE_NB][RANK_NB] = {");
        for (x = 0; x < 16; x++){
            printf("\n  %s", x % 8 ? " {" : "{{");
            for (y = 0; y < RANK_NB; y++, i++){
                printf("S(%4d,%4d)", tparams[i][MG], tparams[i][EG]);
                printf("%s", y < RANK_NB - 1 ? ", " : x % 8 == 7 ? "}}," : "},");
            }
        } printf("\n};\n");
    }


    // Print Passed Pawn Values

    printf("\n\n// Definition of evaluation terms related to Passed Pawns\n");

    if (TunePassedPawn){
        printf("\nconst int PassedPawn[2][2][RANK_NB] = {");
        for (x = 0; x < 4; x++){
            printf("\n  %s", x % 2 ? " {" : "{{");
            for (y = 0; y < RANK_NB; y++, i++){
                printf("S(%4d,%4d)", tparams[i][MG], tparams[i][EG]);
                printf("%s", y < RANK_NB - 1 ? ", " : x % 2 ? "}}," : "},");
            }
        } printf("\n};\n");
    }


    // Print Threat Values

    printf("\n\n// Definition of evaluation terms related to Threats\n");

    if (TuneThreatPawnAttackedByOne){
        printf("\nconst int ThreatPawnAttackedByOne    = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneThreatMinorAttackedByPawn){
        printf("\nconst int ThreatMinorAttackedByPawn  = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneThreatMinorAttackedByMajor){
        printf("\nconst int ThreatMinorAttackedByMajor = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneThreatQueenAttackedByMinor){
        printf("\nconst int ThreatQueenAttackedByMinor = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    if (TuneThreatQueenAttackedByOne){
        printf("\nconst int ThreatQueenAttackedByOne   = S(%4d,%4d);\n", tparams[i][MG], tparams[i][EG]); i++;
    }

    // Print any remaining General Evaluation values

    printf("\n\n// Definition of evaluation terms related to general properties\n");

    printf("\nconst int Tempo[COLOUR_NB] = { S(  25,  12), S( -25, -12) };");
}

double computeOptimalK(TexelEntry* tes){

    int i;
    double start = -10.0, end = 10.0, delta = 1.0;
    double curr = start, thisError, bestError = completeEvaluationError(tes, start);

    for (i = 0; i < 10; i++){
        printf("Computing K Iteration [%d] ", i);

        // Find the best value if K within the range [start, end],
        // with a step size based on the current iteration
        curr = start - delta;
        while (curr < end){
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

double completeEvaluationError(TexelEntry* tes, double K){

    int i;
    double total = 0.0;

    // Determine the error margin using the evaluation from evaluateBoard

    #pragma omp parallel shared(total)
    {
        #pragma omp for schedule(static, NP / 48) reduction(+:total)
        for (i = 0; i < NP; i++)
            total += pow(tes[i].result - sigmoid(K, tes[i].eval), 2);
    }

    return total / (double)NP;
}

double completeLinearError(TexelEntry* tes, double params[NT][PHASE_NB], double K){

    int i;
    double total = 0.0;

    // Determine the error margin using evaluation from summing up PARAMS

    #pragma omp parallel shared(total)
    {
        #pragma omp for schedule(static, NP / 48) reduction(+:total)
        for (i = 0; i < NP; i++)
            total += pow(tes[i].result - sigmoid(K, linearEvaluation(tes[i], params)), 2);
    }

    return total / (double)NP;
}

double singleLinearError(TexelEntry te, double params[NT][PHASE_NB], double K){
    return te.result - sigmoid(K, linearEvaluation(te, params));
}

double linearEvaluation(TexelEntry te, double params[NT][PHASE_NB]){

    int i;
    double mg = 0, eg = 0;

    for (i = 0; i < te.ntuples; i++){
        mg += te.tuples[i].coeff * params[te.tuples[i].index][MG];
        eg += te.tuples[i].coeff * params[te.tuples[i].index][EG];
    }

    return te.eval + ((mg * (256 - te.phase) + eg * te.phase) / 256.0);
}

double sigmoid(double K, double S){
    return 1.0 / (1.0 + pow(10.0, -K * S / 400.0));
}

#endif
