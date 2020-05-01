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
#include "search.h"
#include "thread.h"
#include "time.h"
#include "transposition.h"
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
        runTexelTuning()
        exit(EXIT_SUCCESS);
    #endif
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