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

#include "bitutils.h"
#include "board.h"
#include "evaluate.h"
#include "history.h"
#include "move.h"
#include "search.h"
#include "square.h"
#include "thread.h"
#include "texel.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"

// Our own memory managment system, so that that all of the texel
// tuple structs are, for the most part, storied sequentially in memory
TexelTuple* TupleStack;
int TupleStackSize = STACKSIZE;

// Hack so we can lower the table size for speed
extern TransTable Table;

// Need these in order to get the coefficients
// for each of the evaluation terms
extern EvalTrace T, EmptyTrace;

// To determine the starting values for the Piece Values
extern const int PawnValue[PHASE_NB];
extern const int KnightValue[PHASE_NB];
extern const int BishopValue[PHASE_NB];
extern const int RookValue[PHASE_NB];
extern const int QueenValue[PHASE_NB];
extern const int KingValue[PHASE_NB];

// To determine the starting values for Piece Square Table Values
extern const int PawnPSQT32[32][PHASE_NB];
extern const int KnightPSQT32[32][PHASE_NB];
extern const int BishopPSQT32[32][PHASE_NB];
extern const int RookPSQT32[32][PHASE_NB];
extern const int QueenPSQT32[32][PHASE_NB];
extern const int KingPSQT32[32][PHASE_NB];

// To determine the starting values for the Pawn terms
extern const int PawnIsolated[PHASE_NB];
extern const int PawnStacked[PHASE_NB];
extern const int PawnBackwards[2][PHASE_NB];
extern const int PawnConnected32[32][PHASE_NB];

// To determine the starting values for the Knight terms
extern const int KnightAttackedByPawn[PHASE_NB];
extern const int KnightRammedPawns[PHASE_NB];
extern const int KnightOutpost[2][PHASE_NB];
extern const int KnightMobility[9][PHASE_NB];

// To determine the starting values for the Bishop terms
extern const int BishopPair[PHASE_NB];
extern const int BishopAttackedByPawn[PHASE_NB];
extern const int BishopRammedPawns[PHASE_NB];
extern const int BishopOutpost[2][PHASE_NB];
extern const int BishopMobility[14][PHASE_NB];

// To determine the starting values for the Rook terms
extern const int RookFile[2][PHASE_NB];
extern const int RookOnSeventh[PHASE_NB];
extern const int RookMobility[15][PHASE_NB];

// To determine the starting values for the Queen terms
extern const int QueenChecked[PHASE_NB];
extern const int QueenCheckedByPawn[PHASE_NB];
extern const int QueenMobility[28][PHASE_NB];

// To determine the starting values for the King terms
extern const int KingDefenders[12][PHASE_NB];
extern const int KingShelter[2][FILE_NB][RANK_NB][PHASE_NB];

// To determine the starting values for the Passed Pawn terms
extern const int PassedPawn[2][2][RANK_NB][PHASE_NB];


