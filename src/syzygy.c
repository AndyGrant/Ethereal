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

#include "bitutils.h"
#include "board.h"
#include "fathom/tbprobe.h"
#include "move.h"
#include "movegen.h"
#include "piece.h"
#include "types.h"
#include "uci.h"


unsigned TB_PROBE_DEPTH; // Set by UCI options

extern unsigned TB_LARGEST; // Set by Fathom in tb_init()


unsigned tablebasesProbeWDL(Board* board, int depth, int height){
    
    // Tap into Fathom's API routines. Fathom checks for empty 
    // castling rights and no enpassant square, so unlike Stockfish
    // we do not have to check those conditions in the main search.
    // But because we trust no one, we will do it here anyway
    
    // Also, worth noting, that Fathom expects a position without
    // an enpassant square to be passed 0, but Ethereal encodes this
    // with -1. Also, Fathom uses true for WHITE, and false for BLACK,
    // where as Ethereal uses false for WHITE, and true for BLACK
    
    // Check for an enpass free position, a position with no castling
    // we have just zeroed on the last move, and we have few enough
    // pieces to be in the table. Finally, if our cardinality is the
    // largest possible for our tables, then only probe if our depth
    // is less than TB_PROBE_DEPTH, to reduce throughput on the HDD
    
    // Also, we avoid probing at the Root because the WDL tables do
    // not return a best move for the PV, but only a known score
    
    int cardinality = popcount(board->colours[WHITE]
                              |board->colours[BLACK]);
                           
    if (    height == 0
        ||  board->epSquare != -1
        ||  board->castleRights != 0
        ||  board->fiftyMoveRule != 0
        ||  cardinality > (int)TB_LARGEST
        || (cardinality == (int)TB_LARGEST && depth < (int)TB_PROBE_DEPTH))
        return TB_RESULT_FAILED;
    
    return tb_probe_wdl(
        board->colours[WHITE],
        board->colours[BLACK],
        board->pieces[KING  ],
        board->pieces[QUEEN ],
        board->pieces[ROOK  ],
        board->pieces[BISHOP],
        board->pieces[KNIGHT],
        board->pieces[PAWN  ],
        board->fiftyMoveRule,
        board->castleRights,
        board->epSquare == -1 ? 0 : board->epSquare,
        board->turn == WHITE ? 1 : 0
    );
}

int tablebasesProbeDTZ(Board* board, uint16_t* move){
    
    int i, size = 0;
    uint16_t moves[MAX_MOVES];
    unsigned wdl, dtz, to, from, ep, promo;
    
    // Check to make sure we expect to be within the Syzygy tables
    if (popcount(board->colours[WHITE] | board->colours[BLACK]) > (int)TB_LARGEST)
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
        board->fiftyMoveRule,
        board->castleRights,
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
        *move = MoveMake(from, to, NORMAL_MOVE);
    
    // Enpass Moves. Fathom returns a to square, but in Ethereal board->epSquare
    // is not the square of the captured pawn, but the square that the capturing
    // pawn will be moving to. Thus, we ignore Fathom's to value to be safe
    else if (ep != 0u)
        *move = MoveMake(from, board->epSquare, ENPASS_MOVE);
    
    // Promotion Moves. Fathom has the inverted order of our promotion
    // flags. Thus, four minus the flag converts to our representation.
    // Also, we shift by 14 to actually match the flags we use in Ethereal
    else if (promo != 0u)
        *move = MoveMake(from, to, PROMOTION_MOVE | ((4 - promo) << 14));
    
    // Verify the legality of the parsed move as a final safety check
    genAllLegalMoves(board, moves, &size);
    for (i = 0; i < size; i++){
        if (moves[i] == *move){
            uciReportTBRoot(*move, wdl, dtz);
            return 1;
        }
    }
    
    // Something went wrong, but as long as we pretend
    // we failed to probe then nothing is going to break
    assert(0); return 0;
}
