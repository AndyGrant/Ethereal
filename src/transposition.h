/******************************************************************************/
/*                                                                            */
/*    Ethereal is a UCI chess playing engine authored by Andrew Grant.        */
/*    <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>        */
/*                                                                            */
/*    Ethereal is free software: you can redistribute it and/or modify        */
/*    it under the terms of the GNU General Public License as published by    */
/*    the Free Software Foundation, either version 3 of the License, or       */
/*    (at your option) any later version.                                     */
/*                                                                            */
/*    Ethereal is distributed in the hope that it will be useful,             */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*    GNU General Public License for more details.                            */
/*                                                                            */
/*    You should have received a copy of the GNU General Public License       */
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>    */
/*                                                                            */
/******************************************************************************/

#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)
    #include <sys/mman.h>
#endif

#include "types.h"

/// The Transposition Table contains information from positions that have been
/// searched previously. Each entry contains a bound, a depth, an age, and some
/// additional Zobrist bits. An Entry may also contain a static evaluation for
/// the node, a search evaluation for the node, and a best move at that node.
///
/// Each Entry contains 10-bytes of information. We group together entries into
/// Buckets, containing three Entries a piece, as well as 2 additional bytes to
/// pad out the structure to 32-bytes. This gives us multiple options when we
/// run into a Zobrist collision with the Transposition Table lookup key.
///
/// Generally, we prefer to replace entries that came from previous searches,
/// as well as those which come from a lower depth. However, sometimes we do
/// not replace any such entry, if it would be too harmful to do so.
///
/// The minimum size of the Transposition Table is 2MB. This is so that we
/// can lookup the table with at least 16-bits, and so that we may align the
/// Table on a 2MB memory boundary, when available via the Operating System.

enum {
    BOUND_NONE  = 0,
    BOUND_LOWER = 1,
    BOUND_UPPER = 2,
    BOUND_EXACT = 3,

    TT_MASK_BOUND = 0x03,
    TT_MASK_AGE   = 0xFC,
    TT_BUCKET_NB  = 3,
};

struct TTEntry {
    int8_t depth;
    uint8_t generation;
    int16_t eval, value;
    uint16_t move, hash16;
};

struct TTBucket {
    TTEntry slots[TT_BUCKET_NB];
    uint16_t padding;
};

struct TTable {
    TTBucket *buckets;
    uint64_t hashMask;
    uint8_t generation;
};

void tt_update();
void tt_prefetch(uint64_t hash);

int tt_init(int nthreads, int megabytes);
int tt_hashfull();
bool tt_probe(uint64_t hash, int height, uint16_t *move, int *value, int *eval, int *depth, int *bound);
void tt_store(uint64_t hash, int height, uint16_t move, int value, int eval, int depth, int bound);

struct TTClear { int index, count; };
void tt_clear(int nthreads);
void *tt_clear_threaded(void *cargo);

/// The Pawn King table contains saved evaluations, and additional Pawn information
/// that is expensive to compute during evaluation. This includes the location of all
/// passed pawns, and Pawn-Shelter / Pawn-Storm scores for use in King Safety evaluation.
///
/// While this table is seldom accessed when using Ethereal NNUE, the table generally has
/// an extremely high, 95%+ hit rate, generating a substantial overall speedup to Ethereal.

enum {
    PK_CACHE_KEY_SIZE = 16,
    PK_CACHE_MASK     = 0xFFFF,
    PK_CACHE_SIZE     = 1 << PK_CACHE_KEY_SIZE,
};

struct PKEntry { uint64_t pkhash, passed; int eval, safetyw, safetyb; };
typedef PKEntry PKTable[PK_CACHE_SIZE];

PKEntry* getCachedPawnKingEval(Thread *thread, const Board *board);
void storeCachedPawnKingEval(Thread *thread, const Board *board, uint64_t passed, int eval, int safety[2]);
