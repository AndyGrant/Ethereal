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

// Hack so we can lower the table size for speed
extern TransTable Table;

// Need these in order to get the coefficients
// for each of the evaluation terms
extern EvalTrace T, EmptyTrace;

// To determine the starting values for the Pawn terms
extern const int PawnValue[PHASE_NB];
extern const int PawnPSQT32[32][PHASE_NB];
extern const int PawnIsolated[PHASE_NB];
extern const int PawnStacked[PHASE_NB];
extern const int PawnBackwards[2][PHASE_NB];
extern const int PawnConnected32[32][PHASE_NB];

// To determine the starting values for the Knight terms
extern const int KnightValue[PHASE_NB];
extern const int KnightPSQT32[32][PHASE_NB];
extern const int KnightAttackedByPawn[PHASE_NB];
extern const int KnightOutpost[2][PHASE_NB];
extern const int KnightMobility[9][PHASE_NB];

// To determine the starting values for the Bishop terms
extern const int BishopValue[PHASE_NB];
extern const int BishopPSQT32[32][PHASE_NB];
extern const int BishopWings[PHASE_NB];
extern const int BishopPair[PHASE_NB];
extern const int BishopAttackedByPawn[PHASE_NB];
extern const int BishopOutpost[2][PHASE_NB];
extern const int BishopMobility[14][PHASE_NB];

// To determine the starting values for the Rook terms
extern const int RookValue[PHASE_NB];
extern const int RookPSQT32[32][PHASE_NB];
extern const int RookFile[2][PHASE_NB];
extern const int RookOnSeventh[PHASE_NB];
extern const int RookMobility[15][PHASE_NB];

// To determine the starting values for the Queen terms
extern const int QueenValue[PHASE_NB];
extern const int QueenChecked[PHASE_NB];
extern const int QueenCheckedByPawn[PHASE_NB];
extern const int QueenPSQT32[32][PHASE_NB];
extern const int QueenMobility[28][PHASE_NB];

// To determine the starting values for the King terms
extern const int KingPSQT32[32][PHASE_NB];
extern const int KingDefenders[12][PHASE_NB];
extern const int KingShelter[2][FILE_NB][RANK_NB][PHASE_NB];

// To determine the starting values for the Passed Pawn terms
extern const int PassedPawn[2][2][RANK_NB][PHASE_NB];


void runTexelTuning(Thread* thread){
    
    TexelEntry* tes;
    int i, j, iteration = -1;
    double K, thisError, baseRate = 10.0;
    double rates[NT][PHASE_NB] = {{0}, {0}};
    double params[NT][PHASE_NB] = {{0}, {0}};
    double cparams[NT][PHASE_NB] = {{0}, {0}};
    
    setvbuf(stdout, NULL, _IONBF, 0);
    
    printf("\nSetting Transposition Table to 1MB...");
    initializeTranspositionTable(&Table, 1);
    
    printf("\n\nAllocating Memory for Texel Tuner [%dMB]...",
           (int)(NP * sizeof(TexelEntry) / (1024 * 1024)));
    tes = calloc(NP, sizeof(TexelEntry));
    
    printf("\n\nReading and Initializing Texel Entries from FENS...");
    initializeTexelEntries(tes, thread);
    
    printf("\n\nFetching Current Evaluation Terms as a Starting Point...");
    initializeCurrentParameters(cparams);
    
    printf("\n\nScaling Params For Phases and Occurance Rates...");
    calculateLearningRates(tes, rates);
    
    printf("\n\nComputing Optimal K Value...");
    K = computeOptimalK(tes);
    
    while (1){
        
        iteration++;
        
        if (iteration % 100 == 0){
            printParameters(params, cparams);
            printf("\nIteration [%d] Error = %g \n", iteration, completeLinearError(tes, params, K));
        }
                
        double gradients[NT][PHASE_NB] = {{0}, {0}};
        #pragma omp parallel shared(gradients)
        {
            double localgradients[NT][PHASE_NB] = {{0}, {0}};
            #pragma omp for schedule(static, NP / 48)
            for (i = 0; i < NP; i++){
                thisError = singleLinearError(tes[i], params, K);
                
                for (j = 0; j < NT; j++){
                    localgradients[j][MG] += thisError * tes[i].coeffs[j] * tes[i].factors[MG];
                    localgradients[j][EG] += thisError * tes[i].coeffs[j] * tes[i].factors[EG];
                }
            }
            
            for (i = 0; i < NT; i++){
                gradients[i][MG] += localgradients[i][MG];
                gradients[i][EG] += localgradients[i][EG];
            }
        }
        
        for (i = 0; i < NT; i++){
            params[i][MG] += (2.0 / NP) * baseRate * rates[i][MG] * gradients[i][MG];
            params[i][EG] += (2.0 / NP) * baseRate * rates[i][EG] * gradients[i][EG];
        }
    }
}

