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

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attacks.h"
#include "bitboards.h"
#include "board.h"
#include "castle.h"
#include "masks.h"
#include "piece.h"
#include "psqt.h"
#include "search.h"
#include "time.h"
#include "thread.h"
#include "uci.h"
#include "transposition.h"
#include "types.h"
#include "move.h"
#include "movegen.h"
#include "zorbist.h"

#define NUM_BENCHMARKS (36)

// Benchmark positions are taken from stockfish/benchmark.cpp. FRC,
// and positions including moves after the FEN have been removed, as
// well as the positions containing stalemate and checkmated boards.
char Benchmarks[NUM_BENCHMARKS][256] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 10",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 11",
    "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
    "r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15",
    "r1bbk1nr/pp3p1p/2n5/1N4p1/2Np1B2/8/PPP2PPP/2KR1B1R w kq - 0 13",
    "r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16",
    "4r1k1/r1q2ppp/ppp2n2/4P3/5Rb1/1N1BQ3/PPP3PP/R5K1 w - - 1 17",
    "2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11",
    "r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16",
    "3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22",
    "r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18",
    "4k2r/1pb2ppp/1p2p3/1R1p4/3P4/2r1PN2/P4PPP/1R4K1 b - - 3 22",
    "3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26",
    "6k1/6p1/6Pp/ppp5/3pn2P/1P3K2/1PP2P2/3N4 b - - 0 1",
    "3b4/5kp1/1p1p1p1p/pP1PpP1P/P1P1P3/3KN3/8/8 w - - 0 1",
    "8/6pk/1p6/8/PP3p1p/5P2/4KP1q/3Q4 w - - 0 1",
    "7k/3p2pp/4q3/8/4Q3/5Kp1/P6b/8 w - - 0 1",
    "8/2p5/8/2kPKp1p/2p4P/2P5/3P4/8 w - - 0 1",
    "8/1p3pp1/7p/5P1P/2k3P1/8/2K2P2/8 w - - 0 1",
    "8/pp2r1k1/2p1p3/3pP2p/1P1P1P1P/P5KR/8/8 w - - 0 1",
    "8/3p4/p1bk3p/Pp6/1Kp1PpPp/2P2P1P/2P5/5B2 b - - 0 1",
    "5k2/7R/4P2p/5K2/p1r2P1p/8/8/8 b - - 0 1",
    "6k1/6p1/P6p/r1N5/5p2/7P/1b3PP1/4R1K1 w - - 0 1",
    "1r3k2/4q3/2Pp3b/3Bp3/2Q2p2/1p1P2P1/1P2KP2/3N4 w - - 0 1",
    "6k1/4pp1p/3p2p1/P1pPb3/R7/1r2P1PP/3B1P2/6K1 w - - 0 1",
    "8/3p3B/5p2/5P2/p7/PP5b/k7/6K1 w - - 0 1",
    "8/8/8/8/5kp1/P7/8/1K1N4 w - - 0 1",
    "8/8/8/5N2/8/p7/8/2NK3k w - - 0 1",
    "8/3k4/8/8/8/4B3/4KB2/2B5 w - - 0 1",
    "8/8/1P6/5pr1/8/4R3/7k/2K5 w - - 0 1",
    "8/2p4P/8/kr6/6R1/8/8/1K6 w - - 0 1",
    "8/8/3P3k/8/1p6/8/1P6/1K3n2 b - - 0 1",
    "8/R7/2q5/8/6k1/8/1P5p/K6R w - - 0 124",
    "6k1/3b3r/1p1p4/p1n2p2/1PPNpP1q/P3Q1p1/1R1RB1P1/5K2 b - - 0 1",
    "r2r1n2/pp2bk2/2p1p2p/3q4/3PN1QP/2P3R1/P4PP1/5RK1 w - - 0 1",
};

static const char *PieceLabel[COLOUR_NB] = {"PNBRQK", "pnbrqk"};

static void clearBoard(Board *board) {
    memset(board, 0, sizeof(*board));
    memset(&board->squares, EMPTY, sizeof(board->squares));
    board->epSquare = -1;
}

static void setSquare(Board *board, int c, int p, int s) {

    assert(0 <= c && c < COLOUR_NB);
    assert(0 <= p && p < PIECE_NB);
    assert(0 <= s && s < SQUARE_NB);

    board->squares[s] = MakePiece(p, c);
    setBit(&board->colours[c], s);
    setBit(&board->pieces[p], s);

    board->hash ^= ZorbistKeys[board->squares[s]][s];
    board->psqtmat += PSQT[board->squares[s]][s];

    if (p == PAWN || p == KING)
        board->pkhash ^= PawnKingKeys[board->squares[s]][s];
}

static int stringToSquare(const char *str) {

    if (str[0] == '-')
        return -1;
    else
        return square(str[1] - '1', str[0] - 'a');
}

static void squareToString(int s, char *str) {

    assert(-1 <= s && s < SQUARE_NB);

    if (s == -1)
        *str++ = '-';
    else {
        *str++ = fileOf(s) + 'a';
        *str++ = rankOf(s) + '1';
    }

    *str++ = '\0';
}

