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
#include "pyrrhic/tbprobe.h"
#include "history.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "search.h"
#include "thread.h"
#include "time.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"
#include "zobrist.h"

extern int ContemptDrawPenalty;   // Defined by Thread.c
extern int ContemptComplexity;    // Defined by Thread.c
extern int MoveOverhead;          // Defined by Time.c
extern unsigned TB_PROBE_DEPTH;   // Defined by Syzygy.c
extern volatile int ABORT_SIGNAL; // Defined by Search.c
extern volatile int IS_PONDERING; // Defined by Search.c
extern volatile int ANALYSISMODE; // Defined by Search.c

pthread_mutex_t READYLOCK = PTHREAD_MUTEX_INITIALIZER;
const char *StartPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int main(int argc, char **argv) {

    Board board;
    char str[8192];
    Thread *threads;
    pthread_t pthreadsgo;
    UCIGoStruct uciGoStruct;

    int chess960 = 0;
    int multiPV  = 1;

    // Initialize core components of Ethereal
    initAttacks(); initMasks(); initEval();
    initSearch(); initZobrist(); initTT(16);
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
            printf("option name MultiPV type spin default 1 min 1 max 256\n");
            printf("option name ContemptDrawPenalty type spin default 0 min -300 max 300\n");
            printf("option name ContemptComplexity type spin default 0 min -100 max 100\n");
            printf("option name MoveOverhead type spin default 100 min 0 max 10000\n");
            printf("option name SyzygyPath type string default <empty>\n");
            printf("option name SyzygyProbeDepth type spin default 0 min 0 max 127\n");
            printf("option name Ponder type check default false\n");
            printf("option name AnalysisMode type check default false\n");
            printf("option name UCI_Chess960 type check default false\n");
            printf("uciok\n"), fflush(stdout);
        }

        else if (strEquals(str, "isready")) {
            pthread_mutex_lock(&READYLOCK);
            printf("readyok\n"), fflush(stdout);
            pthread_mutex_unlock(&READYLOCK);
        }

        else if (strEquals(str, "ucinewgame")) {
            pthread_mutex_lock(&READYLOCK);
            resetThreadPool(threads), clearTT();
            pthread_mutex_unlock(&READYLOCK);
        }

        else if (strStartsWith(str, "setoption")) {
            pthread_mutex_lock(&READYLOCK);
            uciSetOption(str, &threads, &multiPV, &chess960);
            pthread_mutex_unlock(&READYLOCK);
        }

        else if (strStartsWith(str, "position")) {
            pthread_mutex_lock(&READYLOCK);
            uciPosition(str, &board, chess960);
            pthread_mutex_unlock(&READYLOCK);
        }

        else if (strStartsWith(str, "go")) {
            pthread_mutex_lock(&READYLOCK);
            uciGoStruct.multiPV = multiPV;
            uciGoStruct.board   = &board;
            uciGoStruct.threads = threads;
            strncpy(uciGoStruct.str, str, 512);
            pthread_create(&pthreadsgo, NULL, &uciGo, &uciGoStruct);
            pthread_detach(pthreadsgo);
        }

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

