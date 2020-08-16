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
#include "pyrrhic/tbprobe.h"
#include "move.h"
#include "movegen.h"
#include "types.h"
#include "uci.h"

unsigned TB_PROBE_DEPTH;          // Set by UCI options
extern int TB_LARGEST;       // Set by Pyrrhic in tb_init()
extern volatile int ANALYSISMODE; // Defined by Search.c

static uint16_t convertPyrrhicMove(Board *board, unsigned result) {

    // Extract Pyrhic's move representation
    unsigned to    = TB_GET_TO(result);
    unsigned from  = TB_GET_FROM(result);
    unsigned ep    = TB_GET_EP(result);
    unsigned promo = TB_GET_PROMOTES(result);

    // Convert the move notation. Care that Pyrrhic's promotion flags are inverted
    if (ep == 0u && promo == 0u) return MoveMake(from, to, NORMAL_MOVE);
    else if (ep != 0u)           return MoveMake(from, board->epSquare, ENPASS_MOVE);
    else /* if (promo != 0u) */  return MoveMake(from, to, PROMOTION_MOVE | ((4 - promo) << 14));
}

static void removeBadWDL(Board *board, Limits *limits, unsigned result, unsigned *results) {

    // Remove for any moves that fail to maintain the ideal WDL outcome
    for (int i = 0; i < MAX_MOVES && results[i] != TB_RESULT_FAILED; i++) {
        if (TB_GET_WDL(results[i]) != TB_GET_WDL(result)) {
            limits->excludedMoves[i] = convertPyrrhicMove(board, results[i]);
            char movestr[6]; moveToString(limits->excludedMoves[i], movestr, board->chess960);
            printf("info string ignoring %s as per Syzygy\n", movestr);
        }
    }
}

int tablebasesProbeDTZ(Board *board, Limits *limits, uint16_t *best, uint16_t *ponder) {

    unsigned results[MAX_MOVES];
    uint64_t white = board->colours[WHITE];
    uint64_t black = board->colours[BLACK];

    // We cannot probe when there are castling rights, or when
    // we have more pieces than our largest Tablebase has pieces
    if (   board->castleRooks
        || popcount(white | black) > TB_LARGEST)
        return 0;

    // Tap into Pyrrhic's API. Pyrrhic takes the board representation and the
    // fifty move rule counter, followed by the enpass square (0 if none set),
    // and the turn Pyrrhic defines WHITE as 1, and BLACK as 0, which is the
    // opposite of how Ethereal defines them

    unsigned result = tb_probe_root(
        board->colours[WHITE],  board->colours[BLACK],
        board->pieces[KING  ],  board->pieces[QUEEN ],
        board->pieces[ROOK  ],  board->pieces[BISHOP],
        board->pieces[KNIGHT],  board->pieces[PAWN  ],
        board->halfMoveCounter, board->epSquare == -1 ? 0 : board->epSquare,
        board->turn == WHITE ? 1 : 0, results
    );

    // Probe failed, or we are already in a finished position.
    if (   result == TB_RESULT_FAILED
        || result == TB_RESULT_CHECKMATE
        || result == TB_RESULT_STALEMATE)
        return 0;

    // If doing analysis, remove sub-optimal WDL moves
    if (ANALYSISMODE)
        removeBadWDL(board, limits, result, results);

    // Otherwise, set the best move to any which maintains the WDL
    else {
        *best = convertPyrrhicMove(board, result);
        *ponder = NONE_MOVE;
    }

    return !ANALYSISMODE;
}

unsigned tablebasesProbeWDL(Board *board, int depth, int height) {

    uint64_t white = board->colours[WHITE];
    uint64_t black = board->colours[BLACK];

    // Never take a Syzygy Probe in a Root node, in a node with Castling rights,
    // in a node which was not just zero'ed by a Pawn Move or Capture, or in a
    // node which has more pieces than our largest found Tablebase can handle

    if (   height == 0
        || board->castleRooks
        || board->halfMoveCounter
        || popcount(white | black) > TB_LARGEST)
        return TB_RESULT_FAILED;


    // We also will avoid probing beneath the provided TB_PROBE_DEPTH, except
    // for when our board has even fewer pieces than the largest Tablebase is
    // able to handle. Namely, when we have a 7man Tablebase, we will always
    // probe the 6man Tablebase if possible, irregardless of TB_PROBE_DEPTH

    if (   depth < (int) TB_PROBE_DEPTH
        && popcount(white | black) == TB_LARGEST)
        return TB_RESULT_FAILED;


    // Tap into Pyrrhic's API. Pyrrhic takes the board representation, followed
    // by the enpass square (0 if none set), and the turn. Pyrrhic defines WHITE
    // as 1, and BLACK as 0, which is the opposite of how Ethereal defines them

    return tb_probe_wdl(
        board->colours[WHITE], board->colours[BLACK],
        board->pieces[KING  ], board->pieces[QUEEN ],
        board->pieces[ROOK  ], board->pieces[BISHOP],
        board->pieces[KNIGHT], board->pieces[PAWN  ],
        board->epSquare == -1 ? 0 : board->epSquare,
        board->turn == WHITE ? 1 : 0
    );
}