void boardFromFEN(Board *board, const char *fen) {

    clearBoard(board);
    char *str = strdup(fen), *strPos = NULL;
    char *token = strtok_r(str, " ", &strPos);

    // Piece placement
    char ch;
    int s = 56;

    while ((ch = *token++)) {
        if (isdigit(ch))
            s += ch - '0';
        else if (ch == '/')
            s -= 16;
        else {
            const bool c = islower(ch);
            const char *p = strchr(PieceLabel[c], ch);

            if (p)
                setSquare(board, c, p - PieceLabel[c], s++);
        }
    }

    // Turn of play
    token = strtok_r(NULL, " ", &strPos);

    if (token[0] == 'w')
        board->turn = WHITE;
    else {
        board->turn = BLACK;
        board->hash ^= ZorbistKeys[TURN][0];
    }

    // Castling rights
    token = strtok_r(NULL, " ", &strPos);

    while ((ch = *token++)) {
        if (ch =='K')
            board->castleRights |= WHITE_KING_RIGHTS;
        else if (ch == 'Q')
            board->castleRights |= WHITE_QUEEN_RIGHTS;
        else if (ch == 'k')
            board->castleRights |= BLACK_KING_RIGHTS;
        else if (ch == 'q')
            board->castleRights |= BLACK_QUEEN_RIGHTS;
    }

    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];

    // En passant
    board->epSquare = stringToSquare(strtok_r(NULL, " ", &strPos));

    if (board->epSquare != -1)
        board->hash ^= ZorbistKeys[ENPASS][fileOf(board->epSquare)];

    // 50 move counter
    board->fiftyMoveRule = atoi(strtok_r(NULL, " ", &strPos));

    // Move count: ignore and use zero, as we count since root
    board->numMoves = 0;

    board->kingAttackers = attackersToKingSquare(board);

    free(str);
}

void boardToFEN(Board *board, char *fen) {

    // Piece placement
    for (int r = 7; r >= 0; r--) {
        int cnt = 0;

        for (int f = 0; f < FILE_NB; f++) {
            const int s = square(r, f);
            const int p = board->squares[s];

            if (p != EMPTY) {
                if (cnt)
                    *fen++ = cnt + '0';

                *fen++ = PieceLabel[PieceColour(p)][PieceType(p)];
                cnt = 0;
            } else
                cnt++;
        }

        if (cnt)
            *fen++ = cnt + '0';

        *fen++ = r == 0 ? ' ' : '/';
    }

    // Turn of play
    *fen++ = board->turn == WHITE ? 'w' : 'b';
    *fen++ = ' ';

    if (board->castleRights & WHITE_KING_RIGHTS)
        *fen++ = 'K';
    if (board->castleRights & WHITE_QUEEN_RIGHTS)
        *fen++ = 'Q';
    if (board->castleRights & BLACK_KING_RIGHTS)
        *fen++ = 'k';
    if (board->castleRights & BLACK_QUEEN_RIGHTS)
        *fen++ = 'q';

    char str[3];
    squareToString(board->epSquare, str);
    sprintf(fen, " %s %d", str, board->fiftyMoveRule);
}

void printBoard(Board* board) {

    static const char *separator = "  |---|---|---|---|---|---|---|---|";

    for (int r = 7; r >= 0; r--) {
        char line[] = "  | . | . | . | . | . | . | . | . |";
        line[0] = r + '1';
        puts(separator);

        for (int f = 0; f < FILE_NB; f++) {
            const int s = square(r, f), v = board->squares[s];
            line[4 + 4 * f] = v != EMPTY
                ? PieceLabel[PieceColour(v)][PieceType(v)]
                : s == board->epSquare ? '*' : '.';
        }

        puts(line);
    }

    puts(separator);
    puts("    A   B   C   D   E   F   G   H");

    // Print FEN
    char fen[256];
    boardToFEN(board, fen);
    printf("fen: %s\n", fen);

    // Print checkers, if any
    uint64_t b = board->kingAttackers;

    if (b) {
        printf("checkers:");
        char str[3];

        while (b) {
            const int s = poplsb(&b);
            squareToString(s, str);
            printf(" %s", str);
        }

        puts("");
    }
}

uint64_t perft(Board* board, int depth){

    Undo undo[1];
    int size = 0;
    uint64_t found = 0ull;
    uint16_t moves[MAX_MOVES];

    if (depth == 0) return 1ull;

    genAllMoves(board, moves, &size);

    // Recurse on all valid moves
    for(size -= 1; size >= 0; size--){
        applyMove(board, moves[size], undo);
        if (isNotInCheck(board, !board->turn))
            found += perft(board, depth-1);
        revertMove(board, moves[size], undo);
    }

    return found;
}

void runBenchmark(Thread* threads, int depth){

    int i;
    double start, end;
    Board board;
    Limits limits;

    uint64_t nodes = 0ull;

    // Initialize limits for the search
    limits.limitedByNone  = 0;
    limits.limitedByTime  = 0;
    limits.limitedByDepth = 1;
    limits.limitedBySelf  = 0;
    limits.timeLimit      = 0;
    limits.depthLimit     = depth == 0 ? 13 : depth;

    start = getRealTime();

    // Search each benchmark position
    for (i = 0; i < NUM_BENCHMARKS; i++){
        printf("\nPosition [%2d|%2d]\n", i + 1, NUM_BENCHMARKS);
        boardFromFEN(&board, Benchmarks[i]);

        limits.start = getRealTime();
        getBestMove(threads, &board, &limits);
        nodes += nodesSearchedThreadPool(threads);

        clearTT(); // Reset TT for new search
    }

    end = getRealTime();

    printf("\n------------------------\n");
    printf("Time  : %dms\n", (int)(end - start));
    printf("Nodes : %"PRIu64"\n", nodes);
    printf("NPS   : %d\n", (int)(nodes / ((end - start ) / 1000.0)));
    fflush(stdout);
}
