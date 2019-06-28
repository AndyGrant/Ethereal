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
#include "search.h"
#include "texel.h"
#include "thread.h"
#include "time.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"
#include "zobrist.h"

extern int MoveOverhead;          // Defined by Time.c
extern unsigned TB_PROBE_DEPTH;   // Defined by Syzygy.c
extern volatile int ABORT_SIGNAL; // Defined by Search.c
extern volatile int IS_PONDERING; // Defined by Search.c

pthread_mutex_t READYLOCK = PTHREAD_MUTEX_INITIALIZER;
const char *StartPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int main(int argc, char **argv) {

    Board board;
    char str[8192];
    Thread *threads;
    ThreadsGo threadsgo;
    pthread_t pthreadsgo;

    // By default use 1 thread, 16MB hash, and Standard chess.
    // Usage ./Ethereal bench <depth=13> <threads=1> <hash=16>
    int depth     = argc > 2 ? atoi(argv[2]) : 13;
    int nthreads  = argc > 3 ? atoi(argv[3]) : 1;
    int megabytes = argc > 4 ? atoi(argv[4]) : 16;
    int chess960  = 0;

    // Initialize the core components of Ethereal
    initAttacks(); initMasks(); initEval();
    initSearch(); initZobrist(); initTT(megabytes);
    threads = createThreadPool(nthreads);
    boardFromFEN(&board, StartPosition, chess960);

    // Allow the bench to be run from the command line
    if (argc > 1 && stringEquals(argv[1], "bench")) {
        runBenchmark(threads, depth);
        return 0;
    }

    // Allow the tuner to be run when compiled
    #ifdef TUNE
        runTexelTuning(threads);
        return 0;
    #endif

    while (getInput(str)) {

        if (stringEquals(str, "uci")) {
            printf("id name Ethereal " ETHEREAL_VERSION "\n");
            printf("id author Andrew Grant & Laldon\n");
            printf("option name Hash type spin default 16 min 1 max 65536\n");
            printf("option name Threads type spin default 1 min 1 max 2048\n");
            printf("option name MoveOverhead type spin default 100 min 0 max 10000\n");
            printf("option name SyzygyPath type string default <empty>\n");
            printf("option name SyzygyProbeDepth type spin default 0 min 0 max 127\n");
            printf("option name Ponder type check default false\n");
            printf("option name UCI_Chess960 type check default false\n");
            printf("uciok\n"), fflush(stdout);
        }

        else if (stringEquals(str, "isready")) {
            pthread_mutex_lock(&READYLOCK);
            printf("readyok\n"), fflush(stdout);
            pthread_mutex_unlock(&READYLOCK);
        }

        else if (stringEquals(str, "ucinewgame"))
            resetThreadPool(threads), clearTT();

        else if (stringStartsWith(str, "setoption"))
            uciSetOption(str, &megabytes, &chess960, &nthreads, &threads);

        else if (stringStartsWith(str, "position"))
            uciPosition(str, &board, chess960);

        else if (stringStartsWith(str, "go")) {
            strncpy(threadsgo.str, str, 512);
            threadsgo.threads = threads;
            threadsgo.board = &board;
            pthread_create(&pthreadsgo, NULL, &uciGo, &threadsgo);
        }

        else if (stringEquals(str, "ponderhit"))
            IS_PONDERING = 0;

        else if (stringEquals(str, "stop")) {
            ABORT_SIGNAL = 1, IS_PONDERING = 0;
            pthread_join(pthreadsgo, NULL);
        }

        else if (stringEquals(str, "quit"))
            break;

        else if (stringStartsWith(str, "perft"))
            printf("%"PRIu64"\n", perft(&board, atoi(str + strlen("perft ")))), fflush(stdout);

        else if (stringStartsWith(str, "print"))
            printBoard(&board), fflush(stdout);
    }

    return 0;
}

