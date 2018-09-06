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
#include "psqt.h"
#include "search.h"
#include "time.h"
#include "thread.h"
#include "uci.h"
#include "transposition.h"
#include "types.h"
#include "move.h"
#include "movegen.h"
#include "zobrist.h"

const char *PieceLabel[COLOUR_NB] = {"PNBRQK", "pnbrqk"};

static const char *Benchmarks[] = {
    #include "bench.csv"
    ""
};

static void clearBoard(Board *board) {
    memset(board, 0, sizeof(*board));
    memset(&board->squares, EMPTY, sizeof(board->squares));
    board->epSquare = -1;
}

static void setSquare(Board *board, int colour, int piece, int sq) {

    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= piece && piece < PIECE_NB);
    assert(0 <= sq && sq < SQUARE_NB);

    board->squares[sq] = makePiece(piece, colour);
    setBit(&board->colours[colour], sq);
    setBit(&board->pieces[piece], sq);

    board->psqtmat += PSQT[board->squares[sq]][sq];
    board->hash ^= ZobristKeys[board->squares[sq]][sq];
    if (piece == PAWN || piece == KING)
        board->pkhash ^= ZobristKeys[board->squares[sq]][sq];
}

static int stringToSquare(const char *str) {

    return str[0] == '-' ? -1 : square(str[1] - '1', str[0] - 'a');
}

void squareToString(int sq, char *str) {

    assert(-1 <= sq && sq < SQUARE_NB);

    if (sq == -1)
        *str++ = '-';
    else {
        *str++ = fileOf(sq) + 'a';
        *str++ = rankOf(sq) + '1';
    }

    *str++ = '\0';
}

void boardFromFEN(Board *board, const char *fen) {

    int sq = 56;
    char ch;
    char *str = strdup(fen), *strPos = NULL;
    char *token = strtok_r(str, " ", &strPos);

    clearBoard(board); // Zero out, set squares to EMPTY

    // Piece placement
    while ((ch = *token++)) {
        if (isdigit(ch))
            sq += ch - '0';
        else if (ch == '/')
            sq -= 16;
        else {
            const bool colour = islower(ch);
            const char *piece = strchr(PieceLabel[colour], ch);

            if (piece)
                setSquare(board, colour, piece - PieceLabel[colour], sq++);
        }
    }

    // Turn of play
    token = strtok_r(NULL, " ", &strPos);
    board->turn = token[0] == 'w' ? WHITE : BLACK;
    if (board->turn == BLACK) board->hash ^= ZobristTurnKey;

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

    board->hash ^= ZobristCastleKeys[board->castleRights];

    // En passant
    board->epSquare = stringToSquare(strtok_r(NULL, " ", &strPos));
    if (board->epSquare != -1)
        board->hash ^= ZobristEnpassKeys[fileOf(board->epSquare)];

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

                *fen++ = PieceLabel[pieceColour(p)][pieceType(p)];
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
    if (!board->castleRights)
        *fen++ = '-';

    // En passant and Fifty move
    squareToString(board->epSquare, str);
    sprintf(fen, " %s %d", str, board->fiftyMoveRule);
}

void printBoard(Board *board) {

    static const char table[COLOUR_NB][PIECE_NB] = {
        {'P','N','B','R','Q','K'},
        {'p','n','b','r','q','k'},
    };

    char fen[256];
    int i, j, file, colour, type;

    // Print each row of the board, starting from the top
    for(i = 56, file = 8; i >= 0; i -= 8, file--){

        printf("\n     |----|----|----|----|----|----|----|----|\n");
        printf("   %d ", file);

        // Print each square in a row, starting from the left
        for(j = 0; j < 8; j++){
            colour = pieceColour(board->squares[i+j]);
            type = pieceType(board->squares[i+j]);

            switch(colour){
                case WHITE: printf("| *%c ", table[colour][type]); break;
                case BLACK: printf("|  %c ", table[colour][type]); break;
                default   : printf("|    "); break;
            }
        }

        printf("|");
    }

    printf("\n     |----|----|----|----|----|----|----|----|");
    printf("\n        A    B    C    D    E    F    G    H\n");

    // Print FEN
    boardToFEN(board, fen);
    printf("\n%s\n\n", fen);
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
    uint16_t bestMove, ponderMove;
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
        getBestMove(threads, &board, &limits, &bestMove, &ponderMove);
        nodes += nodesSearchedThreadPool(threads);

        clearTT(); // Reset TT for new search
    }

    end = getRealTime();

    printf("\n------------------------\n");
    printf("Time  : %dms\n", (int)(end - start));
    printf("Nodes : %"PRIu64"\n", nodes);
    printf("NPS   : %d\n", (int)(nodes / ((end - start) / 1000.0)));
}

int boardIsDrawn(Board *board, int height) {

    // Drawn if any of the three possible cases
    return drawnByFiftyMoveRule(board)
        || drawnByRepetition(board, height)
        || drawnByInsufficientMaterial(board);
}

int drawnByFiftyMoveRule(Board *board) {

    // Fifty move rule triggered. BUG: We do not account for the case
    // when the fifty move rule occurs as checkmate is delivered, which
    // should not be considered a drawn position, but a checkmated one.
    return board->fiftyMoveRule > 99;
}

int drawnByRepetition(Board *board, int height) {

    int reps = 0;

    // Look through hash histories for our moves
    for (int i = board->numMoves - 2; i >= 0; i -= 2) {

        // No draw can occur before a zeroing move
        if (i < board->numMoves - board->fiftyMoveRule)
            break;

        // Check for matching hash with a two fold after the root,
        // or a three fold which occurs in part before the root move
        if (    board->history[i] == board->hash
            && (i > board->numMoves - height || ++reps == 2))
            return 1;
    }

    return 0;
}

int drawnByInsufficientMaterial(Board *board) {

    // No draw by insufficient material with pawns, rooks, or queens
    if (board->pieces[PAWN] | board->pieces[ROOK] | board->pieces[QUEEN])
        return 0;

    // Check for KvK
    if (board->pieces[KING] == (board->colours[WHITE] | board->colours[BLACK]))
        return 1;

    if ((board->colours[WHITE] & board->pieces[KING]) == board->colours[WHITE]){

        // Check for K v KN or K v KB
        if (!several(board->pieces[KNIGHT] | board->pieces[BISHOP]))
            return 1;

        // Check for K v KNN
        if (!board->pieces[BISHOP] && popcount(board->pieces[KNIGHT]) <= 2)
            return 1;
    }

    if ((board->colours[BLACK] & board->pieces[KING]) == board->colours[BLACK]){

        // Check for K v KN or K v KB
        if (!several(board->pieces[KNIGHT] | board->pieces[BISHOP]))
            return 1;

        // Check for K v KNN
        if (!board->pieces[BISHOP] && popcount(board->pieces[KNIGHT]) <= 2)
            return 1;
    }

    return 0;
}