void initializeTexelEntries(TexelEntry* tes, Thread* thread){
    
    int i, j;
    Undo undo;
    EvalInfo ei;
    Limits limits;
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
        
        if ((i + 1) % 1000 == 0 || i == NP - 1)
            printf("\rReading and Initializing Texel Entries from FENS...  [%7d of %7d]", i + 1, NP);
        
        fgets(line, 128, fin);
        
        // Determine the result of the game
        if      (strstr(line, "1-0")) tes[i].result = 1.0;
        else if (strstr(line, "1/2")) tes[i].result = 0.5;
        else if (strstr(line, "0-1")) tes[i].result = 0.0;
        else    {printf("Unable to Parse Result\n"); exit(0);}
        
        // Search, then and apply all moves in the principle variation
        initializeBoard(&thread->board, line);
        search(thread, &thread->pv, -MATE, MATE, 1, 0);
        for (j = 0; j < thread->pv.length; j++)
            applyMove(&thread->board, thread->pv.line[j], &undo);
            
        // Get the eval trace for the final position in the pv
        T = EmptyTrace;
        tes[i].eval = evaluateBoard(&thread->board, &ei, NULL);
        if (thread->board.turn == BLACK) tes[i].eval *= -1;
        
        // Determine the game phase based on remaining material
        tes[i].phase = 24 - 4 * popcount(thread->board.pieces[QUEEN ])
                          - 2 * popcount(thread->board.pieces[ROOK  ])
                          - 1 * popcount(thread->board.pieces[KNIGHT])
                          - 1 * popcount(thread->board.pieces[BISHOP]);
                          
        // When updating gradients, we use the coefficients for each
        // term, as well as the phase of the position it came from
        tes[i].factors[MG] = 1 - tes[i].phase / 24.0;
        tes[i].factors[EG] =     tes[i].phase / 24.0;
        
        // Finish determining the phase
        tes[i].phase = (tes[i].phase * 256 + 12) / 24.0;
        
        // Fill out tes[i].coeffs
        initializeCoefficients(&tes[i]);
    }
    
    fclose(fin);
}

