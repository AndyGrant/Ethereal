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


#if 1
    #define TUNEABLE
#endif

TUNEABLE float LMRBase    = 0.75;
TUNEABLE float LMRDivisor = 2.25;

TUNEABLE float LMPNonImpBase   = 2.50;
TUNEABLE float LMRNonImpFactor = 2 / 4.5;
TUNEABLE float LMPImpBase      = 4.00;
TUNEABLE float LMRImpFactor    = 4 / 4.5;

TUNEABLE int LMPDepth = 8;

TUNEABLE int WindowDepth   = 5;
TUNEABLE int WindowSize    = 10;
TUNEABLE int WindowTimerMS = 2500;

TUNEABLE int CurrmoveTimerMS = 2500;

TUNEABLE int TTResearchMargin = 128;

TUNEABLE int BetaPruningDepth = 8;
TUNEABLE int BetaMargin = 75;

TUNEABLE int AlphaPruningDepth = 5;
TUNEABLE int AlphaMargin = 3000;

TUNEABLE int NullMovePruningDepth = 2;
TUNEABLE int NMPBase              = 4;
TUNEABLE int NMPDepthDivisor      = 6;
TUNEABLE int NMPEvalCap           = 3;
TUNEABLE int NMPEvalDivisor       = 200;

TUNEABLE int ProbCutDepth = 5;
TUNEABLE int ProbCutMargin = 100;

TUNEABLE int IIRDepth = 7;

TUNEABLE int SingularDepth = 8;
TUNEABLE int SingularTTDepth = 3;
TUNEABLE int SingularDoubleMargin = 15;

TUNEABLE int FutilityPruningDepth = 8;
TUNEABLE int FutilityMarginBase = 92;
TUNEABLE int FutilityMarginPerDepth = 59;
TUNEABLE int FutilityMarginNoHistory = 158;
TUNEABLE int FutilityPruningHistoryLimit[] = { 12000, 6000 };

TUNEABLE int ContinuationPruningDepth[] = { 3, 2 };
TUNEABLE int ContinuationPruningHistoryLimit[] = { -1000, -2500 };

TUNEABLE int LateMovePruningDepth = 8;

TUNEABLE int LMRHistoryCap = 2;
TUNEABLE int LMRHistoryDivisor = 5000;
TUNEABLE int LMRCaptureHistoryDivisor = 5000;
TUNEABLE int LMRCaptureBase = 2;

TUNEABLE int SEEPruningDepth = 9;
TUNEABLE int SEEQuietMargin = -64;
TUNEABLE int SEENoisyMargin = -19;
TUNEABLE int SEEPieceValues[] = { 100,  450,  450,  675, 1300,    0,    0,    0 };