void runTexelTuning(Thread* thread){
    
    TexelEntry* tes;
    int i, j, iteration = -1;
    double K, thisError, bestError = 1e6, baseRate = 10.0;
    double rates[NT][PHASE_NB] = {{0}, {0}};
    double params[NT][PHASE_NB] = {{0}, {0}};
    double cparams[NT][PHASE_NB] = {{0}, {0}};
    
    setvbuf(stdout, NULL, _IONBF, 0);
    
    printf("\nSetting Transposition Table to 1MB...");
    initializeTranspositionTable(&Table, 1);
    
    printf("\n\nTuner Will Be Tuning %d Terms...", NT);
    
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
    EvalInfo ei;
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
    thread->abort  = 0;
    
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
        initializeBoard(&thread->board, line);
        
        // Determine the game phase based on remaining material
        tes[i].phase = 24 - 4 * popcount(thread->board.pieces[QUEEN ])
                          - 2 * popcount(thread->board.pieces[ROOK  ])
                          - 1 * popcount(thread->board.pieces[KNIGHT])
                          - 1 * popcount(thread->board.pieces[BISHOP]);
        
        
        // Use the search value as the evaluation, to provide a better
        // understanding the potential of a position's eval terms. Make
        // sure the evaluation is from the perspective of WHITE
        tes[i].eval = search(thread, &thread->pv, -MATE, MATE, 1, 0);
        if (thread->board.turn == BLACK) tes[i].eval *= -1;
        
        // Now collect an evaluation from a quiet position
        qsearch(thread, &thread->pv, -MATE, MATE, 0);
        for (j = 0; j < thread->pv.length; j++)
            applyMove(&thread->board, thread->pv.line[j], undo);
        T = EmptyTrace;
        evaluateBoard(&thread->board, &ei, NULL);
                          
        // When updating gradients, we use the coefficients for each
        // term, as well as the phase of the position it came from
        tes[i].factors[MG] = 1 - tes[i].phase / 24.0;
        tes[i].factors[EG] =     tes[i].phase / 24.0;
        
        // Finish determining the phase
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
    
    if (TuneKnightAttackedByPawn)
        coeffs[i++] = T.knightAttackedByPawn[WHITE] - T.knightAttackedByPawn[BLACK];
    
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
    
    if (TuneBishopAttackedByPawn)
        coeffs[i++] = T.bishopAttackedByPawn[WHITE] - T.bishopAttackedByPawn[BLACK];
    
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
    
    if (TuneQueenChecked)
        coeffs[i++] = T.queenChecked[WHITE] - T.queenChecked[BLACK];
    
    if (TuneQueenCheckedByPawn)
        coeffs[i++] = T.queenCheckedByPawn[WHITE] - T.queenCheckedByPawn[BLACK];

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
}

