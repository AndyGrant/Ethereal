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
#include "cmdline.h"
#include "evaluate.h"
#include "history.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "network.h"
#include "nnue/nnue.h"
#include "pyrrhic/tbprobe.h"
#include "search.h"
#include "thread.h"
#include "timeman.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"
#include "zobrist.h"

int NORMALIZE_EVAL = 1;

extern int MoveOverhead;          // Defined by time.c
extern unsigned TB_PROBE_DEPTH;   // Defined by syzygy.c
extern volatile int ABORT_SIGNAL; // Defined by search.c
extern volatile int IS_PONDERING; // Defined by search.c
extern PKNetwork PKNN;            // Defined by network.c

const char *StartPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int main(int argc, char **argv) {

    Board board;
    char str[8192] = {0};
    Thread *threads;
    pthread_t pthreadsgo;
    UCIGoStruct uciGoStruct;

    int chess960 = 0;
    int multiPV  = 1;

    // Initialize core components of Ethereal
    initAttacks(); initMasks(); initEval();
    initSearch(); initZobrist(); tt_init(1, 16);
    initPKNetwork(); nnue_incbin_init();

    // Create the UCI-board and our threads
    threads = createThreadPool(1);
    boardFromFEN(&board, StartPosition, chess960);

    // Handle any command line requests
    handleCommandLine(argc, argv);

    /*
    |------------|-----------------------------------------------------------------------|
    |  Commands  | Response. * denotes that the command blocks until no longer searching |
    |------------|-----------------------------------------------------------------------|
    |        uci |           Outputs the engine name, authors, and all available options |
    |    isready | *           Responds with readyok when no longer searching a position |
    | ucinewgame | *  Resets the TT and any Hueristics to ensure determinism in searches |
    |  setoption | *     Sets a given option and reports that the option was set if done |
    |   position | *  Sets the board position via an optional FEN and optional move list |
    |         go | *       Searches the current position with the provided time controls |
    |  ponderhit |          Flags the search to indicate that the ponder move was played |
    |       stop |            Signals the search threads to finish and report a bestmove |
    |       quit |             Exits the engine and any searches by killing the UCI loop |
    |      perft |            Custom command to compute PERFT(N) of the current position |
    |      print |         Custom command to print an ASCII view of the current position |
    |------------|-----------------------------------------------------------------------|
    */

    while (getInput(str)) {

        if (strEquals(str, "uci")) {
            printf("id name Ethereal " ETHEREAL_VERSION "\n");
            printf("id author Andrew Grant, Alayan & Laldon\n");
            printf("option name Hash type spin default 16 min 2 max 131072\n");
            printf("option name Threads type spin default 1 min 1 max 2048\n");
            printf("option name EvalFile type string default <empty>\n");
            printf("option name MultiPV type spin default 1 min 1 max 256\n");
            printf("option name MoveOverhead type spin default 300 min 0 max 10000\n");
            printf("option name SyzygyPath type string default <empty>\n");
            printf("option name SyzygyProbeDepth type spin default 0 min 0 max 127\n");
            printf("option name Ponder type check default false\n");
            printf("option name Normalize type check default true\n");
            printf("option name UCI_Chess960 type check default false\n");
            printf("info string licensed to " LICENSE_OWNER "\n");
            printf("uciok\n"), fflush(stdout);
        }

        else if (strEquals(str, "isready"))
            printf("readyok\n"), fflush(stdout);

        else if (strEquals(str, "ucinewgame"))
            resetThreadPool(threads), tt_clear(threads->nthreads);

        else if (strStartsWith(str, "setoption"))
            uciSetOption(str, &threads, &multiPV, &chess960);

        else if (strStartsWith(str, "position"))
            uciPosition(str, &board, chess960);

        else if (strStartsWith(str, "go"))
            uciGo(&uciGoStruct, &pthreadsgo, threads, &board, multiPV, str);

        else if (strEquals(str, "ponderhit"))
            IS_PONDERING = 0;

        else if (strEquals(str, "stop"))
            ABORT_SIGNAL = 1, IS_PONDERING = 0;

        else if (strEquals(str, "quit"))
            break;

        else if (strStartsWith(str, "perft"))
            printf("%"PRIu64"\n", perft(&board, atoi(str + strlen("perft ")))), fflush(stdout);

        else if (strStartsWith(str, "print"))
            printBoard(&board), fflush(stdout);
    }

    return 0;
}