void initializeCoefficients(TexelEntry* te){
    
    int i = 0, a, b, c;
    
    // Initialize coefficients for the pawns
    
    te->coeffs[i++] = T.pawnCounts[WHITE] - T.pawnCounts[BLACK];
    
    for (a = 0; a < 64; a++){
        te->coeffs[i + relativeSquare32(a, WHITE)] += T.pawnPSQT[WHITE][a];
        te->coeffs[i + relativeSquare32(a, BLACK)] -= T.pawnPSQT[BLACK][a];
    } i += 32;
    
    te->coeffs[i++] = T.pawnIsolated[WHITE] - T.pawnIsolated[BLACK];
    
    te->coeffs[i++] = T.pawnStacked[WHITE] - T.pawnStacked[BLACK];
    
    for (a = 0; a < 2; a++)
        te->coeffs[i++] = T.pawnBackwards[WHITE][a] - T.pawnBackwards[BLACK][a];
    
    for (a = 0; a < 64; a++){
        te->coeffs[i + relativeSquare32(a, WHITE)] += T.pawnConnected[WHITE][a];
        te->coeffs[i + relativeSquare32(a, BLACK)] -= T.pawnConnected[BLACK][a];
    } i += 32;
    
    
    // Initialze coefficients for the knights
    
    te->coeffs[i++] = T.knightCounts[WHITE] - T.knightCounts[BLACK];
    
    for (a = 0; a < 64; a++){
        te->coeffs[i + relativeSquare32(a, WHITE)] += T.knightPSQT[WHITE][a];
        te->coeffs[i + relativeSquare32(a, BLACK)] -= T.knightPSQT[BLACK][a];
    } i += 32;
    
    te->coeffs[i++] = T.knightAttackedByPawn[WHITE] - T.knightAttackedByPawn[BLACK];
    
    for (a = 0; a < 2; a++)
        te->coeffs[i++] = T.knightOutpost[WHITE][a] - T.knightOutpost[BLACK][a];
        
    for (a = 0; a < 9; a++)
        te->coeffs[i++] = T.knightMobility[WHITE][a] - T.knightMobility[BLACK][a];
    
    
    // Initialize coefficients for the bishops
    
    te->coeffs[i++] = T.bishopCounts[WHITE] - T.bishopCounts[BLACK];
    
    for (a = 0; a < 64; a++){
        te->coeffs[i + relativeSquare32(a, WHITE)] += T.bishopPSQT[WHITE][a];
        te->coeffs[i + relativeSquare32(a, BLACK)] -= T.bishopPSQT[BLACK][a];
    } i += 32;
    
    te->coeffs[i++] = T.bishopWings[WHITE] - T.bishopWings[BLACK];
    
    te->coeffs[i++] = T.bishopPair[WHITE] - T.bishopPair[BLACK];
    
    te->coeffs[i++] = T.bishopAttackedByPawn[WHITE] - T.bishopAttackedByPawn[BLACK];
    
    for (a = 0; a < 2; a++)
        te->coeffs[i++] = T.bishopOutpost[WHITE][a] - T.bishopOutpost[BLACK][a];
        
    for (a = 0; a < 14; a++)
        te->coeffs[i++] = T.bishopMobility[WHITE][a] - T.bishopMobility[BLACK][a];
    
    
    // Initialize coefficients for the rooks
    
    te->coeffs[i++] = T.rookCounts[WHITE] - T.rookCounts[BLACK];
    
    for (a = 0; a < 64; a++){
        te->coeffs[i + relativeSquare32(a, WHITE)] += T.rookPSQT[WHITE][a];
        te->coeffs[i + relativeSquare32(a, BLACK)] -= T.rookPSQT[BLACK][a];
    } i += 32;
    
    for (a = 0; a < 2; a++)
        te->coeffs[i++] = T.rookFile[WHITE][a] - T.rookFile[BLACK][a];
        
    te->coeffs[i++] = T.rookOnSeventh[WHITE] - T.rookOnSeventh[BLACK];
        
    for (a = 0; a < 15; a++)
        te->coeffs[i++] = T.rookMobility[WHITE][a] - T.rookMobility[BLACK][a];
    
    
    // Initialize coefficients for the queens
    
    te->coeffs[i++] = T.queenCounts[WHITE] - T.queenCounts[BLACK];
    
    te->coeffs[i++] = T.queenChecked[WHITE] - T.queenChecked[BLACK];
    
    te->coeffs[i++] = T.queenCheckedByPawn[WHITE] - T.queenCheckedByPawn[BLACK];
    
    for (a = 0; a < 64; a++){
        te->coeffs[i + relativeSquare32(a, WHITE)] += T.queenPSQT[WHITE][a];
        te->coeffs[i + relativeSquare32(a, BLACK)] -= T.queenPSQT[BLACK][a];
    } i += 32;
    
    for (a = 0; a < 28; a++)
        te->coeffs[i++] = T.queenMobility[WHITE][a] - T.queenMobility[BLACK][a];
    
    
    // Intitialize coefficients for the kings
    
    for (a = 0; a < 64; a++){
        te->coeffs[i + relativeSquare32(a, WHITE)] += T.kingPSQT[WHITE][a];
        te->coeffs[i + relativeSquare32(a, BLACK)] -= T.kingPSQT[BLACK][a];
    } i += 32;
    
    for (a = 0; a < 12; a++)
        te->coeffs[i++] = T.kingDefenders[WHITE][a] - T.kingDefenders[BLACK][a];
    
    for (a = 0; a < 2; a++)
        for (b = 0; b < 2; b++)
            for (c = 0; c < RANK_NB; c++)
                te->coeffs[i++] = T.kingShelter[WHITE][a][b][c] - T.kingShelter[BLACK][a][b][c];
    
    // Initialize coefficients for the passed pawns
    
    for (a = 0; a < 2; a++)
        for (b = 0; b < 2; b++)
            for (c = 0; c < RANK_NB; c++)
                te->coeffs[i++] = T.passedPawn[WHITE][a][b][c] - T.passedPawn[BLACK][a][b][c];
}

