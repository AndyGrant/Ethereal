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

#include "attacks.h"
#include "board.h"
#include "evaluate.h"
#include "fathom/tbprobe.h"
#include "history.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "psqt.h"
#include "search.h"
#include "texel.h"
#include "thread.h"
#include "time.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"
#include "zobrist.h"


extern int MoveOverhead; // Defined by Time.c

extern unsigned TB_PROBE_DEPTH; // Defined by Syzygy.c

extern volatile int ABORT_SIGNAL; // For killing active search

extern volatile int IS_PONDERING; // For swapping out of PONDER

pthread_mutex_t READYLOCK = PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char **argv) {

    Board board;
    char str[8192], *ptr;
    ThreadsGo threadsgo;
    pthread_t pthreadsgo;

    int nthreads = argc > 3 ? atoi(argv[3]) : 1;
    int megabytes = argc > 4 ? atoi(argv[4]) : 16;

    // Initialize the core components of Ethereal
    initAttacks();
    initializePSQT();
    initMasks();
    initZobrist();
    initSearch();

    // Default to 16MB TT
    initTT(megabytes);

    // Not required, but always setup the board from the starting position
    boardFromFEN(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Build our Thread Pool, with default size of 1-thread
    Thread* threads = createThreadPool(nthreads);

    #ifdef TUNE
        runTexelTuning(threads);
        exit(0);
    #endif

    if (argc > 1 && stringEquals(argv[1], "bench")) {
        runBenchmark(threads, argc > 2 ? atoi(argv[2]) : 0);
        return 0;
    }

    while (1){

        getInput(str);

        if (stringEquals(str, "uci")){
            printf("id name Ethereal " ETHEREAL_VERSION "\n");
            printf("id author Andrew Grant & Laldon\n");
            printf("option name Hash type spin default 16 min 1 max 65536\n");
            printf("option name Threads type spin default 1 min 1 max 2048\n");
            printf("option name MoveOverhead type spin default 100 min 0 max 10000\n");
            printf("option name SyzygyPath type string default <empty>\n");
            printf("option name SyzygyProbeDepth type spin default 0 min 0 max 127\n");
            printf("option name Ponder type check default false\n");
            printf("uciok\n");
            fflush(stdout);
        }

        else if (stringEquals(str, "isready")){
            pthread_mutex_lock(&READYLOCK);
            printf("readyok\n");
            fflush(stdout);
            pthread_mutex_unlock(&READYLOCK);
        }

        else if (stringStartsWith(str, "setoption")){

            if (stringStartsWith(str, "setoption name Hash value ")){
                megabytes = atoi(str + strlen("setoption name Hash value "));
                initTT(megabytes);
                printf("info string set Hash to %dMB\n", megabytes);
            }

            if (stringStartsWith(str, "setoption name Threads value ")){
                free(threads);
                nthreads = atoi(str + strlen("setoption name Threads value "));
                threads = createThreadPool(nthreads);
                printf("info string set Threads to %d\n", nthreads);
            }

            if (stringStartsWith(str, "setoption name MoveOverhead value ")){
                MoveOverhead = atoi(str + strlen("setoption name MoveOverhead value "));
                printf("info string set MoveOverhead to %d\n", MoveOverhead);
            }

            if (stringStartsWith(str, "setoption name SyzygyPath value ")){
                ptr = str + strlen("setoption name SyzygyPath value ");
                tb_init(ptr); printf("info string set SyzygyPath to %s\n", ptr);
            }

            if (stringStartsWith(str, "setoption name SyzygyProbeDepth value ")){
                TB_PROBE_DEPTH = atoi(str + strlen("setoption name SyzygyProbeDepth value "));
                printf("info string set SyzygyProbeDepth to %u\n", TB_PROBE_DEPTH);
            }

            fflush(stdout);
        }

        else if (stringEquals(str, "ucinewgame")){
            resetThreadPool(threads);
            clearTT();
        }

        else if (stringStartsWith(str, "position"))
            uciPosition(str, &board);

        else if (stringStartsWith(str, "go")){
            strncpy(threadsgo.str, str, 512);
            threadsgo.threads = threads;
            threadsgo.board = &board;
            pthread_create(&pthreadsgo, NULL, &uciGo, &threadsgo);
        }

        else if (stringEquals(str, "ponderhit"))
            IS_PONDERING = 0;

        else if (stringEquals(str, "stop")){
            ABORT_SIGNAL = 1;
            IS_PONDERING = 0;
            pthread_join(pthreadsgo, NULL);
        }

        else if (stringEquals(str, "quit"))
            break;

        else if (stringStartsWith(str, "perft")){
            printf("%"PRIu64"\n", perft(&board, atoi(str + strlen("perft "))));
            fflush(stdout);
        }

        else if (stringStartsWith(str, "print")){
            printBoard(&board);
            fflush(stdout);
        }
    }

    return 0;
}

void* uciGo(void* vthreadsgo){

    // Get our starting time as soon as possible
    double start = getRealTime();

    // Grab the ready lock, as we cannot be ready until we finish this search
    pthread_mutex_lock(&READYLOCK);

    char* str       = ((ThreadsGo*)vthreadsgo)->str;
    Board* board    = ((ThreadsGo*)vthreadsgo)->board;
    Thread* threads = ((ThreadsGo*)vthreadsgo)->threads;

    Limits limits; limits.start = start;

    uint16_t bestMove, ponderMove;
    char bestMoveStr[6], ponderMoveStr[6];

    int depth = -1, infinite = -1;
    double wtime = -1, btime = -1, mtg = -1, movetime = -1;
    double winc = 0, binc = 0;

    // Reset pondering flag before starting search
    IS_PONDERING = 0;

    // Init the tokenizer with spaces
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

        else if (stringEquals(ptr, "ponder"))
            IS_PONDERING = 1;
    }

    // Initialize limits for the search
    limits.limitedByNone  = infinite != -1;
    limits.limitedByTime  = movetime != -1;
    limits.limitedByDepth = depth    != -1;
    limits.limitedBySelf  = depth == -1 && movetime == -1 && infinite == -1;
    limits.timeLimit      = movetime;
    limits.depthLimit     = depth;

    // Pick the time values for the colour we are playing as
    limits.time = (board->turn == WHITE) ? wtime : btime;
    limits.mtg  = (board->turn == WHITE) ?   mtg :   mtg;
    limits.inc  = (board->turn == WHITE) ?  winc :  binc;

    // Execute search, return best and ponder moves
    getBestMove(threads, board, &limits, &bestMove, &ponderMove);

    // UCI spec does not want reports until out of pondering
    while (IS_PONDERING);

    // Report best move (we should always have one)
    moveToString(bestMove, bestMoveStr);
    printf("bestmove %s ", bestMoveStr);

    // Report ponder move if we have one
    if (ponderMove != NONE_MOVE) {
        moveToString(ponderMove, ponderMoveStr);
        printf("ponder %s", ponderMoveStr);
    }

    // Make sure this all gets reported
    printf("\n"); fflush(stdout);

    // Drop the ready lock, as we are prepared to handle a new search
    pthread_mutex_unlock(&READYLOCK);

    return NULL;
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
        boardFromFEN(board, strstr(str, "fen") + strlen("fen "));

    // Position just starts at the normal beggining of game
    else if (stringContains(str, "startpos"))
        boardFromFEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

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
            moveToString(moves[size], test);
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

void uciReport(Thread* threads, int alpha, int beta, int value){

    PVariation* pv  = &threads[0].pv;
    int hashfull    = hashfullTT();
    int depth       = threads[0].depth;
    int seldepth    = threads[0].seldepth;
    int elapsed     = elapsedTime(threads[0].info);
    uint64_t nodes  = nodesSearchedThreadPool(threads);
    uint64_t tbhits = tbhitsSearchedThreadPool(threads);
    int nps         = (int)(1000 * (nodes / (1 + elapsed)));

    value = MAX(alpha, MIN(value, beta));

    // If the score is MATE or MATED in X, convert to X
    int score   = value >=  MATE_IN_MAX ?  (MATE - value + 1) / 2
                : value <= MATED_IN_MAX ? -(value + MATE)     / 2 : value;

    // Two possible score types, mate and cp = centipawns
    char* type  = value >=  MATE_IN_MAX ? "mate"
                : value <= MATED_IN_MAX ? "mate" : "cp";

    // Partial results from a window'ed search have bounds
    char* bound = value >=  beta ? " lowerbound "
                : value <= alpha ? " upperbound " : " ";

    // Main chunk of interface reporting
    printf("info depth %d seldepth %d score %s %d%stime %d "
           "nodes %"PRIu64" nps %d tbhits %"PRIu64" hashfull %d pv ",
           depth, seldepth, type, score, bound, elapsed, nodes, nps, tbhits, hashfull);

    // Iterate over the PV and print each move
    for (int i = 0; i < pv->length; i++){
        char moveStr[6];
        moveToString(pv->line[i], moveStr);
        printf("%s ", moveStr);
    }

    puts("");
    fflush(stdout);
}

void uciReportTBRoot(uint16_t move, unsigned wdl, unsigned dtz){

    int score = wdl == TB_LOSS ? -MATE + MAX_PLY + dtz + 1
              : wdl == TB_WIN  ?  MATE - MAX_PLY - dtz - 1 : 0;

    printf("info depth %d seldepth %d score cp %d time 0 "
           "nodes 0 tbhits 1 nps 0 hashfull %d pv ",
           MAX_PLY - 1, MAX_PLY - 1, score, 0);

    char moveStr[6];
    moveToString(move, moveStr);
    puts(moveStr);
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