TUNEABLE int QSSeeMargin = 110;
TUNEABLE int QSDeltaMargin = 150;



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
            printf("option name AnalysisMode type check default false\n");
            printf("option name UCI_Chess960 type check default false\n");

            printf("option name LMRBase type string default 0.75\n");
            printf("option name LMRDivisor type string default 2.25\n");
            printf("option name LMPNonImpBase type string default 2.5\n");
            printf("option name LMRNonImpFactor type string default 0.444\n");
            printf("option name LMPImpBase type string default 4.0\n");
            printf("option name LMRImpFactor type string default 0.888\n");
            printf("option name LMPDepth type string default 8\n");
            printf("option name WindowDepth type string default 5\n");
            printf("option name WindowSize type string default 10\n");
            printf("option name WindowTimerMS type string default 2500\n");
            printf("option name CurrmoveTimerMS type string default 2500\n");
            printf("option name TTResearchMargin type string default 128\n");
            printf("option name BetaPruningDepth type string default 8\n");
            printf("option name BetaMargin type string default 75\n");
            printf("option name AlphaPruningDepth type string default 5\n");
            printf("option name AlphaMargin type string default 3000\n");
            printf("option name NullMovePruningDepth type string default 2\n");
            printf("option name NMPBase type string default 4\n");
            printf("option name NMPDepthDivisor type string default 6\n");
            printf("option name NMPEvalCap type string default 3\n");
            printf("option name NMPEvalDivisor type string default 200\n");
            printf("option name ProbCutDepth type string default 5\n");
            printf("option name ProbCutMargin type string default 100\n");
            printf("option name IIRDepth type string default 7\n");
            printf("option name SingularDepth type string default 8\n");
            printf("option name SingularTTDepth type string default 3\n");
            printf("option name SingularDoubleMargin type string default 15\n");
            printf("option name FutilityPruningDepth type string default 8\n");
            printf("option name FutilityMarginBase type string default 92\n");
            printf("option name FutilityMarginPerDepth type string default 59\n");
            printf("option name FutilityMarginNoHistory type string default 158\n");
            printf("option name FutilityPruningHistoryLimit_0 type string default 12000\n");
            printf("option name FutilityPruningHistoryLimit_1 type string default 6000\n");
            printf("option name ContinuationPruningDepth_0 type string default 3\n");
            printf("option name ContinuationPruningDepth_1 type string default 2\n");
            printf("option name ContinuationPruningHistoryLimit_0 type string default -1000\n");
            printf("option name ContinuationPruningHistoryLimit_1 type string default -2500\n");
            printf("option name LateMovePruningDepth type string default 8\n");
            printf("option name LMRHistoryCap type string default 2\n");
            printf("option name LMRHistoryDivisor type string default 5000\n");
            printf("option name LMRCaptureHistoryDivisor type string default 5000\n");
            printf("option name LMRCaptureBase type string default 2\n");
            printf("option name SEEPruningDepth type string default 9\n");
            printf("option name SEEQuietMargin type string default -64\n");
            printf("option name SEENoisyMargin type string default -19\n");
            printf("option name SEEPieceValues_0 type string default 100\n");
            printf("option name SEEPieceValues_1 type string default 450\n");
            printf("option name SEEPieceValues_2 type string default 450\n");
            printf("option name SEEPieceValues_3 type string default 675\n");
            printf("option name SEEPieceValues_4 type string default 1300\n");
            printf("option name SEEPieceValues_5 type string default 0\n");
            printf("option name SEEPieceValues_6 type string default 0\n");
            printf("option name SEEPieceValues_7 type string default 0\n");
            printf("option name QSSeeMargin type string default 110\n");
            printf("option name QSDeltaMargin type string default 150\n");

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

    if (strStartsWith(str, "setoption name UCI_Chess960 value ")) {
        if (strStartsWith(str, "setoption name UCI_Chess960 value true"))
            printf("info string set UCI_Chess960 to true\n"), *chess960 = 1;
        if (strStartsWith(str, "setoption name UCI_Chess960 value false"))
            printf("info string set UCI_Chess960 to false\n"), *chess960 = 0;
    }

    if (strStartsWith(str, "setoption name LMRBase value ")) {
        char *ptr = str + strlen("setoption name LMRBase value ");
        LMRBase = atof(ptr);
    }

    if (strStartsWith(str, "setoption name LMRDivisor value ")) {
        char *ptr = str + strlen("setoption name LMRDivisor value ");
        LMRDivisor = atof(ptr);
    }

    if (strStartsWith(str, "setoption name LMPNonImpBase value ")) {
        char *ptr = str + strlen("setoption name LMPNonImpBase value ");
        LMPNonImpBase = atof(ptr);
    }

    if (strStartsWith(str, "setoption name LMRNonImpFactor value ")) {
        char *ptr = str + strlen("setoption name LMRNonImpFactor value ");
        LMRNonImpFactor = atof(ptr);
    }

    if (strStartsWith(str, "setoption name LMPImpBase value ")) {
        char *ptr = str + strlen("setoption name LMPImpBase value ");
        LMPImpBase = atof(ptr);
    }

    if (strStartsWith(str, "setoption name LMRImpFactor value ")) {
        char *ptr = str + strlen("setoption name LMRImpFactor value ");
        LMRImpFactor = atof(ptr);
    }

    if (strStartsWith(str, "setoption name LMPDepth value ")) {
        char *ptr = str + strlen("setoption name LMPDepth value ");
        LMPDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name WindowDepth value ")) {
        char *ptr = str + strlen("setoption name WindowDepth value ");
        WindowDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name WindowSize value ")) {
        char *ptr = str + strlen("setoption name WindowSize value ");
        WindowSize = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name WindowTimerMS value ")) {
        char *ptr = str + strlen("setoption name WindowTimerMS value ");
        WindowTimerMS = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name CurrmoveTimerMS value ")) {
        char *ptr = str + strlen("setoption name CurrmoveTimerMS value ");
        CurrmoveTimerMS = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name TTResearchMargin value ")) {
        char *ptr = str + strlen("setoption name TTResearchMargin value ");
        TTResearchMargin = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name BetaPruningDepth value ")) {
        char *ptr = str + strlen("setoption name BetaPruningDepth value ");
        BetaPruningDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name BetaMargin value ")) {
        char *ptr = str + strlen("setoption name BetaMargin value ");
        BetaMargin = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name AlphaPruningDepth value ")) {
        char *ptr = str + strlen("setoption name AlphaPruningDepth value ");
        AlphaPruningDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name AlphaMargin value ")) {
        char *ptr = str + strlen("setoption name AlphaMargin value ");
        AlphaMargin = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name NullMovePruningDepth value ")) {
        char *ptr = str + strlen("setoption name NullMovePruningDepth value ");
        NullMovePruningDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name NMPBase value ")) {
        char *ptr = str + strlen("setoption name NMPBase value ");
        NMPBase = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name NMPDepthDivisor value ")) {
        char *ptr = str + strlen("setoption name NMPDepthDivisor value ");
        NMPDepthDivisor = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name NMPEvalCap value ")) {
        char *ptr = str + strlen("setoption name NMPEvalCap value ");
        NMPEvalCap = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name NMPEvalDivisor value ")) {
        char *ptr = str + strlen("setoption name NMPEvalDivisor value ");
        NMPEvalDivisor = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name ProbCutDepth value ")) {
        char *ptr = str + strlen("setoption name ProbCutDepth value ");
        ProbCutDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name ProbCutMargin value ")) {
        char *ptr = str + strlen("setoption name ProbCutMargin value ");
        ProbCutMargin = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name IIRDepth value ")) {
        char *ptr = str + strlen("setoption name IIRDepth value ");
        IIRDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SingularDepth value ")) {
        char *ptr = str + strlen("setoption name SingularDepth value ");
        SingularDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SingularTTDepth value ")) {
        char *ptr = str + strlen("setoption name SingularTTDepth value ");
        SingularTTDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SingularDoubleMargin value ")) {
        char *ptr = str + strlen("setoption name SingularDoubleMargin value ");
        SingularDoubleMargin = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name FutilityPruningDepth value ")) {
        char *ptr = str + strlen("setoption name FutilityPruningDepth value ");
        FutilityPruningDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name FutilityMarginBase value ")) {
        char *ptr = str + strlen("setoption name FutilityMarginBase value ");
        FutilityMarginBase = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name FutilityMarginPerDepth value ")) {
        char *ptr = str + strlen("setoption name FutilityMarginPerDepth value ");
        FutilityMarginPerDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name FutilityMarginNoHistory value ")) {
        char *ptr = str + strlen("setoption name FutilityMarginNoHistory value ");
        FutilityMarginNoHistory = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name FutilityPruningHistoryLimit_0 value ")) {
        char *ptr = str + strlen("setoption name FutilityPruningHistoryLimit_0 value ");
        FutilityPruningHistoryLimit[0] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name FutilityPruningHistoryLimit_1 value ")) {
        char *ptr = str + strlen("setoption name FutilityPruningHistoryLimit_1 value ");
        FutilityPruningHistoryLimit[1] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name ContinuationPruningDepth_0 value ")) {
        char *ptr = str + strlen("setoption name ContinuationPruningDepth_0 value ");
        ContinuationPruningDepth[0] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name ContinuationPruningDepth_1 value ")) {
        char *ptr = str + strlen("setoption name ContinuationPruningDepth_1 value ");
        ContinuationPruningDepth[1] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name ContinuationPruningHistoryLimit_0 value ")) {
        char *ptr = str + strlen("setoption name ContinuationPruningHistoryLimit_0 value ");
        ContinuationPruningHistoryLimit[0] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name ContinuationPruningHistoryLimit_1 value ")) {
        char *ptr = str + strlen("setoption name ContinuationPruningHistoryLimit_1 value ");
        ContinuationPruningHistoryLimit[1] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name LateMovePruningDepth value ")) {
        char *ptr = str + strlen("setoption name LateMovePruningDepth value ");
        LateMovePruningDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name LMRHistoryCap value ")) {
        char *ptr = str + strlen("setoption name LMRHistoryCap value ");
        LMRHistoryCap = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name LMRHistoryDivisor value ")) {
        char *ptr = str + strlen("setoption name LMRHistoryDivisor value ");
        LMRHistoryDivisor = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name LMRCaptureHistoryDivisor value ")) {
        char *ptr = str + strlen("setoption name LMRCaptureHistoryDivisor value ");
        LMRCaptureHistoryDivisor = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name LMRCaptureBase value ")) {
        char *ptr = str + strlen("setoption name LMRCaptureBase value ");
        LMRCaptureBase = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEPruningDepth value ")) {
        char *ptr = str + strlen("setoption name SEEPruningDepth value ");
        SEEPruningDepth = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEQuietMargin value ")) {
        char *ptr = str + strlen("setoption name SEEQuietMargin value ");
        SEEQuietMargin = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEENoisyMargin value ")) {
        char *ptr = str + strlen("setoption name SEENoisyMargin value ");
        SEENoisyMargin = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEPieceValues_0 value ")) {
        char *ptr = str + strlen("setoption name SEEPieceValues_0 value ");
        SEEPieceValues[0] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEPieceValues_1 value ")) {
        char *ptr = str + strlen("setoption name SEEPieceValues_1 value ");
        SEEPieceValues[1] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEPieceValues_2 value ")) {
        char *ptr = str + strlen("setoption name SEEPieceValues_2 value ");
        SEEPieceValues[2] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEPieceValues_3 value ")) {
        char *ptr = str + strlen("setoption name SEEPieceValues_3 value ");
        SEEPieceValues[3] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEPieceValues_4 value ")) {
        char *ptr = str + strlen("setoption name SEEPieceValues_4 value ");
        SEEPieceValues[4] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEPieceValues_5 value ")) {
        char *ptr = str + strlen("setoption name SEEPieceValues_5 value ");
        SEEPieceValues[5] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEPieceValues_6 value ")) {
        char *ptr = str + strlen("setoption name SEEPieceValues_6 value ");
        SEEPieceValues[6] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name SEEPieceValues_7 value ")) {
        char *ptr = str + strlen("setoption name SEEPieceValues_7 value ");
        SEEPieceValues[7] = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name QSSeeMargin value ")) {
        char *ptr = str + strlen("setoption name QSSeeMargin value ");
        QSSeeMargin = atoi(ptr);
    }

    if (strStartsWith(str, "setoption name QSDeltaMargin value ")) {
        char *ptr = str + strlen("setoption name QSDeltaMargin value ");
        QSDeltaMargin = atoi(ptr);
    }

    fflush(stdout);

    initSearch();
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