void *uciGo(void *vthreadsgo) {

    // Get our starting time as soon as possible
    double start = getRealTime();

    Limits limits;

    uint16_t bestMove, ponderMove;
    char moveStr[6];

    int depth = 0, infinite = 0;
    double wtime = 0, btime = 0, movetime = 0;
    double winc = 0, binc = 0, mtg = -1;

    char *str       = ((ThreadsGo*)vthreadsgo)->str;
    Board *board    = ((ThreadsGo*)vthreadsgo)->board;
    Thread *threads = ((ThreadsGo*)vthreadsgo)->threads;

    // Grab the ready lock, as we cannot be ready until we finish this search
    pthread_mutex_lock(&READYLOCK);

    // Reset global signals
    IS_PONDERING = 0;

    // Init the tokenizer with spaces
    char* ptr = strtok(str, " ");

    // Parse any time control and search method information that was sent
    for (ptr = strtok(NULL, " "); ptr != NULL; ptr = strtok(NULL, " ")) {

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
    limits.limitedByNone  = infinite != 0;
    limits.limitedByTime  = movetime != 0;
    limits.limitedByDepth = depth    != 0;
    limits.limitedBySelf  = !depth && !movetime && !infinite;
    limits.timeLimit      = movetime;
    limits.depthLimit     = depth;

    // Pick the time values for the colour we are playing as
    limits.start = (board->turn == WHITE) ? start : start;
    limits.time  = (board->turn == WHITE) ? wtime : btime;
    limits.inc   = (board->turn == WHITE) ?  winc :  binc;
    limits.mtg   = (board->turn == WHITE) ?   mtg :   mtg;

    // Execute search, return best and ponder moves
    getBestMove(threads, board, &limits, &bestMove, &ponderMove);

    // UCI spec does not want reports until out of pondering
    while (IS_PONDERING);

    // Report best move ( we should always have one )
    moveToString(board, bestMove, moveStr);
    printf("bestmove %s ", moveStr);

    // Report ponder move ( if we have one )
    if (ponderMove != NONE_MOVE) {
        moveToString(board, ponderMove, moveStr);
        printf("ponder %s", moveStr);
    }

    // Make sure this all gets reported
    printf("\n"); fflush(stdout);

    // Drop the ready lock, as we are prepared to handle a new search
    pthread_mutex_unlock(&READYLOCK);

    return NULL;
}

void uciSetOption(char *str, int *megabytes, int *chess960, int *nthreads, Thread **threads) {

    // Handle setting UCI options in Ethereal. Options include:
    //   Hash             : Size of the Transposition Table in Megabyes
    //   Threads          : Number of search threads to use
    //   MoveOverhead     : Overhead on time allocation to avoid time losses
    //   SyzygyPath       : Path to Syzygy Tablebases
    //   SyzygyProbeDepth : Minimal Depth to probe the highest cardinality Tablebase
    //   UCI_Chess960     : Set when playing FRC, but not required in order to work

    if (stringStartsWith(str, "setoption name Hash value ")) {
        *megabytes = atoi(str + strlen("setoption name Hash value "));
        initTT(*megabytes); printf("info string set Hash to %dMB\n", *megabytes);
    }

    if (stringStartsWith(str, "setoption name Threads value ")) {
        free(*threads);
        *nthreads = atoi(str + strlen("setoption name Threads value "));
        *threads = createThreadPool(*nthreads);
        printf("info string set Threads to %d\n", *nthreads);
    }

    if (stringStartsWith(str, "setoption name MoveOverhead value ")) {
        MoveOverhead = atoi(str + strlen("setoption name MoveOverhead value "));
        printf("info string set MoveOverhead to %d\n", MoveOverhead);
    }

    if (stringStartsWith(str, "setoption name SyzygyPath value ")) {
        char *ptr = str + strlen("setoption name SyzygyPath value ");
        tb_init(ptr); printf("info string set SyzygyPath to %s\n", ptr);
    }

    if (stringStartsWith(str, "setoption name SyzygyProbeDepth value ")) {
        TB_PROBE_DEPTH = atoi(str + strlen("setoption name SyzygyProbeDepth value "));
        printf("info string set SyzygyProbeDepth to %u\n", TB_PROBE_DEPTH);
    }

    if (stringStartsWith(str, "setoption name UCI_Chess960 value ")) {
        if (stringStartsWith(str, "setoption name UCI_Chess960 value true"))
            printf("info string set UCI_Chess960 to true\n"), *chess960 = 1;
        if (stringStartsWith(str, "setoption name UCI_Chess960 value false"))
            printf("info string set UCI_Chess960 to false\n"), *chess960 = 0;
    }

    fflush(stdout);
}

void uciPosition(char *str, Board *board, int chess960) {

    int size;
    uint16_t moves[MAX_MOVES];
    char *ptr, moveStr[6],testStr[6];
    Undo undo[1];

    // Position is defined by a FEN, X-FEN or Shredder-FEN
    if (stringContains(str, "fen"))
        boardFromFEN(board, strstr(str, "fen") + strlen("fen "), chess960);

    // Position is simply the usual starting position
    else if (stringContains(str, "startpos"))
        boardFromFEN(board, StartPosition, chess960);

    // Position command may include a list of moves
    ptr = strstr(str, "moves");
    if (ptr != NULL)
        ptr += strlen("moves ");

    // Apply each move in the move list
    while (ptr != NULL && *ptr != '\0') {

        // UCI sends moves in long algebraic notation
        for (int i = 0; i < 4; i++) moveStr[i] = *ptr++;
        moveStr[4] = *ptr == '\0' || *ptr == ' ' ? '\0' : *ptr++;
        moveStr[5] = '\0';

        // Generate moves for this position
        size = 0; genAllLegalMoves(board, moves, &size);

        // Find and apply the given move
        for (int i = 0; i < size; i++) {
            moveToString(board, moves[i], testStr);
            if (stringEquals(moveStr, testStr)) {
                applyMove(board, moves[i], undo);
                break;
            }
        }

        // Reset move history whenever we reset the fifty move rule. This way
        // we can track all positions that are candidates for repetitions, and
        // are still able to use a fixed size for the history array (512)
        if (board->halfMoveCounter == 0)
            board->numMoves = 0;

        // Skip over all white space
        while (*ptr == ' ') ptr++;
    }
}

void uciReport(Thread *threads, int alpha, int beta, int value) {

    // Gather all of the statistics that the UCI protocol would be
    // interested in. Also, bound the value passed by alpha and
    // beta, since Ethereal uses a mix of fail-hard and fail-soft

    int hashfull    = hashfullTT();
    int depth       = threads->depth;
    int seldepth    = threads->seldepth;
    int elapsed     = elapsedTime(threads->info);
    int bounded     = value = MAX(alpha, MIN(value, beta));
    uint64_t nodes  = nodesSearchedThreadPool(threads);
    uint64_t tbhits = tbhitsThreadPool(threads);
    int nps         = (int)(1000 * (nodes / (1 + elapsed)));

    // If the score is MATE or MATED in X, convert to X
    int score   = bounded >=  MATE_IN_MAX ?  (MATE - bounded + 1) / 2
                : bounded <= MATED_IN_MAX ? -(bounded + MATE)     / 2 : bounded;

    // Two possible score types, mate and cp = centipawns
    char *type  = bounded >=  MATE_IN_MAX ? "mate"
                : bounded <= MATED_IN_MAX ? "mate" : "cp";

    // Partial results from a windowed search have bounds
    char *bound = bounded >=  beta ? " lowerbound "
                : bounded <= alpha ? " upperbound " : " ";

    printf("info depth %d seldepth %d score %s %d%stime %d "
           "nodes %"PRIu64" nps %d tbhits %"PRIu64" hashfull %d pv ",
           depth, seldepth, type, score, bound, elapsed, nodes, nps, tbhits, hashfull);

    // Iterate over the PV and print each move
    for (int i = 0; i < threads->pv.length; i++) {
        char moveStr[6];
        moveToString(&threads->board, threads->pv.line[i], moveStr);
        printf("%s ", moveStr);
    }

    // Send out a newline and flush
    puts(""); fflush(stdout);
}

void uciReportTBRoot(Board *board, uint16_t move, unsigned wdl, unsigned dtz) {

    char moveStr[6];

    // Convert result to a score. We place wins and losses just outside
    // the range of possible mate scores, and move further from them
    // as the depth to zero increases. Draws are of course, zero.
    int score = wdl == TB_LOSS ? -MATE + MAX_PLY + dtz + 1
              : wdl == TB_WIN  ?  MATE - MAX_PLY - dtz - 1 : 0;

    printf("info depth %d seldepth %d score cp %d time 0 "
           "nodes 0 tbhits 1 nps 0 hashfull %d pv ",
           MAX_PLY - 1, MAX_PLY - 1, score, 0);

    // Print out the given move
    moveToString(board, move, moveStr);
    puts(moveStr);
    fflush(stdout);
}

int stringEquals(char *str1, char *str2) {
    return strcmp(str1, str2) == 0;
}

int stringStartsWith(char *str, char *key) {
    return strstr(str, key) == str;
}

int stringContains(char *str, char *key) {
    return strstr(str, key) != NULL;
}

int getInput(char *str) {

    char *ptr;

    if (fgets(str, 8192, stdin) == NULL)
        return 0;

    ptr = strchr(str, '\n');
    if (ptr != NULL) *ptr = '\0';

    ptr = strchr(str, '\r');
    if (ptr != NULL) *ptr = '\0';

    return 1;
}