void *uciGo(void *cargo) {

    // Get our starting time as soon as possible
    double start = getRealTime();

    Limits limits = {0};
    uint16_t bestMove, ponderMove;
    char moveStr[6];

    int depth = 0, infinite = 0;
    double wtime = 0, btime = 0, movetime = 0;
    double winc = 0, binc = 0, mtg = -1;

    int multiPV     = ((UCIGoStruct*)cargo)->multiPV;
    char *str       = ((UCIGoStruct*)cargo)->str;
    Board *board    = ((UCIGoStruct*)cargo)->board;
    Thread *threads = ((UCIGoStruct*)cargo)->threads;

    uint16_t moves[MAX_MOVES];
    int size = genAllLegalMoves(board, moves);
    int idx = 0, searchmoves = 0;

    // Reset global signals
    IS_PONDERING = 0;

    // Init the tokenizer with spaces
    char* ptr = strtok(str, " ");

    // Parse any time control and search method information that was sent
    for (ptr = strtok(NULL, " "); ptr != NULL; ptr = strtok(NULL, " ")) {

        if (strEquals(ptr, "wtime"      )) wtime    = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "btime"      )) btime    = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "winc"       )) winc     = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "binc"       )) binc     = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "movestogo"  )) mtg      = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "depth"      )) depth    = atoi(strtok(NULL, " "));
        if (strEquals(ptr, "movetime"   )) movetime = atoi(strtok(NULL, " "));

        if (strEquals(ptr, "infinite"   )) infinite = 1;
        if (strEquals(ptr, "searchmoves")) searchmoves = 1;
        if (strEquals(ptr, "ponder"     )) IS_PONDERING = 1;

        for (int i = 0; i < size; i++) {
            moveToString(moves[i], moveStr, board->chess960);
            if (strEquals(ptr, moveStr)) limits.searchMoves[idx++] = moves[i];
        }
    }

    // Initialize limits for the search
    limits.limitedByNone  = infinite != 0;
    limits.limitedByTime  = movetime != 0;
    limits.limitedByDepth = depth    != 0;
    limits.limitedBySelf  = !depth && !movetime && !infinite;
    limits.limitedByMoves = searchmoves;
    limits.timeLimit      = movetime;
    limits.depthLimit     = depth;

    // Pick the time values for the colour we are playing as
    limits.start = (board->turn == WHITE) ? start : start;
    limits.time  = (board->turn == WHITE) ? wtime : btime;
    limits.inc   = (board->turn == WHITE) ?  winc :  binc;
    limits.mtg   = (board->turn == WHITE) ?   mtg :   mtg;

    // Cap our MultiPV search based on the suggested or legal moves
    limits.multiPV = MIN(multiPV, searchmoves ? idx : size);

    // Execute search, return best and ponder moves
    getBestMove(threads, board, &limits, &bestMove, &ponderMove);

    // UCI spec does not want reports until out of pondering
    while (IS_PONDERING);

    // Report best move ( we should always have one )
    moveToString(bestMove, moveStr, board->chess960);
    printf("bestmove %s ", moveStr);

    // Report ponder move ( if we have one )
    if (ponderMove != NONE_MOVE) {
        moveToString(ponderMove, moveStr, board->chess960);
        printf("ponder %s", moveStr);
    }

    // Make sure this all gets reported
    printf("\n"); fflush(stdout);

    // Drop the ready lock, as we are prepared to handle a new search
    pthread_mutex_unlock(&READYLOCK);

    return NULL;
}

