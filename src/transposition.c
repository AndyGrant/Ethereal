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
#include <string.h>

#include <sys/mman.h>

#include "transposition.h"
#include "types.h"

TTable Table; // Global Transposition Table

void initTT(uint64_t megabytes) {

    uint64_t keySize = 16ull;
    size_t hashSize;

    // Cleanup memory when resizing the table
    if (Table.hashMask) free(Table.buckets);

    // The smallest TT size we allow is 1MB, which matches up with
    // a TT using a 15 bit lookup key. We start the key at 16, because
    // while adjusting the given size to the nearest power of two less
    // than or equal to the size, we end with a decrement to the key
    // size. The formula works under the assumption that a TTBucket is
    // exactly 32 bytes. We assure this in order to get good caching

    for (;1ull << (keySize + 5) <= megabytes << 20 ; keySize++);
    assert(sizeof(TTBucket) == 32);
    keySize = keySize - 1;

    // Allocate the TTBuckets and save the lookup mask
    Table.hashMask = (1ull << keySize) - 1u;

    hashSize = sizeof(TTBucket) * (1ull << keySize);

    // allocate hash with 2M align. This is for huge pages
    Table.buckets = aligned_alloc(1ull << 21, hashSize);

#ifdef MADV_HUGEPAGE
    // Linux-specific call to request huge pages, in case aligned_alloc() call
    // above doesn't already trigger them (depends on transparent huge page
    // settings)
    madvise(Table.buckets, hashSize, MADV_HUGEPAGE);
#endif
    // Tell kernel that we need the memory. This should improve clear speed a
    // bit.
    madvise(Table.buckets, hashSize, MADV_WILLNEED);

    clearTT(); // Clear the table and load everything into the cache
}

void updateTT() {

    // The two LSBs are used for storing the entry bound
    // types, and the six MSBs are for storing the entry
    // age. Therefore add TT_MASK_BOUND + 1 to increment

    Table.generation += TT_MASK_BOUND + 1;
    assert(!(Table.generation & TT_MASK_BOUND));

}

void clearTT() {

    // Wipe the Table in preperation for a new game. The
    // Hash Mask is known to be one less than the size

    memset(Table.buckets, 0, sizeof(TTBucket) * (Table.hashMask + 1u));
}

void lockTTBucket(TTBucket *bucket) {

#ifndef NO_TT_LOCKS
    uint8_t previous;
    unsigned iteration = 0;

    while (1) {
        previous =
            atomic_exchange_explicit(
                &bucket->lock,
                1, // desired
                memory_order_acquire);

        if (previous == 0)
            return;

        // didn't get the lock, sleep a bit
        // amounts: 50, 100, 200, ..., 6400, 12800, 12800, ... (max sleep)
        uint32_t sleepAmount = 50 << iteration;

        // volatile here should prevent the compiler from optimizing this out
        for (volatile uint32_t c = 0; c < sleepAmount; ++c) ;

        if (iteration < 8)
            ++iteration;
    }
#else
    (void)bucket;
#endif
}

void unlockTTBucket(TTBucket *bucket) {

#ifndef NO_TT_LOCKS
    atomic_store_explicit(&bucket->lock, 0, memory_order_release);
#else
    (void)bucket;
#endif
}

int hashfullTT() {

    // Take a sample of the first thousand buckets in the table
    // in order to estimate the permill of the table that is in
    // use for the most recent search. We do this, instead of
    // tracking this while probing in order to avoid sharing
    // memory between the search threads.

    int used = 0;

    for (int i = 0; i < 1000; i++)
        for (int j = 0; j < TT_BUCKET_NB; j++)
            used += (Table.buckets[i].slots[j].generation & TT_MASK_BOUND) != BOUND_NONE
                 && (Table.buckets[i].slots[j].generation & TT_MASK_AGE) == Table.generation;

    return used / TT_BUCKET_NB;
}

