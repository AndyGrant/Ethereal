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

int tablebasesProbeDTZ(Board *board, uint16_t *best, uint16_t *ponder) {

    int size = 0;
    uint16_t moves[MAX_MOVES];
    unsigned wdl, dtz, to, from, ep, promo;

    // Check to make sure we expect to be within the Syzygy tables
    if (board->castleRooks || popcount(board->colours[WHITE] | board->colours[BLACK]) > (int)TB_LARGEST)
        return 0;

    // Tap into Fathom's API routines
    unsigned result = tb_probe_root(
        board->colours[WHITE],
        board->colours[BLACK],
        board->pieces[KING  ],
        board->pieces[QUEEN ],
        board->pieces[ROOK  ],
        board->pieces[BISHOP],
        board->pieces[KNIGHT],
        board->pieces[PAWN  ],
        board->halfMoveCounter,
        board->castleRooks,
        board->epSquare == -1 ? 0 : board->epSquare,
        board->turn == WHITE ? 1 : 0,
        NULL
    );

    // Probe failed, or we are already in a finished position, in which
    // case someone made a mistake by playing the game further, so we will
    // let Ethereal's main search algorithm handle this as we have always done
    if (   result == TB_RESULT_FAILED
        || result == TB_RESULT_CHECKMATE
        || result == TB_RESULT_STALEMATE)
        return 0;

    // Extract Fathom's score representations
    wdl = TB_GET_WDL(result);
    dtz = TB_GET_DTZ(result);

    // Extract Fathom's move representation
    to    = TB_GET_TO(result);
    from  = TB_GET_FROM(result);
    ep    = TB_GET_EP(result);
    promo = TB_GET_PROMOTES(result);

    // Normal Moves ( Syzygy does not support castling )
    if (ep == 0u && promo == 0u)
        *best = MoveMake(from, to, NORMAL_MOVE);

    // Enpass Moves. Fathom returns a to square, but in Ethereal board->epSquare
    // is not the square of the captured pawn, but the square that the capturing
    // pawn will be moving to. Thus, we ignore Fathom's to value to be safe
    else if (ep != 0u)
        *best = MoveMake(from, board->epSquare, ENPASS_MOVE);

    // Promotion Moves. Fathom has the inverted order of our promotion
    // flags. Thus, four minus the flag converts to our representation.
    // Also, we shift by 14 to actually match the flags we use in Ethereal
    else if (promo != 0u)
        *best = MoveMake(from, to, PROMOTION_MOVE | ((4 - promo) << 14));

    // Unable to read back the move type. Setting the move to NONE_MOVE
    // ensures that we will not illegally return the move to the interface
    else
        *best = NONE_MOVE, assert(0);

    // Verify the legality of the parsed move as a final safety check
    genAllLegalMoves(board, moves, &size);
    for (int i = 0; i < size; i++) {
        if (moves[i] == *best) {
            uciReportTBRoot(board, *best, wdl, dtz);
            *ponder = NONE_MOVE;
            return 1;
        }
    }

    // Something went wrong, but as long as we pretend
    // we failed the probe then nothing is going to break
    assert(0); return 0;
}
