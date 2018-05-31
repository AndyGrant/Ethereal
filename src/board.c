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

const char *PieceLabel[COLOUR_NB] = {"PNBRQK", "pnbrqk"};

// Benchmark positions were chosen randomly, giving a representative sample of real games.
static const char *Benchmarks[] = {
    #include "bench.csv"
    ""
};

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

void squareToString(int s, char *str) {

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

    int s = 56;
    char ch;
    char *str = strdup(fen), *strPos = NULL;
    char *token = strtok_r(str, " ", &strPos);

    clearBoard(board); // Zero out, set squares to EMPTY

    // Piece placement
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
    board->turn = token[0] == 'w' ? WHITE : BLACK;
    if (board->turn == BLACK) board->hash ^= ZorbistKeys[TURN][0];

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

    // Need king attackers for move generation
    board->kingAttackers = attackersToKingSquare(board);

    free(str);
}

void boardToFEN(Board *board, char *fen) {

    char str[3];

    // Piece placement
    for (int r = RANK_NB-1; r >= 0; r--) {
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

    // Castle rights
    if (board->castleRights & WHITE_KING_RIGHTS)
        *fen++ = 'K';
    if (board->castleRights & WHITE_QUEEN_RIGHTS)
        *fen++ = 'Q';
    if (board->castleRights & BLACK_KING_RIGHTS)
        *fen++ = 'k';
    if (board->castleRights & BLACK_QUEEN_RIGHTS)
        *fen++ = 'q';

    // En passant and Fifty move
    squareToString(board->epSquare, str);
    sprintf(fen, " %s %d", str, board->fiftyMoveRule);
}

void printBoard(Board *board) {

    static const char *separator = "  |---|---|---|---|---|---|---|---|\n";

    char fen[256], str[3];
    uint64_t b;

    for (int r = 7; r >= 0; r--) {
        char line[] = "  | . | . | . | . | . | . | . | . |\n";
        line[0] = r + '1';
        printf("%s", separator);

        for (int f = 0; f < FILE_NB; f++) {
            const int s = square(r, f), v = board->squares[s];
            line[4 + 4 * f] = v != EMPTY
                ? PieceLabel[PieceColour(v)][PieceType(v)]
                : s == board->epSquare ? '*' : '.';
        }

        printf("%s", line);
    }

    printf("%s", separator);
    printf("    A   B   C   D   E   F   G   H\n");

    // Print FEN
    boardToFEN(board, fen);
    printf("fen: %s\n", fen);

    // Print any threats to the king
    if ((b = board->kingAttackers)){
        printf("checkers:");

        while (b) {
            const int s = poplsb(&b);
            squareToString(s, str);
            printf(" %s", str);
        }

        printf("\n");
    }
}

uint64_t perft(Board *board, int depth){

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

void runBenchmark(Thread *threads, int depth) {

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
    for (int i = 0; strcmp(Benchmarks[i], ""); i++) {
        printf("\nPosition #%d: %s\n", i + 1, Benchmarks[i]);
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
    printf("NPS   : %d\n", (int)(nodes / ((end - start) / 1000.0)));
}
