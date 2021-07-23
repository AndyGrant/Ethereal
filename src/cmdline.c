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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboards.h"
#include "board.h"
#include "cmdline.h"
#include "move.h"
#include "pgn.h"
#include "search.h"
#include "thread.h"
#include "time.h"
#include "transposition.h"
#include "tuner.h"
#include "uci.h"

#include "nnue/nnue.h"

typedef struct PSQBBSample {
    uint64_t occupied;   // 8-byte occupancy bitboard ( All Pieces )
    int16_t  eval;       // 2-byte int for the target evaluation
    uint8_t  result;     // 1-byte int for result. { L=0, D=1, W=2 }
    uint8_t  packed[16]; // 1-byte int per two pieces
} PSQBBSample;

typedef struct HalfKPSample {
    uint64_t occupied;   // 8-byte occupancy bitboard ( No Kings )
    int16_t  eval;       // 2-byte int for the target evaluation
    uint8_t  result;     // 1-byte int for result. { L=0, D=1, W=2 }
    uint8_t  turn;       // 1-byte int for the side-to-move flag
    uint8_t  wking;      // 1-byte int for the White King Square
    uint8_t  bking;      // 1-byte int for the Black King Square
    uint8_t  packed[15]; // 1-byte int per two non-King pieces
} HalfKPSample;

static void packBitboard(uint8_t *packed, Board* board, uint64_t pieces) {

    #define encode_piece(p) (8 * pieceColour(p) + pieceType(p))
    #define pack_pieces(p1, p2) (((p1) << 4) | (p2))

    uint8_t types[32] = {0};
    int N = (1 + popcount(pieces)) / 2;

    for (int i = 0; pieces; i++) {
        int sq = poplsb(&pieces);
        types[i] = encode_piece(board->squares[sq]);
    }

    for (int i = 0; i < N; i++)
        packed[i] = pack_pieces(types[i*2], types[i*2+1]);

    #undef encode_piece
    #undef pack_pieces
}


static void runBenchmark(int argc, char **argv) {

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

    if (argc > 5) {
        nnue_init(argv[5]);
        printf("info string set EvalFile to %s\n", argv[5]);
    }

    init_TT(megabytes);
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
        getBestMove(threads, &board, &limits, &bestMoves[i], &ponderMoves[i], &scores[i]);

        // Stat collection for later printing
        times[i] = getRealTime() - limits.start;
        nodes[i] = nodesSearchedThreadPool(threads);

        clear_TT(); // Reset TT between searches
    }

    printf("\n===============================================================================\n");

    for (int i = 0; strcmp(Benchmarks[i], ""); i++) {

        // Convert moves to typical UCI notation
        char bestStr[6], ponderStr[6];
        moveToString(bestMoves[i], bestStr, 0);
        moveToString(ponderMoves[i], ponderStr, 0);

        // Log all collected information for the current position
        printf("[# %2d] %5d cp  Best:%6s  Ponder:%6s %12d nodes %12d nps\n", i + 1, scores[i],
            bestStr, ponderStr, (int)nodes[i], (int)(1000.0f * nodes[i] / (times[i] + 1)));
    }

    printf("===============================================================================\n");

    // Report the overall statistics
    time = getRealTime() - time;
    for (int i = 0; strcmp(Benchmarks[i], ""); i++) totalNodes += nodes[i];
    printf("OVERALL: %47d nodes %12d nps\n", (int)totalNodes, (int)(1000.0f * totalNodes / (time + 1)));

    deleteThreadPool(threads);
}

static void runEvalBook(int argc, char **argv) {

    int score;
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
    init_TT(megabytes);

    while ((fgets(line, 256, book)) != NULL) {
        limits.start = getRealTime();
        boardFromFEN(&board, line, 0);
        getBestMove(threads, &board, &limits, &best, &ponder, &score);
        resetThreadPool(threads); clear_TT();
        printf("FEN: %s", line);
    }

    printf("Time %dms\n", (int)(getRealTime() - start));
}

static void buildPSQBBBook(int argc, char **argv) {

    (void) argc;

    char line[256];
    uint64_t positions = 0;
    double start = getRealTime(), elapsed;
    FILE *fin = fopen(argv[2], "r");
    FILE *fout = fopen(argv[3], "wb");

    while (fgets(line, 256, fin) != NULL) {

        Board board;
        PSQBBSample sample = {0};
        boardFromFEN(&board, line, 0);

        sample.occupied = board.colours[WHITE] | board.colours[BLACK];
        sample.eval     = atoi(strstr(line, "] ") + strlen("] "));
        sample.result   = strstr(line, "[0.0]") ? 0u : strstr(line, "[0.5]") ? 1u : 2u;
        packBitboard(sample.packed, &board, sample.occupied);

        fwrite(&sample, sizeof(PSQBBSample), 1, fout);

        if (positions++ % (1024 * 1024) == 0) {
            elapsed = (getRealTime() - start) / 1000.0;
            printf("\r[%.3fs] %" PRIu64 " Positions", elapsed, positions);
            fflush(stdout);
        }
    }

    elapsed = (getRealTime() - start) / 1000.0;
    printf("\r[%.3fs] %" PRIu64 " Positions\n", elapsed, positions);

    fclose(fin);
    fclose(fout);
}

