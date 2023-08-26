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

#include <pthread.h>

#include "board.h"
#include "evaluate.h"
#include "thread.h"
#include "transposition.h"
#include "types.h"
#include "zobrist.h"

TTable Table; // Global Transposition Table

/// Mate and Tablebase scores need to be adjusted relative to the Root
/// when going into the Table and when coming out of the Table. Otherwise,
/// we will not know when we have a "faster" vs "slower" Mate or TB Win/Loss

static int tt_value_from(int value, int height) {
    return value ==  VALUE_NONE   ? VALUE_NONE
         : value >=  TBWIN_IN_MAX ? value - height
         : value <= -TBWIN_IN_MAX ? value + height : value;
}

static int tt_value_to(int value, int height) {
    return value ==  VALUE_NONE   ? VALUE_NONE
         : value >=  TBWIN_IN_MAX ? value + height
         : value <= -TBWIN_IN_MAX ? value - height : value;
}


/// Trivial helper functions to Transposition Table handleing

void tt_update() { Table.generation += TT_MASK_BOUND + 1; }
void tt_prefetch(uint64_t hash) { __builtin_prefetch(&Table.buckets[hash & Table.hashMask]); }


int tt_init(int nthreads, int megabytes) {

    const uint64_t MB = 1ull << 20;
    uint64_t keySize = 16ull;

    // Cleanup memory when resizing the table
    if (Table.hashMask) free(Table.buckets);

    // Default keysize of 16 bits maps to a 2MB TTable
    assert((1ull << 16ull) * sizeof(TTBucket) == 2 * MB);

    // Find the largest keysize that is still within our given megabytes
    while ((1ull << keySize) * sizeof(TTBucket) <= megabytes * MB / 2) keySize++;
    assert((1ull << keySize) * sizeof(TTBucket) <= megabytes * MB);

#if defined(__linux__) && !defined(__ANDROID__)

    // On Linux systems we align on 2MB boundaries and request Huge Pages
    Table.buckets = aligned_alloc(2 * MB, (1ull << keySize) * sizeof(TTBucket));
    madvise(Table.buckets, (1ull << keySize) * sizeof(TTBucket), MADV_HUGEPAGE);
#else

    // Otherwise, we simply allocate as usual and make no requests
    Table.buckets = malloc((1ull << keySize) * sizeof(TTBucket));
#endif

    // Save the lookup mask
    Table.hashMask = (1ull << keySize) - 1u;

    // Clear the table and load everything into the cache
    tt_clear(nthreads);

    // Return the number of MB actually allocated for the TTable
    return ((Table.hashMask + 1) * sizeof(TTBucket)) / MB;
}

int tt_hashfull() {

    /// Estimate the permill of the table being used, by looking at a thousand
    /// Buckets and seeing how many Entries contain a recent Transposition.

    int used = 0;

    for (int i = 0; i < 1000; i++)
        for (int j = 0; j < TT_BUCKET_NB; j++)
            used += (Table.buckets[i].slots[j].generation & TT_MASK_BOUND) != BOUND_NONE
                 && (Table.buckets[i].slots[j].generation & TT_MASK_AGE) == Table.generation;

    return used / TT_BUCKET_NB;
}

bool tt_probe(uint64_t hash, int height, uint16_t *move, int *value, int *eval, int *depth, int *bound) {

    /// Search for a Transposition matching the provided Zobrist Hash. If one is found,
    /// we update its age in order to indicate that it is still relevant, before copying
    /// over its contents and signaling to the caller that an Entry was found.

    const uint16_t hash16 = hash >> 48;
    TTEntry *slots = Table.buckets[hash & Table.hashMask].slots;

    for (int i = 0; i < TT_BUCKET_NB; i++) {

        if (slots[i].hash16 == hash16) {

            slots[i].generation = Table.generation | (slots[i].generation & TT_MASK_BOUND);

            *move  = slots[i].move;
            *value = tt_value_from(slots[i].value, height);
            *eval  = slots[i].eval;
            *depth = slots[i].depth;
            *bound = slots[i].generation & TT_MASK_BOUND;
            return TRUE;
        }
    }

    return FALSE;
}

