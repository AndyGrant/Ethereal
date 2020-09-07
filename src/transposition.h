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

#pragma once

#include <stdint.h>

#include "types.h"

enum {
    BOUND_NONE  = 0,
    BOUND_LOWER = 1,
    BOUND_UPPER = 2,
    BOUND_EXACT = 3,
};

enum {
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

void initTT(uint64_t megabytes);
int hashSizeMBTT();
void updateTT();
void clearTT();
int hashfullTT();
int valueFromTT(int value, int height);
int valueToTT(int value, int height);
void prefetchTTEntry(uint64_t hash);
int getTTEntry(uint64_t hash, uint16_t *move, int *value, int *eval, int *depth, int *bound);
void storeTTEntry(uint64_t hash, uint16_t move, int value, int eval, int depth, int bound);