static void buildHalfKPBook(int argc, char **argv) {

    (void) argc;

    char line[256];
    uint64_t positions = 0;
    double start = getRealTime(), elapsed;
    FILE *fin = fopen(argv[2], "r");
    FILE *fout = fopen(argv[3], "wb");


    while (fgets(line, 256, fin) != NULL) {

        Board board;
        HalfKPSample sample = {0};
        boardFromFEN(&board, line, 0);

        uint64_t white  = board.colours[WHITE];
        uint64_t black  = board.colours[BLACK];
        uint64_t pieces = (white | black);

        sample.occupied = pieces & ~board.pieces[KING];
        sample.eval     = atoi(strstr(line, "] ") + strlen("] "));
        sample.result   = strstr(line, "[0.0]") ? 0u : strstr(line, "[0.5]") ? 1u : 2u;
        sample.turn     = board.turn;
        sample.wking    = getlsb(white & board.pieces[KING]);
        sample.bking    = getlsb(black & board.pieces[KING]);
        packBitboard(sample.packed, &board, sample.occupied);

        sample.eval   = sample.turn ? -sample.eval : sample.eval;
        sample.result = sample.turn ? 2u - sample.result : sample.result;

        fwrite(&sample, sizeof(HalfKPSample), 1, fout);

        if (positions++ % (1024 * 1024) == 0) {
            elapsed = (getRealTime() - start) / 1000.0;
            printf("\r[%.3fs] %" PRIu64 " Positions", elapsed, positions);
            fflush(stdout);
        }
    }

    elapsed = (getRealTime() - start) / 1000.0;
    printf("\r[%.3fs] %" PRIu64 " Positions\n", elapsed, positions);

    fclose(fin);
    fclose(fout);
}


void handleCommandLine(int argc, char **argv) {

    // Output all the wonderful things we can do from the Command Line
    if (argc > 1 && strEquals(argv[1], "--help")) {
        printf("\nbench       [depth=13] [threads=1] [hash=16] [NNUE=None]");
        printf("\n            Run searches on a set of positions to compute a hash\n");
        printf("\nevalbook    [input-file] [depth=12] [threads=1] [hash=2]");
        printf("\n            Evaluate all positions in a FEN file using various options\n");
        printf("\npsqbb       [input-file] [output-file]");
        printf("\n            Build an nndata file for the NNTrainer with PSQBB Architecture");
        printf("\n            Format: [FEN] [RESULT] [EVAL]. Result = { [0.0], [0.5], [1.0] }\n");
        printf("\nhalfkp      [input-file] [output-file]");
        printf("\n            Build an nndata file for the NNTrainer with HalfKP Architecture");
        printf("\n            Format: [FEN] [RESULT] [EVAL]. Result = { [0.0], [0.5], [1.0] }\n");
        printf("\npgnfen      [input-file]");
        printf("\n            Build an FEN file to be processed later by Ethereal's psqbb or halfkp");
        printf("\n            Format: [FEN] [RESULT] [EVAL]. Result = { [0.0], [0.5], [1.0] }\n");
        exit(EXIT_SUCCESS);
    }

    // Benchmark is being run from the command line
    // USAGE: ./Ethereal bench <depth> <threads> <hash> <evalfile>
    if (argc > 1 && strEquals(argv[1], "bench")) {
        runBenchmark(argc, argv);
        exit(EXIT_SUCCESS);
    }

    // Evaluate all positions in a datafile to a given depth
    // USAGE: ./Ethereal evalbook <book> <depth> <threads> <hash>
    if (argc > 2 && strEquals(argv[1], "evalbook")) {
        runEvalBook(argc, argv);
        exit(EXIT_SUCCESS);
    }

    // Build a .nndata file using the PSQBB Architecture
    // USAGE: ./Ethereal psqbb <input> <output>
    if (argc > 3 && strEquals(argv[1], "psqbb")) {
        buildPSQBBBook(argc, argv);
        exit(EXIT_SUCCESS);
    }

    // Build a .nndata file using the HalfKP Architecture
    // USAGE: ./Ethereal halfkp <input> <output>
    if (argc > 3 && strEquals(argv[1], "halfkp")) {
        buildHalfKPBook(argc, argv);
        exit(EXIT_SUCCESS);
    }

    // Convert a PGN file to a list of FENs with results and evals
    // USAGE: ./Ethereal pgnfen <input>
    if (argc > 2 && strEquals(argv[1], "pgnfen")) {
        process_pgn(argv[2]);
        exit(EXIT_SUCCESS);
    }

    // Tuner is being run from the command line
    #ifdef TUNE
        runTuner();
        exit(EXIT_SUCCESS);
    #endif
}