void tt_store(uint64_t hash, int height, uint16_t move, int value, int eval, int depth, int bound) {

    int i;
    const uint16_t hash16 = hash >> 48;
    TTEntry *slots = Table.buckets[hash & Table.hashMask].slots;
    TTEntry *replace = slots; // &slots[0]

    // Find a matching hash, or replace using MIN(x1, x2, x3),
    // where xN equals the depth minus 4 times the age difference
    for (i = 0; i < TT_BUCKET_NB && slots[i].hash16 != hash16; i++)
        if (   replace->depth - ((259 + Table.generation - replace->generation) & TT_MASK_AGE)
            >= slots[i].depth - ((259 + Table.generation - slots[i].generation) & TT_MASK_AGE))
            replace = &slots[i];

    // Prefer a matching hash, otherwise score a replacement
    replace = (i != TT_BUCKET_NB) ? &slots[i] : replace;

    // Don't overwrite an entry from the same position, unless we have
    // an exact bound or depth that is nearly as good as the old one
    if (   bound != BOUND_EXACT
        && hash16 == replace->hash16
        && depth < replace->depth - 2)
        return;

    // Don't overwrite a move if we don't have a new one
    if (move || hash16 != replace->hash16)
        replace->move = (uint16_t) move;

    // Finally, copy the new data into the replaced slot
    replace->depth      = (int8_t  ) depth;
    replace->generation = (uint8_t ) bound | Table.generation;
    replace->value      = (int16_t ) tt_value_to(value, height);
    replace->eval       = (int16_t ) eval;
    replace->hash16     = (uint16_t) hash16;
}


void tt_clear(int nthreads) {

    // Only use 1/4th of the enabled search Threads
    int nworkers = MAX(1, nthreads / 4);
    pthread_t pthreads[nworkers];
    struct TTClear ttclears[nworkers];

    // Initalize the data passed via a void* in pthread_create()
    for (int i = 0; i < nworkers; i++)
        ttclears[i] = (struct TTClear) { i, nworkers };

    // Launch each of the helper threads to clear their sections
    for (int i = 1; i < nworkers; i++)
        pthread_create(&pthreads[i], NULL, tt_clear_threaded, &ttclears[i]);

    // Reuse this thread for the 0th sections of the Transposition Table
    tt_clear_threaded((void*) &ttclears[0]);

    // Join each of the helper threads after they've cleared their sections
    for (int i = 1; i < nworkers; i++)
        pthread_join(pthreads[i], NULL);
}

void *tt_clear_threaded(void *cargo) {

    const uint64_t MB = 1ull << 20;
    struct TTClear *ttclear = (struct TTClear*) cargo;

    // Logic for dividing the Table taken from Weiss and CFish
    const uint64_t size   = (Table.hashMask + 1) * sizeof(TTBucket);
    const uint64_t slice  = (size + ttclear->count - 1) / ttclear->count;
    const uint64_t blocks = (slice + 2 * MB - 1) / (2 * MB);
    const uint64_t begin  = MIN(size, ttclear->index * blocks * 2 * MB);
    const uint64_t end    = MIN(size, begin + blocks * 2 * MB);

    memset(Table.buckets + begin / sizeof(TTBucket), 0, end - begin);
    return NULL;
}

/// Simple Pawn+King Evaluation Hash Table, which also stores some additional
/// safety information for use in King Safety, when not using NNUE evaluations

PKEntry* getCachedPawnKingEval(Thread *thread, const Board *board) {
    PKEntry *pke = &thread->pktable[board->pkhash & PK_CACHE_MASK];
    return pke->pkhash == board->pkhash ? pke : NULL;
}

void storeCachedPawnKingEval(Thread *thread, const Board *board, uint64_t passed, int eval, int safety[2]) {
    PKEntry *pke = &thread->pktable[board->pkhash & PK_CACHE_MASK];
    *pke = (PKEntry) { board->pkhash, passed, eval, safety[WHITE], safety[BLACK] };
}