void initializeCurrentParameters(double cparams[NT][PHASE_NB]){
    
    int i = 0, a, b, c;
    
    // Initialize parameters for the pawns
    
    cparams[i  ][MG] = PawnValue[MG];
    cparams[i++][EG] = PawnValue[EG];
    
    for (a = 0; a < 32; a++, i++){
        cparams[i][MG] = PawnPSQT32[a][MG];
        cparams[i][EG] = PawnPSQT32[a][EG];
    }
    
    cparams[i  ][MG] = PawnIsolated[MG];
    cparams[i++][EG] = PawnIsolated[EG];
    
    cparams[i  ][MG] = PawnStacked[MG];
    cparams[i++][EG] = PawnStacked[EG];
    
    for (a = 0; a < 2; a++, i++){
        cparams[i][MG] = PawnBackwards[a][MG];
        cparams[i][EG] = PawnBackwards[a][EG];
    }
    
    for (a = 0; a < 32; a++, i++){
        cparams[i][MG] = PawnConnected32[a][MG];
        cparams[i][EG] = PawnConnected32[a][EG];
    }
    
    
    // Initialize parameters for the knights
    
    cparams[i  ][MG] = KnightValue[MG];
    cparams[i++][EG] = KnightValue[EG];
    
    for (a = 0; a < 32; a++, i++){
        cparams[i][MG] = KnightPSQT32[a][MG];
        cparams[i][EG] = KnightPSQT32[a][EG];
    }
    
    cparams[i  ][MG] = KnightAttackedByPawn[MG];
    cparams[i++][EG] = KnightAttackedByPawn[EG];
    
    for (a = 0; a < 2; a++, i++){
        cparams[i][MG] = KnightOutpost[a][MG];
        cparams[i][EG] = KnightOutpost[a][EG];
    }
    
    for (a = 0; a < 9; a++, i++){
        cparams[i][MG] = KnightMobility[a][MG];
        cparams[i][EG] = KnightMobility[a][EG];
    }
    
    
    // Initialize parameters for the bishops
    
    cparams[i  ][MG] = BishopValue[MG];
    cparams[i++][EG] = BishopValue[EG];
    
    for (a = 0; a < 32; a++, i++){
        cparams[i][MG] = BishopPSQT32[a][MG];
        cparams[i][EG] = BishopPSQT32[a][EG];
    }
    
    cparams[i  ][MG] = BishopWings[MG];
    cparams[i++][EG] = BishopWings[EG];
    
    cparams[i  ][MG] = BishopPair[MG];
    cparams[i++][EG] = BishopPair[EG];
    
    cparams[i  ][MG] = BishopAttackedByPawn[MG];
    cparams[i++][EG] = BishopAttackedByPawn[EG];
    
    for (a = 0; a < 2; a++, i++){
        cparams[i][MG] = BishopOutpost[a][MG];
        cparams[i][EG] = BishopOutpost[a][EG];
    }
    
    for (a = 0; a < 14; a++, i++){
        cparams[i][MG] = BishopMobility[a][MG];
        cparams[i][EG] = BishopMobility[a][EG];
    }
    
    
    // Initialize parameters for the rooks
    
    cparams[i  ][MG] = RookValue[MG];
    cparams[i++][EG] = RookValue[EG];
    
    for (a = 0; a < 32; a++, i++){
        cparams[i][MG] = RookPSQT32[a][MG];
        cparams[i][EG] = RookPSQT32[a][EG];
    }
    
    for (a = 0; a < 2; a++, i++){
        cparams[i][MG] = RookFile[a][MG];
        cparams[i][EG] = RookFile[a][EG];
    }
    
    cparams[i  ][MG] = RookOnSeventh[MG];
    cparams[i++][EG] = RookOnSeventh[EG];
    
    for (a = 0; a < 15; a++, i++){
        cparams[i][MG] = RookMobility[a][MG];
        cparams[i][EG] = RookMobility[a][EG];
    }
    
    
    // Initialize parameters for the queens
    
    cparams[i  ][MG] = QueenValue[MG];
    cparams[i++][EG] = QueenValue[EG];
    
    cparams[i  ][MG] = QueenChecked[MG];
    cparams[i++][EG] = QueenChecked[EG];
    
    cparams[i  ][MG] = QueenCheckedByPawn[MG];
    cparams[i++][EG] = QueenCheckedByPawn[EG];
    
    for (a = 0; a < 32; a++, i++){
        cparams[i][MG] = QueenPSQT32[a][MG];
        cparams[i][EG] = QueenPSQT32[a][EG];
    }
    
    for (a = 0; a < 28; a++, i++){
        cparams[i][MG] = QueenMobility[a][MG];
        cparams[i][EG] = QueenMobility[a][EG];
    }
    
    
    // Initialize parameters for the kings
    
    for (a = 0; a < 32; a++, i++){
        cparams[i][MG] = KingPSQT32[a][MG];
        cparams[i][EG] = KingPSQT32[a][EG];
    }
    
    for (a = 0; a < 12; a++, i++){
        cparams[i][MG] = KingDefenders[a][MG];
        cparams[i][EG] = KingDefenders[a][EG];
    }
    
    for (a = 0; a < 2; a++){
        for (b = 0; b < FILE_NB; b++){
            for (c = 0; c < RANK_NB; c++, i++){
                cparams[i][MG] = KingShelter[a][b][c][MG];
                cparams[i][EG] = KingShelter[a][b][c][EG];
            }
        }
    }
    
    // Initialize parameters for the passed pawns
    
    for (a = 0; a < 2; a++){
        for (b = 0; b < 2; b++){
            for (c = 0; c < RANK_NB; c++, i++){
                cparams[i][MG] = PassedPawn[a][b][c][MG];
                cparams[i][EG] = PassedPawn[a][b][c][EG];
            }
        }
    }
}

