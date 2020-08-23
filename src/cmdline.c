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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "cmdline.h"
#include "move.h"
#include "search.h"
#include "thread.h"
#include "time.h"
#include "transposition.h"
#include "tuner.h"
#include "uci.h"

void handleCommandLine(int argc, char **argv) {

    // Benchmarker is being run from the command line
    // USAGE: ./Ethereal bench <depth> <threads> <hash>
    if (argc > 1 && strEquals(argv[1], "bench")) {
        runBenchmark(argc, argv);
        exit(EXIT_SUCCESS);
    }

    // Bench is being run from the command line
    // USAGE: ./Ethereal evalbook <book> <depth> <threads> <hash>
    if (argc > 2 && strEquals(argv[1], "evalbook")) {
        runEvalBook(argc, argv);
        exit(EXIT_SUCCESS);
    }

    // Tuner is being run from the command line
    #ifdef TUNE
        runTuner();
        exit(EXIT_SUCCESS);
    #endif
}

void runBenchmark(int argc, char **argv) {

    static const char *Benchmarks[] = {
        #include "bench.csv"
        ""
    };

    Board board;
    Thread *threads;
    Limits limits = {0};

    int scores[256];
    double times[256];
    uint64_t nodes[256];
    uint16_t bestMoves[256];
    uint16_t ponderMoves[256];

    double time;
    uint64_t totalNodes = 0ull;

    int depth     = argc > 2 ? atoi(argv[2]) : 13;
    int nthreads  = argc > 3 ? atoi(argv[3]) :  1;
    int megabytes = argc > 4 ? atoi(argv[4]) : 16;

    initTT(megabytes);
    time = getRealTime();
    threads = createThreadPool(nthreads);

    // Initialize a "go depth <x>" search
    limits.multiPV        = 1;
    limits.limitedByDepth = 1;
    limits.depthLimit     = depth;

    for (int i = 0; strcmp(Benchmarks[i], ""); i++) {

        // Perform the search on the position
        limits.start = getRealTime();
        boardFromFEN(&board, Benchmarks[i], 0);
        getBestMove(threads, &board, &limits, &bestMoves[i], &ponderMoves[i]);

        // Stat collection for later printing
        scores[i] = threads->info->values[depth];
        times[i] = getRealTime() - limits.start;
        nodes[i] = nodesSearchedThreadPool(threads);

        clearTT(); // Reset TT between searches
    }

    printf("\n=================================================================================\n");

    for (int i = 0; strcmp(Benchmarks[i], ""); i++) {

        // Convert moves to typical UCI notation
        char bestStr[6], ponderStr[6];
        moveToString(bestMoves[i], bestStr, 0);
        moveToString(ponderMoves[i], ponderStr, 0);

        // Log all collected information for the current position
        printf("Bench [# %2d] %5d cp  Best:%6s  Ponder:%6s %12d nodes %8d nps\n", i + 1, scores[i],
            bestStr, ponderStr, (int)nodes[i], (int)(1000.0f * nodes[i] / (times[i] + 1)));
    }

    printf("=================================================================================\n");

    // Report the overall statistics
    time = getRealTime() - time;
    for (int i = 0; strcmp(Benchmarks[i], ""); i++) totalNodes += nodes[i];
    printf("OVERALL: %53d nodes %8d nps\n", (int)totalNodes, (int)(1000.0f * totalNodes / (time + 1)));

    free(threads);
}

void runEvalBook(int argc, char **argv) {

    Board board;
    char line[256];
    Limits limits = {0};
    uint16_t best, ponder;
    double start = getRealTime();

    FILE *book    = fopen(argv[2], "r");
    int depth     = argc > 3 ? atoi(argv[3]) : 12;
    int nthreads  = argc > 4 ? atoi(argv[4]) :  1;
    int megabytes = argc > 5 ? atoi(argv[5]) :  2;

    Thread *threads = createThreadPool(nthreads);

    limits.multiPV = 1;
    limits.limitedByDepth = 1;
    limits.depthLimit = depth;
    initTT(megabytes);

    while ((fgets(line, 256, book)) != NULL) {
        limits.start = getRealTime();
        boardFromFEN(&board, line, 0);
        getBestMove(threads, &board, &limits, &best, &ponder);
        resetThreadPool(threads); clearTT();
        printf("FEN: %s", line);
    }

    printf("Time %dms\n", (int)(getRealTime() - start));
}