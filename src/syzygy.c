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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitboards.h"
#include "board.h"
#include "fathom/tbprobe.h"
#include "move.h"
#include "movegen.h"
#include "types.h"
#include "uci.h"

unsigned TB_PROBE_DEPTH;    // Set by UCI options
extern unsigned TB_LARGEST; // Set by Fathom in tb_init()

static void removeBadWDL(Board *board, uint16_t *move, unsigned result) {

    // Extract Fathom's move representation
    unsigned to    = TB_GET_TO(result);
    unsigned from  = TB_GET_FROM(result);
    unsigned ep    = TB_GET_EP(result);
    unsigned promo = TB_GET_PROMOTES(result);

    // Convert the move notation. Care that Fathom's promotion flags are inverted
    if (ep == 0u && promo == 0u) *move = MoveMake(from, to, NORMAL_MOVE);
    else if (ep != 0u)           *move = MoveMake(from, board->epSquare, ENPASS_MOVE);
    else if (promo != 0u)        *move = MoveMake(from, to, PROMOTION_MOVE | ((4 - promo) << 14));

    char movestr[6]; moveToString(*move, movestr, board->chess960);
    printf("info string ignoring %s as per Syzygy\n", movestr);
    fflush(stdout);
}

void tablebasesProbeDTZ(Board *board, Limits *limits) {

    unsigned results[MAX_MOVES];
    uint64_t white = board->colours[WHITE];
    uint64_t black = board->colours[BLACK];

    // Check to make sure we expect to be within the Syzygy tables
    if (board->castleRooks || popcount(white | black) > (int)TB_LARGEST)
        return;

    // Tap into Fathom's API routines
    unsigned result = tb_probe_root(
        board->colours[WHITE],  board->colours[BLACK],
        board->pieces[KING  ],  board->pieces[QUEEN ],
        board->pieces[ROOK  ],  board->pieces[BISHOP],
        board->pieces[KNIGHT],  board->pieces[PAWN  ],
        board->halfMoveCounter, board->castleRooks,
        board->epSquare == -1 ? 0 : board->epSquare,
        board->turn == WHITE ? 1 : 0,
        results
    );

    // Probe failed, or we are already in a finished position.
    if (   result == TB_RESULT_FAILED
        || result == TB_RESULT_CHECKMATE
        || result == TB_RESULT_STALEMATE)
        return;

    // Search for any bad moves and remove them from Root Moves
    for (int i = 0; i < MAX_MOVES; i++) {

        // Fathom flags the end like this
        if (results[i] == TB_RESULT_FAILED)
            break;

        // Move fails to maintain the ideal WDL outcome
        if (TB_GET_WDL(results[i]) != TB_GET_WDL(result))
            removeBadWDL(board, &limits->excludedMoves[i], results[i]);
    }
}

unsigned tablebasesProbeWDL(Board *board, int depth, int height) {

    // The basic rules for Syzygy assume that the last move was a zero'ing move,
    // there are no potential castling moves, and there is not an enpass square.
    if (board->halfMoveCounter || board->castleRooks || board->epSquare != -1)
        return TB_RESULT_FAILED;

    // Root Nodes cannot take early exits, as we need a best move
    if (height == 0) return TB_RESULT_FAILED;

    // Count the remaining pieces to see if we fall into the scope of the Tables
    int cardinality = popcount(board->colours[WHITE] | board->colours[BLACK]);

    // Check to see if we are within the scope of the Tables. Also check the UCI
    // option TB_PROBE_DEPTH. We only probe when below TB_PROBE_DEPTH or when
    // the cardinality is below TB_LARGEST. The purpose here is that we may set
    // TB_PROBE_DEPTH to reduce hardware latency, however if the cardinality is
    // even lower than TB_LARGEST, we assume the table has already been cached.
    if (    cardinality > (int)TB_LARGEST
        || (cardinality == (int)TB_LARGEST && depth < (int)TB_PROBE_DEPTH))
        return TB_RESULT_FAILED;

    // Tap into Fathom's API, which takes in the board representation, followed
    // by the half-move counter, the castling rights, the enpass square, and the
    // side to move. We verify that the half-move and the castle rights are zero
    // before calling. Additionally, a position with no potential enpass is set
    // as 0 in Fathom but -1 in Ethereal. Fathom sets WHITE=1 and BLACK=0.

    return tb_probe_wdl(
        board->colours[WHITE], board->colours[BLACK],
        board->pieces[KING  ], board->pieces[QUEEN ],
        board->pieces[ROOK  ], board->pieces[BISHOP],
        board->pieces[KNIGHT], board->pieces[PAWN  ],
        0, 0, 0, board->turn == WHITE ? 1 : 0
    );
}