void calculateLearningRates(TexelEntry* tes, double rates[NT][PHASE_NB]){
    
    int i, j;
    double avgByPhase[PHASE_NB] = {0};
    double occurances[NT][PHASE_NB] = {{0}, {0}};
    
    for (i = 0; i < NP; i++){
        for (j = 0; j < NT; j++){
            occurances[j][MG] += abs(tes[i].coeffs[j]) * tes[i].factors[MG];
            occurances[j][EG] += abs(tes[i].coeffs[j]) * tes[i].factors[EG];
            avgByPhase[MG]    += abs(tes[i].coeffs[j]) * tes[i].factors[MG];
            avgByPhase[EG]    += abs(tes[i].coeffs[j]) * tes[i].factors[EG];
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
    
    double tparams[NT][PHASE_NB];
    
    for (x = 0; x < NT; x++){
        tparams[x][MG] = params[x][MG] + cparams[x][MG];
        tparams[x][EG] = params[x][EG] + cparams[x][EG];
    }    
    
    // Print Pawn Parameters
    
    printf("\nconst int PawnValue[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int PawnPSQT32[32][PHASE_NB] = {");
    for (x = 0; x < 8; x++){
        printf("\n   ");
        for (y = 0; y < 4; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    printf("\nconst int PawnIsolated[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int PawnStacked[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int PawnBackwards[2][PHASE_NB] = { {%4d,%4d}, {%4d,%4d} };\n",
            (int)tparams[i  ][MG], (int)tparams[i  ][EG],
            (int)tparams[i+1][MG], (int)tparams[i+1][EG]); i += 2;
    
    printf("\nconst int PawnConnected32[32][PHASE_NB] = {");
    for (x = 0; x < 8; x++){
        printf("\n   ");
        for (y = 0; y < 4; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    
    // Print Knight Parameters
    
    printf("\nconst int KnightValue[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int KnightPSQT32[32][PHASE_NB] = {");
    for (x = 0; x < 8; x++){
        printf("\n   ");
        for (y = 0; y < 4; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    printf("\nconst int KnightAttackedByPawn[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int KnightOutpost[2][PHASE_NB] = { {%4d,%4d}, {%4d,%4d} };\n",
            (int)tparams[i  ][MG], (int)tparams[i  ][EG],
            (int)tparams[i+1][MG], (int)tparams[i+1][EG]); i += 2;
            
    printf("\nconst int KnightMobility[9][PHASE_NB] = {");
    for (x = 0; x < 3; x++){
        printf("\n   ");
        for (y = 0; y < 3; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    
    // Print Bishop Parameters
    
    printf("\nconst int BishopValue[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int BishopPSQT32[32][PHASE_NB] = {");
    for (x = 0; x < 8; x++){
        printf("\n   ");
        for (y = 0; y < 4; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    printf("\nconst int BishopWings[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int BishopPair[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int BishopAttackedByPawn[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int BishopOutpost[2][PHASE_NB] = { {%4d,%4d}, {%4d,%4d} };\n",
            (int)tparams[i  ][MG], (int)tparams[i  ][EG],
            (int)tparams[i+1][MG], (int)tparams[i+1][EG]); i += 2;
            
    printf("\nconst int BishopMobility[14][PHASE_NB] = {");
    for (x = 0; x < 4; x++){
        printf("\n   ");
        for (y = 0; y < 4 && 4 * x + y < 14; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    
    // Print Rook Parameters
    
    printf("\nconst int RookValue[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int RookPSQT32[32][PHASE_NB] = {");
    for (x = 0; x < 8; x++){
        printf("\n   ");
        for (y = 0; y < 4; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    printf("\nconst int RookFile[2][PHASE_NB] = { {%4d,%4d}, {%4d,%4d} };\n",
            (int)tparams[i  ][MG], (int)tparams[i  ][EG],
            (int)tparams[i+1][MG], (int)tparams[i+1][EG]); i += 2;
            
    printf("\nconst int RookOnSeventh[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
            
    printf("\nconst int RookMobility[15][PHASE_NB] = {");
    for (x = 0; x < 4; x++){
        printf("\n   ");
        for (y = 0; y < 4 && x * 4 + y < 15; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    
    // Print Queen Parameters
    
    printf("\nconst int QueenValue[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int QueenChecked[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int QueenCheckedByPawn[PHASE_NB] = {%4d,%4d};\n", (int)tparams[i][MG], (int)tparams[i][EG]); i++;
    
    printf("\nconst int QueenPSQT32[32][PHASE_NB] = {");
    for (x = 0; x < 8; x++){
        printf("\n   ");
        for (y = 0; y < 4; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
            
    printf("\nconst int QueenMobility[28][PHASE_NB] = {");
    for (x = 0; x < 7; x++){
        printf("\n   ");
        for (y = 0; y < 4; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    
    // Print King Parameters
    
    printf("\nconst int KingPSQT32[32][PHASE_NB] = {");
    for (x = 0; x < 8; x++){
        printf("\n   ");
        for (y = 0; y < 4; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    printf("\nconst int KingDefenders[12][PHASE_NB] = {");
    for (x = 0; x < 3; x++){
        printf("\n   ");
        for (y = 0; y < 4; y++, i++)
            printf(" {%4d,%4d},", (int)tparams[i][MG], (int)tparams[i][EG]);
    } printf("\n};\n");
    
    printf("\nconst int KingShelter[2][FILE_NB][RANK_NB][PHASE_NB] = {");
    for (x = 0; x < 16; x++){
        printf("\n  %s", x % 8 ? " {" : "{{");
        for (y = 0; y < RANK_NB; y++, i++){
            printf("{%4d,%4d}", (int)tparams[i][MG], (int)tparams[i][EG]);
            printf("%s", y < RANK_NB - 1 ? ", " : x % 8 == 7 ? "}}," : "},");
        }
    } printf("\n};\n");
    
    // Print Passed Pawn Parameters
    
    printf("\nconst int PassedPawn[2][2][RANK_NB][PHASE_NB] = {");
    for (x = 0; x < 4; x++){
        printf("\n  %s", x % 2 ? " {" : "{{");
        for (y = 0; y < RANK_NB; y++, i++){
            printf("{%4d,%4d}", (int)tparams[i][MG], (int)tparams[i][EG]);
            printf("%s", y < RANK_NB - 1 ? ", " : x % 2 ? "}}," : "},");
        }
    } printf("\n};\n");
}

double computeOptimalK(TexelEntry* tes){
    
    int i;
    double start = -10.0, end = 10.0, delta = 1.0;
    double curr = start, thisError, bestError = completeEvaluationError(tes, start);
    
    for (i = 0; i < 10; i++){
        printf("Computing K Iteration [%d] ", i);
        
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
    
    for (i = 0; i < NT; i++){
        mg += te.coeffs[i] * params[i][MG];
        eg += te.coeffs[i] * params[i][EG];
    }
    
    return te.eval + ((mg * (256 - te.phase) + eg * te.phase) / 256.0);
}

double sigmoid(double K, double S){
    return 1.0 / (1.0 + pow(10.0, -K * S / 400.0));
}

#endif