void uciGo(UCIGoStruct *ucigo, pthread_t *pthread, Thread *threads, Board *board, int multiPV, char *str) {

    /// Parse the entire "go" command in order to fill out a Limits struct, found at ucigo->limits.
    /// After we have processed all of this, we can execute a new search thread, held by *pthread,
    /// and detach it.

    double start = get_real_time();
    double wtime = 0, btime = 0;
    double winc = 0, binc = 0, mtg = -1;

    char moveStr[6];
    char *ptr = strtok(str, " ");

    uint16_t moves[MAX_MOVES];
    int size = genAllLegalMoves(board, moves), idx = 0;

    Limits *limits = &ucigo->limits;
    memset(limits, 0, sizeof(Limits));

    IS_PONDERING = FALSE; // Reset PONDERING every time to be safe

    for (ptr = strtok(NULL, " "); ptr != NULL; ptr = strtok(NULL, " ")) {

        // Parse time control conditions
        if (strEquals(ptr, "wtime"      )) wtime    = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "btime"      )) btime    = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "winc"       )) winc     = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "binc"       )) binc     = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "movestogo"  )) mtg      = atoi(strtok(NULL, " "));

        // Parse special search termination conditions
        if (strEquals(ptr, "depth"      )) limits->depthLimit = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "movetime"   )) limits->timeLimit  = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "nodes"      )) limits->nodeLimit  = atof(strtok(NULL, " "));

        // Parse special search modes
        if (strEquals(ptr, "infinite"   )) limits->limitedByNone  = TRUE;
        if (strEquals(ptr, "searchmoves")) limits->limitedByMoves = TRUE;
        if (strEquals(ptr, "ponder"     )) IS_PONDERING           = TRUE;

        // Parse any specific moves that we are to search
        for (int i = 0; i < size; i++) {
            moveToString(moves[i], moveStr, board->chess960);
            if (strEquals(ptr, moveStr)) limits->searchMoves[idx++] = moves[i];
        }
    }

    // Special exit cases: Time, Depth, and Nodes
    limits->limitedByTime  = limits->timeLimit  != 0;
    limits->limitedByDepth = limits->depthLimit != 0;
    limits->limitedByNodes = limits->nodeLimit  != 0;

    // No special case nor infinite, so we set our own time
    limits->limitedBySelf  = !limits->depthLimit    && !limits->timeLimit
                          && !limits->limitedByNone && !limits->nodeLimit;

    // Pick the time values for the colour we are playing as
    limits->start = (board->turn == WHITE) ? start : start;
    limits->time  = (board->turn == WHITE) ? wtime : btime;
    limits->inc   = (board->turn == WHITE) ?  winc :  binc;
    limits->mtg   = (board->turn == WHITE) ?   mtg :   mtg;

    // Cap our MultiPV search based on the suggested or legal moves
    limits->multiPV = MIN(multiPV, limits->limitedByMoves ? idx : size);

    // Prepare the uciGoStruct for the new pthread
    ucigo->board   = board;
    ucigo->threads = threads;

    // Spawn a new thread to handle the search
    pthread_create(pthread, NULL, &start_search_threads, ucigo);
    pthread_detach(*pthread);
}