int valueFromTT(int value, int height) {

    // When probing MATE scores into the table
    // we must factor in the search height

    return value >=  MATE_IN_MAX ? value - height
         : value <= MATED_IN_MAX ? value + height : value;
}

int valueToTT(int value, int height) {

    // When storing MATE scores into the table
    // we must factor in the search height

    return value >=  MATE_IN_MAX ? value + height
         : value <= MATED_IN_MAX ? value - height : value;
}

void prefetchTTEntry(uint64_t hash)
{
    TTBucket *bucket = &Table.buckets[hash & Table.hashMask];
    __builtin_prefetch(bucket);
}

int getTTEntry(uint64_t hash, uint16_t *move, int *value, int *eval, int *depth, int *bound) {

    const uint16_t hash16 = hash >> 48;
    TTBucket *bucket = &Table.buckets[hash & Table.hashMask];
    TTEntry *slots = bucket->slots;
    uint16_t *hashes = bucket->hashes;

    lockTTBucket(bucket);

    // Search for a matching hash signature
    for (int i = 0; i < TT_BUCKET_NB; i++) {
        if (hashes[i] == hash16) {

            // Update age but retain bound type
            slots[i].generation = Table.generation | (slots[i].generation & TT_MASK_BOUND);

            // Copy over the TTEntry and signal success
            *move  = slots[i].move;
            *value = slots[i].value;
            *eval  = slots[i].eval;
            *depth = slots[i].depth;
            *bound = slots[i].generation & TT_MASK_BOUND;

            unlockTTBucket(bucket);
            return 1;
        }
    }

    unlockTTBucket(bucket);
    return 0;
}

void storeTTEntry(uint64_t hash, uint16_t move, int value, int eval, int depth, int bound) {

    int i;
    const uint16_t hash16 = hash >> 48;

    TTBucket *bucket = &Table.buckets[hash & Table.hashMask];
    TTEntry *slots = bucket->slots;
    uint16_t *hashes = bucket->hashes;

    TTEntry *replace = slots; // &slots[0]
    size_t replaceIndex = 0;

    lockTTBucket(bucket);

    // Find a matching hash, or replace using MAX(x1, x2, x3),
    // where xN equals the depth minus 4 times the age difference
    for (i = 0; i < TT_BUCKET_NB && hashes[i] != hash16; i++)
        if (   replace->depth - ((259 + Table.generation - replace->generation) & TT_MASK_AGE)
            >= slots[i].depth - ((259 + Table.generation - slots[i].generation) & TT_MASK_AGE)) {
            replace = &slots[i];
            replaceIndex = i;
        }

    // Prefer a matching hash, otherwise score a replacement
    if (i != TT_BUCKET_NB) {
        replace = &slots[i];
        replaceIndex = i;
    }

    // Don't overwrite an entry from the same position, unless we have
    // an exact bound or depth that is nearly as good as the old one
    if (   bound != BOUND_EXACT
        && hash16 == hashes[replaceIndex]
        && depth < replace->depth - 3) {

        unlockTTBucket(bucket);
        return;
    }

    // Finally, copy the new data into the replaced slot
    replace->depth      = (int8_t)depth;
    replace->generation = (uint8_t)bound | Table.generation;
    replace->value      = (int16_t)value;
    replace->eval       = (int16_t)eval;
    replace->move       = (uint16_t)move;
    hashes[replaceIndex] = (uint16_t)hash16;

    unlockTTBucket(bucket);
}

PKEntry* getPKEntry(PKTable *pktable, uint64_t pkhash) {
    PKEntry *pkentry = &pktable->entries[pkhash >> PKT_HASH_SHIFT];
    return pkentry->pkhash == pkhash ? pkentry : NULL;
}

void storePKEntry(PKTable *pktable, uint64_t pkhash, uint64_t passed, int eval) {
    PKEntry *pkentry = &pktable->entries[pkhash >> PKT_HASH_SHIFT];
    pkentry->pkhash = pkhash;
    pkentry->passed = passed;
    pkentry->eval   = eval;
}
