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

#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "history.h"
#include "magics.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "piece.h"
#include "psqt.h"
#include "search.h"
#include "texel.h"
#include "thread.h"
#include "time.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"
#include "zorbist.h"

extern TransTable Table;

int main(){
    
    Board board;
    char str[8192];
    
    int megabytes = 16;
    
    int nthreads =  1;
    Thread* threads = createThreadPool(nthreads);
    
    initializeMagics();
    initializeZorbist();
    initializePSQT();
    initializeMasks();
    initializeBoard(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    initializeTranspositionTable(&Table, megabytes);
    
    #ifdef TUNE
        runTexelTuning(threads);
        exit(0);
    #endif
    
    while (1){
        
        getInput(str);
        
        if (stringEquals(str, "uci")){
            printf("id name Ethereal 8.88\n");
            printf("id author Andrew Grant\n");
            printf("option name Hash type spin default 16 min 1 max 65536\n");
            printf("option name Threads type spin default 1 min 1 max 2048\n");
            printf("uciok\n");
            fflush(stdout);
        }
        
        else if (stringEquals(str, "isready")){
            printf("readyok\n");
            fflush(stdout);
        } 
        
        else if (stringStartsWith(str, "setoption")){
            
            if (stringStartsWith(str, "setoption name Hash value")){
                megabytes = atoi(str + strlen("setoption name Hash value"));
                destroyTranspositionTable(&Table);
                initializeTranspositionTable(&Table, megabytes);
            }
            
            if (stringStartsWith(str, "setoption name Threads value")){
                free(threads);
                nthreads = atoi(str + strlen("setoption name Threads value"));
                threads = createThreadPool(nthreads);
            }
        }
        
        else if (stringEquals(str, "ucinewgame")){
            resetThreadPool(threads);
            clearTranspositionTable(&Table);
        }
        
        else if (stringStartsWith(str, "position"))
            uciPosition(str, &board);
        
        else if (stringStartsWith(str, "go"))
            uciGo(str, threads, &board);
        
        else if (stringEquals(str, "quit"))
            break;
        
        else if (stringStartsWith(str, "perft")){
            printf("%"PRIu64"\n", perft(&board, atoi(str + strlen("perft "))));
            fflush(stdout);
        }
        
        else if (stringStartsWith(str, "bench"))
            runBenchmark(threads, atoi(str + strlen("bench ")));
    }
    
    return 1;
}

void uciGo(char* str, Thread* threads, Board* board){
    
    Limits limits;
    
    char move[6];
    int depth = -1, infinite = -1; 
    double time = 0, wtime = -1, btime = -1, mtg = -1, movetime = -1;
    double inc = 0, winc = 0, binc = 0;
    
    char* ptr = strtok(str, " ");
    
    // Parse time control and search type parameters
    for (ptr = strtok(NULL, " "); ptr != NULL; ptr = strtok(NULL, " ")){
        
        if (stringEquals(ptr, "wtime"))
            wtime = (double)(atoi(strtok(NULL, " ")));
        
        else if (stringEquals(ptr, "btime"))
            btime = (double)(atoi(strtok(NULL, " ")));
        
        else if (stringEquals(ptr, "winc"))
            winc = (double)(atoi(strtok(NULL, " ")));
        
        else if (stringEquals(ptr, "binc"))
            binc = (double)(atoi(strtok(NULL, " ")));
        
        else if (stringEquals(ptr, "movestogo"))
            mtg = (double)(atoi(strtok(NULL, " ")));
        
        else if (stringEquals(ptr, "depth"))
            depth = atoi(strtok(NULL, " "));
        
        else if (stringEquals(ptr, "movetime"))
            movetime = (double)(atoi(strtok(NULL, " ")));
        
        else if (stringEquals(ptr, "infinite"))
            infinite = 1;
    }
    
    // Initialize limits for the search
    limits.limitedByNone  = infinite != -1;
    limits.limitedByTime  = movetime != -1;
    limits.limitedByDepth = depth    != -1;
    limits.limitedBySelf  = depth == -1 && movetime == -1 && infinite == -1;
    limits.timeLimit      = movetime; 
    limits.depthLimit     = depth;
    
    // Pick the time values for the colour we are playing as
    time = (board->turn == WHITE) ? wtime : btime;
    inc  = (board->turn == WHITE) ?  winc :  binc;
    
    // Execute the search and report the best move
    moveToString(move, getBestMove(threads, board, &limits, time, mtg, inc));
    printf("bestmove %s\n", move);
    fflush(stdout);
}

void uciPosition(char* str, Board* board){
    
    int size;
    char* ptr;
    char move[6];
    char test[6];
    Undo undo[1];
    uint16_t moves[MAX_MOVES];
    
    // Position is defined by a FEN string
    if (stringContains(str, "fen"))
        initializeBoard(board, strstr(str, "fen") + strlen("fen "));
    
    // Position just starts at the normal beggining of game
    else if (stringContains(str, "startpos"))
        initializeBoard(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Position command may include a list of moves
    ptr = strstr(str, "moves");
    if (ptr != NULL) ptr += strlen("moves ");
    
    // Apply each move in the move list
    while (ptr != NULL && *ptr != '\0'){
        
        // Generate moves for this position
        size = 0;
        genAllMoves(board, moves, &size);
        
        // Move is in long algebraic notation
        move[0] = *ptr++; move[1] = *ptr++;
        move[2] = *ptr++; move[3] = *ptr++;
        move[4] = *ptr == '\0' || *ptr == ' ' ? '\0' : *ptr++;
        move[5] = '\0';

        // Find and apply the correct move
        for (size -= 1; size >= 0; size--){
            moveToString(test, moves[size]);
            if (stringEquals(move, test)){
                applyMove(board, moves[size], undo);
                break;
            }
        }
        
        // Skip over all white space
        while (*ptr == ' ') ptr++;
        
        // Reset move history whenever we reset the fifty move rule
        if (board->fiftyMoveRule == 0) board->numMoves = 0;
    }
}

void uciReport(Thread* threads, double startTime, int depth, int value, PVariation* pv){
    
    int i;
    int elapsed    = (int)(getRealTime() - startTime);
    int hashfull   = (1000 * Table.used) / (Table.numBuckets * BUCKET_SIZE);
    uint64_t nodes =  nodesSearchedThreadPool(threads);
    int nps        = (int)(1000 * (nodes / (1 + elapsed)));
    
    printf("info depth %d score cp %d time %d nodes %"PRIu64" nps %d hashfull %d pv ",
           depth, value, elapsed, nodes, nps, hashfull);
           
    for (i = 0; i < pv->length; i++){
        printMove(pv->line[i]);
        printf(" ");
    }
    
    printf("\n");
    fflush(stdout);
}

int stringEquals(char* s1, char* s2){
    return strcmp(s1, s2) == 0;
}

int stringStartsWith(char* str, char* key){
    return strstr(str, key) == str;
}

int stringContains(char* str, char* key){
    return strstr(str, key) != NULL;
}

void getInput(char* str){
    
    char* ptr;
    
    if (fgets(str, 8192, stdin) == NULL)
        exit(EXIT_FAILURE);
    
    ptr = strchr(str, '\n');
    if (ptr != NULL) *ptr = '\0';
    
    ptr = strchr(str, '\r');
    if (ptr != NULL) *ptr = '\0';
}

void moveToString(char* str, uint16_t move){
    
    static char table[4] = {'n', 'b', 'r', 'q'};
    
    str[0] = 'a' + (MoveFrom(move) % 8);
    str[1] = '1' + (MoveFrom(move) / 8);
    str[2] = 'a' + (  MoveTo(move) % 8);
    str[3] = '1' + (  MoveTo(move) / 8);
    str[4] = MoveType(move) == PROMOTION_MOVE ? table[move >> 14] : '\0';
    str[5] = '\0';
}