void uciSetOption(char *str, Thread **threads, int *multiPV, int *chess960) {

    // Handle setting UCI options in Ethereal. Options include:
    //  Hash                : Size of the Transposition Table in Megabyes
    //  Threads             : Number of search threads to use
    //  EvalFile            : Network weights for Ethereal's NNUE evaluation
    //  MultiPV             : Number of search lines to report per iteration
    //  MoveOverhead        : Overhead on time allocation to avoid time losses
    //  SyzygyPath          : Path to Syzygy Tablebases
    //  SyzygyProbeDepth    : Minimal Depth to probe the highest cardinality Tablebase
    //  Normalize           : Normalize UCI output to hope that +1.00 is 50% Won, 50% Drawn
    //  UCI_Chess960        : Set when playing FRC, but not required in order to work

    if (strStartsWith(str, "setoption name Hash value ")) {
        int megabytes = atoi(str + strlen("setoption name Hash value "));
        printf("info string set Hash to %dMB\n", tt_init((*threads)->nthreads, megabytes));
    }

    if (strStartsWith(str, "setoption name Threads value ")) {
        int nthreads = atoi(str + strlen("setoption name Threads value "));
        deleteThreadPool(*threads); *threads = createThreadPool(nthreads);
        printf("info string set Threads to %d\n", nthreads);
    }

    if (strStartsWith(str, "setoption name EvalFile value ")) {
        char *ptr = str + strlen("setoption name EvalFile value ");
        if (!strStartsWith(ptr, "<empty>")) nnue_init(ptr);
        printf("info string set EvalFile to %s\n", ptr);
    }

    if (strStartsWith(str, "setoption name MultiPV value ")) {
        *multiPV = atoi(str + strlen("setoption name MultiPV value "));
        printf("info string set MultiPV to %d\n", *multiPV);
    }

    if (strStartsWith(str, "setoption name MoveOverhead value ")) {
        MoveOverhead = atoi(str + strlen("setoption name MoveOverhead value "));
        printf("info string set MoveOverhead to %d\n", MoveOverhead);
    }

    if (strStartsWith(str, "setoption name SyzygyPath value ")) {
        char *ptr = str + strlen("setoption name SyzygyPath value ");
        if (!strStartsWith(ptr, "<empty>")) tb_init(ptr);
        printf("info string set SyzygyPath to %s\n", ptr);
    }

    if (strStartsWith(str, "setoption name SyzygyProbeDepth value ")) {
        TB_PROBE_DEPTH = atoi(str + strlen("setoption name SyzygyProbeDepth value "));
        printf("info string set SyzygyProbeDepth to %u\n", TB_PROBE_DEPTH);
    }

    if (strStartsWith(str, "setoption name Normalize value ")) {
        if (strStartsWith(str, "setoption name Normalize value true"))
            printf("info string set Normalize to true\n"), NORMALIZE_EVAL = 1;
        if (strStartsWith(str, "setoption name Normalize value false"))
            printf("info string set Normalize to false\n"), NORMALIZE_EVAL = 0;
    }

    if (strStartsWith(str, "setoption name UCI_Chess960 value ")) {
        if (strStartsWith(str, "setoption name UCI_Chess960 value true"))
            printf("info string set UCI_Chess960 to true\n"), *chess960 = 1;
        if (strStartsWith(str, "setoption name UCI_Chess960 value false"))
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
    if (strContains(str, "fen"))
        boardFromFEN(board, strstr(str, "fen") + strlen("fen "), chess960);

    // Position is simply the usual starting position
    else if (strContains(str, "startpos"))
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
        size = genAllLegalMoves(board, moves);

        // Find and apply the given move
        for (int i = 0; i < size; i++) {
            moveToString(moves[i], testStr, board->chess960);
            if (strEquals(moveStr, testStr)) {
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


void uciReport(Thread *threads, PVariation *pv, int alpha, int beta) {

    // Gather all of the statistics that the UCI protocol would be
    // interested in. Also, bound the value passed by alpha and
    // beta, since Ethereal uses a mix of fail-hard and fail-soft

    int hashfull    = tt_hashfull();
    int depth       = threads->depth;
    int seldepth    = threads->seldepth;
    int multiPV     = threads->multiPV + 1;
    int elapsed     = elapsed_time(threads->tm);
    int bounded     = MAX(alpha, MIN(pv->score, beta));
    uint64_t nodes  = nodesSearchedThreadPool(threads);
    uint64_t tbhits = tbhitsThreadPool(threads);
    int nps         = (int)(1000 * (nodes / (1 + elapsed)));

    // If the score is MATE or MATED in X, convert to X
    int score = bounded >=  MATE_IN_MAX ?  (MATE - bounded + 1) / 2
              : bounded <= -MATE_IN_MAX ? -(bounded + MATE)     / 2
              : NORMALIZE_EVAL ? 100 * bounded / 186 : bounded;

    // Two possible score types, mate and cp = centipawns
    char *type  = abs(bounded) >= MATE_IN_MAX ? "mate" : "cp";

    // Partial results from a windowed search have bounds
    char *bound = bounded >=  beta ? " lowerbound "
                : bounded <= alpha ? " upperbound " : " ";

    printf("info depth %d seldepth %d multipv %d score %s %d%stime %d "
           "nodes %"PRIu64" nps %d tbhits %"PRIu64" hashfull %d pv ",
           depth, seldepth, multiPV, type, score, bound, elapsed, nodes, nps, tbhits, hashfull);

    // Iterate over the PV and print each move
    for (int i = 0; i < pv->length; i++) {
        char moveStr[6];
        moveToString(pv->line[i], moveStr, threads->board.chess960);
        printf("%s ", moveStr);
    }

    // Send out a newline and flush
    puts(""); fflush(stdout);
}

void uciReportCurrentMove(Board *board, uint16_t move, int currmove, int depth) {

    char moveStr[6];
    moveToString(move, moveStr, board->chess960);
    printf("info depth %d currmove %s currmovenumber %d\n", depth, moveStr, currmove);
    fflush(stdout);

}


int strEquals(char *str1, char *str2) {
    return strcmp(str1, str2) == 0;
}

int strStartsWith(char *str, char *key) {
    return strstr(str, key) == str;
}

int strContains(char *str, char *key) {
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