void uciSetOption(char *str, Thread **threads, int *multiPV, int *chess960) {

    // Handle setting UCI options in Ethereal. Options include:
    //  Hash                : Size of the Transposition Table in Megabyes
    //  Threads             : Number of search threads to use
    //  MultiPV             : Number of search lines to report per iteration
    //  ContemptDrawPenalty : Evaluation bonus in internal units to avoid forced draws
    //  ContemptComplexity  : Evaluation bonus for keeping a position with more non-pawn material
    //  MoveOverhead        : Overhead on time allocation to avoid time losses
    //  SyzygyPath          : Path to Syzygy Tablebases
    //  SyzygyProbeDepth    : Minimal Depth to probe the highest cardinality Tablebase
    //  UCI_Chess960        : Set when playing FRC, but not required in order to work

    if (strStartsWith(str, "setoption name Hash value ")) {
        int megabytes = atoi(str + strlen("setoption name Hash value "));
        initTT(megabytes); printf("info string set Hash to %dMB\n", hashSizeMBTT());
    }

    if (strStartsWith(str, "setoption name Threads value ")) {
        int nthreads = atoi(str + strlen("setoption name Threads value "));
        free(*threads); *threads = createThreadPool(nthreads);
        printf("info string set Threads to %d\n", nthreads);
    }

    if (strStartsWith(str, "setoption name MultiPV value ")) {
        *multiPV = atoi(str + strlen("setoption name MultiPV value "));
        printf("info string set MultiPV to %d\n", *multiPV);
    }

    if (strStartsWith(str, "setoption name ContemptDrawPenalty value ")){
        ContemptDrawPenalty = atoi(str + strlen("setoption name ContemptDrawPenalty value "));
        printf("info string set ContemptDrawPenalty to %d\n", ContemptDrawPenalty);
    }

    if (strStartsWith(str, "setoption name ContemptComplexity value ")){
        ContemptComplexity = atoi(str + strlen("setoption name ContemptComplexity value "));
        printf("info string set ContemptComplexity to %d\n", ContemptComplexity);
    }

    if (strStartsWith(str, "setoption name MoveOverhead value ")) {
        MoveOverhead = atoi(str + strlen("setoption name MoveOverhead value "));
        printf("info string set MoveOverhead to %d\n", MoveOverhead);
    }

    if (strStartsWith(str, "setoption name SyzygyPath value ")) {
        char *ptr = str + strlen("setoption name SyzygyPath value ");
        tb_init(ptr); printf("info string set SyzygyPath to %s\n", ptr);
    }

    if (strStartsWith(str, "setoption name SyzygyProbeDepth value ")) {
        TB_PROBE_DEPTH = atoi(str + strlen("setoption name SyzygyProbeDepth value "));
        printf("info string set SyzygyProbeDepth to %u\n", TB_PROBE_DEPTH);
    }

    if (strStartsWith(str, "setoption name AnalysisMode value ")) {
        if (strStartsWith(str, "setoption name AnalysisMode value true"))
            printf("info string set AnalysisMode to true\n"), ANALYSISMODE = 1;
        if (strStartsWith(str, "setoption name AnalysisMode value false"))
            printf("info string set AnalysisMode to false\n"), ANALYSISMODE = 0;
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

void uciReport(Thread *threads, int alpha, int beta, int value) {

    // Gather all of the statistics that the UCI protocol would be
    // interested in. Also, bound the value passed by alpha and
    // beta, since Ethereal uses a mix of fail-hard and fail-soft

    int hashfull    = hashfullTT();
    int depth       = threads->depth;
    int seldepth    = threads->seldepth;
    int multiPV     = threads->multiPV + 1;
    int elapsed     = elapsedTime(threads->info);
    int bounded     = MAX(alpha, MIN(value, beta));
    uint64_t nodes  = nodesSearchedThreadPool(threads);
    uint64_t tbhits = tbhitsThreadPool(threads);
    int nps         = (int)(1000 * (nodes / (1 + elapsed)));

    // If the score is MATE or MATED in X, convert to X
    int score   = bounded >=  MATE_IN_MAX ?  (MATE - bounded + 1) / 2
                : bounded <= -MATE_IN_MAX ? -(bounded + MATE)     / 2 : bounded;

    // Two possible score types, mate and cp = centipawns
    char *type  = abs(bounded) >= MATE_IN_MAX ? "mate" : "cp";

    // Partial results from a windowed search have bounds
    char *bound = bounded >=  beta ? " lowerbound "
                : bounded <= alpha ? " upperbound " : " ";

    printf("info depth %d seldepth %d multipv %d score %s %d%stime %d "
           "nodes %"PRIu64" nps %d tbhits %"PRIu64" hashfull %d pv ",
           depth, seldepth, multiPV, type, score, bound, elapsed, nodes, nps, tbhits, hashfull);

    // Iterate over the PV and print each move
    for (int i = 0; i < threads->pv.length; i++) {
        char moveStr[6];
        moveToString(threads->pv.line[i], moveStr, threads->board.chess960);
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
