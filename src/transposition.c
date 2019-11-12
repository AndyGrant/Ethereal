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

#include "transposition.h"
#include "types.h"

TTable Table; // Global Transposition Table

void initTT(uint64_t megabytes) {

    uint64_t keySize = 16ull;

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
    Table.buckets  = malloc(sizeof(TTBucket) * (1ull << keySize));

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

int getTTEntry(uint64_t hash, uint16_t *move, int *value, int *eval, int *depth, int *bound) {

    const uint16_t hash16 = hash >> 48;
    TTEntry *slots = Table.buckets[hash & Table.hashMask].slots;

    // Search for a matching hash signature
    for (int i = 0; i < TT_BUCKET_NB; i++) {
        if (slots[i].hash16 == hash16) {

            // Update age but retain bound type
            slots[i].generation = Table.generation | (slots[i].generation & TT_MASK_BOUND);

            // Copy over the TTEntry and signal success
            *move  = slots[i].move;
            *value = slots[i].value;
            *eval  = slots[i].eval;
            *depth = slots[i].depth;
            *bound = slots[i].generation & TT_MASK_BOUND;
            return 1;
        }
    }

    return 0;
}

void storeTTEntry(uint64_t hash, uint16_t move, int value, int eval, int depth, int bound) {

    int i;
    const uint16_t hash16 = hash >> 48;
    TTEntry *slots = Table.buckets[hash & Table.hashMask].slots;
    TTEntry *replace = slots; // &slots[0]

    // Find a matching hash, or replace using MAX(x1, x2, x3),
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
        && depth < replace->depth - 3)
        return;

    // Finally, copy the new data into the replaced slot
    replace->depth      = (int8_t)depth;
    replace->generation = (uint8_t)bound | Table.generation;
    replace->value      = (int16_t)value;
    replace->eval       = (int16_t)eval;
    replace->move       = (uint16_t)move;
    replace->hash16     = (uint16_t)hash16;
}

PKEntry* getPKEntry(PKTable *pktable, uint64_t pkhash) {
    PKEntry *pkentry = &pktable->entries[pkhash >> PKT_HASH_SHIFT];
    return pkentry->pkhash == pkhash ? pkentry : NULL;
}

void storePKEntry(PKTable *pktable, uint64_t pkhash, uint64_t passed, int eval, uint8_t closedness) {
    PKEntry *pkentry = &pktable->entries[pkhash >> PKT_HASH_SHIFT];
    pkentry->pkhash = pkhash;
    pkentry->passed = passed;
    pkentry->eval   = eval;
    pkentry->closedness = closedness;
}