void initializeCurrentParameters(double cparams[NT][PHASE_NB]){
    
    int i = 0, a, b, c;
    
    // Grab the current parameters for each Piece Value
    
    if (TunePawnValue){
        cparams[i  ][MG] = PawnValue[MG];
        cparams[i++][EG] = PawnValue[EG];
    }
    
    if (TuneKnightValue){
        cparams[i  ][MG] = KnightValue[MG];
        cparams[i++][EG] = KnightValue[EG];
    }
    
    if (TuneBishopValue){
        cparams[i  ][MG] = BishopValue[MG];
        cparams[i++][EG] = BishopValue[EG];
    }
    
    if (TuneRookValue){
        cparams[i  ][MG] = RookValue[MG];
        cparams[i++][EG] = RookValue[EG];
    }
    
    if (TuneQueenValue){
        cparams[i  ][MG] = QueenValue[MG];
        cparams[i++][EG] = QueenValue[EG];
    }
    
    if (TuneKingValue){
        cparams[i  ][MG] = KingValue[MG];
        cparams[i++][EG] = KingValue[EG];
    }
    
    
    // Grab the current parameters for each Piece Square Table Value

    if (TunePawnPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = PawnPSQT32[a][MG];
            cparams[i][EG] = PawnPSQT32[a][EG];
        }
    }
    
    if (TuneKnightPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = KnightPSQT32[a][MG];
            cparams[i][EG] = KnightPSQT32[a][EG];
        }
    }
    
    if (TuneBishopPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = BishopPSQT32[a][MG];
            cparams[i][EG] = BishopPSQT32[a][EG];
        }
    }
        
    if (TuneRookPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = RookPSQT32[a][MG];
            cparams[i][EG] = RookPSQT32[a][EG];
        }
    }
        
    if (TuneQueenPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = QueenPSQT32[a][MG];
            cparams[i][EG] = QueenPSQT32[a][EG];
        }
    }
    
    if (TuneKingPSQT){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = KingPSQT32[a][MG];
            cparams[i][EG] = KingPSQT32[a][EG];
        }
    }
    
    
    // Grab the current parameters for the Pawn evaluation terms
    
    if (TunePawnIsolated){
        cparams[i  ][MG] = PawnIsolated[MG];
        cparams[i++][EG] = PawnIsolated[EG];
    }
    
    if (TunePawnStacked){
        cparams[i  ][MG] = PawnStacked[MG];
        cparams[i++][EG] = PawnStacked[EG];
    }
    
    if (TunePawnBackwards){
        for (a = 0; a < 2; a++, i++){
            cparams[i][MG] = PawnBackwards[a][MG];
            cparams[i][EG] = PawnBackwards[a][EG];
        }    
    }
    
    if (TunePawnConnected){
        for (a = 0; a < 32; a++, i++){
            cparams[i][MG] = PawnConnected32[a][MG];
            cparams[i][EG] = PawnConnected32[a][EG];
        }
    }
    
    
    // Grab the current parameters for the Knight evaluation terms
    
    if (TuneKnightAttackedByPawn){
        cparams[i  ][MG] = KnightAttackedByPawn[MG];
        cparams[i++][EG] = KnightAttackedByPawn[EG];
    }
    
    if (TuneKnightRammedPawns){
        cparams[i  ][MG] = KnightRammedPawns[MG];
        cparams[i++][EG] = KnightRammedPawns[EG];
    }
    
    if (TuneKnightOutpost){
        for (a = 0; a < 2; a++, i++){
            cparams[i][MG] = KnightOutpost[a][MG];
            cparams[i][EG] = KnightOutpost[a][EG];
        }
    }
    
    if (TuneKnightMobility){
        for (a = 0; a < 9; a++, i++){
            cparams[i][MG] = KnightMobility[a][MG];
            cparams[i][EG] = KnightMobility[a][EG];
        }
    }
    
    
    // Grab the current parameters for the Bishop evaluation terms
    
    if (TuneBishopPair){
        cparams[i  ][MG] = BishopPair[MG];
        cparams[i++][EG] = BishopPair[EG];
    }
    
    if (TuneBishopRammedPawns){
        cparams[i  ][MG] = BishopRammedPawns[MG];
        cparams[i++][EG] = BishopRammedPawns[EG];
    }
    
    if (TuneBishopAttackedByPawn){
        cparams[i  ][MG] = BishopAttackedByPawn[MG];
        cparams[i++][EG] = BishopAttackedByPawn[EG];
    }
    
    if (TuneBishopOutpost){
        for (a = 0; a < 2; a++, i++){
            cparams[i][MG] = BishopOutpost[a][MG];
            cparams[i][EG] = BishopOutpost[a][EG];
        }
    }
    
    if (TuneKnightMobility){
        for (a = 0; a < 14; a++, i++){
            cparams[i][MG] = BishopMobility[a][MG];
            cparams[i][EG] = BishopMobility[a][EG];
        }
    }
    
    
    // Grab the current parameters for the Rook evaluation terms
    
    if (TuneRookFile){
        for (a = 0; a < 2; a++, i++){
            cparams[i][MG] = RookFile[a][MG];
            cparams[i][EG] = RookFile[a][EG];
        }
    }
    
    if (TuneRookOnSeventh){
        cparams[i  ][MG] = RookOnSeventh[MG];
        cparams[i++][EG] = RookOnSeventh[EG];
    }
    
    if (TuneRookMobility){
        for (a = 0; a < 15; a++, i++){
            cparams[i][MG] = RookMobility[a][MG];
            cparams[i][EG] = RookMobility[a][EG];
        }
    }
    
    
    // Grab the current parameters for the Queen evaluation terms
    
    if (TuneQueenChecked){
        cparams[i  ][MG] = QueenChecked[MG];
        cparams[i++][EG] = QueenChecked[EG];
    }
    
    if (TuneQueenCheckedByPawn){
        cparams[i  ][MG] = QueenCheckedByPawn[MG];
        cparams[i++][EG] = QueenCheckedByPawn[EG];
    }
    
    if (TuneQueenMobility){
        for (a = 0; a < 28; a++, i++){
            cparams[i][MG] = QueenMobility[a][MG];
            cparams[i][EG] = QueenMobility[a][EG];
        }
    }

    
    // Grab the current parameters for the King evaluation terms

    if (TuneKingDefenders){
        for (a = 0; a < 12; a++, i++){
            cparams[i][MG] = KingDefenders[a][MG];
            cparams[i][EG] = KingDefenders[a][EG];
        }
    }
    
    if (TuneKingShelter){
        for (a = 0; a < 2; a++){
            for (b = 0; b < FILE_NB; b++){
                for (c = 0; c < RANK_NB; c++, i++){
                    cparams[i][MG] = KingShelter[a][b][c][MG];
                    cparams[i][EG] = KingShelter[a][b][c][EG];
                }
            }
        }
    }
    
    
    // Grab the current parameters for the Passed Pawn evaluation terms
    
    if (TunePassedPawn){
        for (a = 0; a < 2; a++){
            for (b = 0; b < 2; b++){
                for (c = 0; c < RANK_NB; c++, i++){
                    cparams[i][MG] = PassedPawn[a][b][c][MG];
                    cparams[i][EG] = PassedPawn[a][b][c][EG];
                }
            }
        }
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
    
    int pvalue = PawnValue[MG] + (TunePawnValue ? params[0][MG] : 0);
    
    // Combine the original params and the param deltas. Scale the params so
    // that the mid game value of a pawn is always 100 centipawns
    for (x = 0; x < NT; x++){
        tparams[x][MG] = (int)((100.0 / pvalue) * (params[x][MG] + cparams[x][MG]));
        tparams[x][EG] = (int)((100.0 / pvalue) * (params[x][EG] + cparams[x][EG]));
    }
    
    // Print Piece Values
    
    if (TunePawnValue){
        printf("\nconst int PawnValue[PHASE_NB]   = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneKnightValue){
        printf("\nconst int KnightValue[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneBishopValue){
        printf("\nconst int BishopValue[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneRookValue){
        printf("\nconst int RookValue[PHASE_NB]   = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneQueenValue){
        printf("\nconst int QueenValue[PHASE_NB]  = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneKingValue){
        printf("\nconst int KingValue[PHASE_NB]   = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    
    // Print Piece Square Table Values
    
    if (TunePawnPSQT){
        printf("\nconst int PawnPSQT32[32][PHASE_NB] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    if (TuneKnightPSQT){
        printf("\nconst int KnightPSQT32[32][PHASE_NB] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    if (TuneBishopPSQT){
        printf("\nconst int BishopPSQT32[32][PHASE_NB] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    if (TuneRookPSQT){
        printf("\nconst int RookPSQT32[32][PHASE_NB] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    if (TuneQueenPSQT){
        printf("\nconst int QueenPSQT32[32][PHASE_NB] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    if (TuneKingPSQT){
        printf("\nconst int KingPSQT32[32][PHASE_NB] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    
    // Print Pawn Values
    
    if (TunePawnIsolated){
        printf("\nconst int PawnIsolated[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TunePawnStacked){
        printf("\nconst int PawnStacked[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TunePawnBackwards){
        printf("\nconst int PawnBackwards[2][PHASE_NB] = { {%4d,%4d}, {%4d,%4d} };\n",
                tparams[i  ][MG], tparams[i  ][EG],
                tparams[i+1][MG], tparams[i+1][EG]); i += 2;
    }
    
    if (TunePawnConnected){
        printf("\nconst int PawnConnected32[32][PHASE_NB] = {");
        for (x = 0; x < 8; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    
    // Print Knight Values
    
    if (TuneKnightAttackedByPawn){
        printf("\nconst int KnightAttackedByPawn[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneKnightRammedPawns){
        printf("\nconst int KnightRammedPawns[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneKnightOutpost){
        printf("\nconst int KnightOutpost[2][PHASE_NB] = { {%4d,%4d}, {%4d,%4d} };\n",
                tparams[i  ][MG], tparams[i  ][EG],
                tparams[i+1][MG], tparams[i+1][EG]); i += 2;
    }
            
    if (TuneKnightMobility){
        printf("\nconst int KnightMobility[9][PHASE_NB] = {");
        for (x = 0; x < 3; x++){
            printf("\n   ");
            for (y = 0; y < 3; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    
    // Print Bishop Values
    
    if (TuneBishopPair){
        printf("\nconst int BishopPair[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneBishopRammedPawns){
        printf("\nconst int BishopRammedPawns[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneBishopAttackedByPawn){
        printf("\nconst int BishopAttackedByPawn[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneBishopOutpost){
        printf("\nconst int BishopOutpost[2][PHASE_NB] = { {%4d,%4d}, {%4d,%4d} };\n",
                tparams[i  ][MG], tparams[i  ][EG],
                tparams[i+1][MG], tparams[i+1][EG]); i += 2;
    }
    
    if (TuneBishopMobility){
        printf("\nconst int BishopMobility[14][PHASE_NB] = {");
        for (x = 0; x < 4; x++){
            printf("\n   ");
            for (y = 0; y < 4 && 4 * x + y < 14; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    
    // Print Rook Values
    
    if (TuneRookFile){
        printf("\nconst int RookFile[2][PHASE_NB] = { {%4d,%4d}, {%4d,%4d} };\n",
                tparams[i  ][MG], tparams[i  ][EG],
                tparams[i+1][MG], tparams[i+1][EG]); i += 2;
    }
            
    if (TuneRookOnSeventh){
        printf("\nconst int RookOnSeventh[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
        
    if (TuneRookMobility){
        printf("\nconst int RookMobility[15][PHASE_NB] = {");
        for (x = 0; x < 4; x++){
            printf("\n   ");
            for (y = 0; y < 4 && x * 4 + y < 15; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    
    // Print Queen Values
    
    if (TuneQueenChecked){
        printf("\nconst int QueenChecked[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
    
    if (TuneQueenCheckedByPawn){
        printf("\nconst int QueenCheckedByPawn[PHASE_NB] = {%4d,%4d};\n", tparams[i][MG], tparams[i][EG]); i++;
    }
        
    if (TuneQueenMobility){
        printf("\nconst int QueenMobility[28][PHASE_NB] = {");
        for (x = 0; x < 7; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    
    // Print King Values
    
    if (TuneKingDefenders){
        printf("\nconst int KingDefenders[12][PHASE_NB] = {");
        for (x = 0; x < 3; x++){
            printf("\n   ");
            for (y = 0; y < 4; y++, i++)
                printf(" {%4d,%4d},", tparams[i][MG], tparams[i][EG]);
        } printf("\n};\n");
    }
    
    if (TuneKingShelter){
        printf("\nconst int KingShelter[2][FILE_NB][RANK_NB][PHASE_NB] = {");
        for (x = 0; x < 16; x++){
            printf("\n  %s", x % 8 ? " {" : "{{");
            for (y = 0; y < RANK_NB; y++, i++){
                printf("{%4d,%4d}", tparams[i][MG], tparams[i][EG]);
                printf("%s", y < RANK_NB - 1 ? ", " : x % 8 == 7 ? "}}," : "},");
            }
        } printf("\n};\n");
    }
    
    
    // Print Passed Pawn Values
    
    if (TunePassedPawn){
        printf("\nconst int PassedPawn[2][2][RANK_NB][PHASE_NB] = {");
        for (x = 0; x < 4; x++){
            printf("\n  %s", x % 2 ? " {" : "{{");
            for (y = 0; y < RANK_NB; y++, i++){
                printf("{%4d,%4d}", tparams[i][MG], tparams[i][EG]);
                printf("%s", y < RANK_NB - 1 ? ", " : x % 2 ? "}}," : "},");
            }
        } printf("\n};\n");
    }
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
