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

#include "attacks.h"
#include "bitboards.h"
#include "masks.h"
#include "movegen.h"
#include "types.h"

uint64_t BitsBetweenMasks[SQUARE_NB][SQUARE_NB];
uint64_t RanksAtOrAboveMasks[COLOUR_NB][RANK_NB];
uint64_t IsolatedPawnMasks[SQUARE_NB];
uint64_t PassedPawnMasks[COLOUR_NB][SQUARE_NB];
uint64_t PawnConnectedMasks[COLOUR_NB][SQUARE_NB];
uint64_t OutpostSquareMasks[COLOUR_NB][SQUARE_NB];
uint64_t OutpostRanks[COLOUR_NB];

void initMasks() {

    for (int s1 = 0; s1 < SQUARE_NB; s1++) {
        for (int s2 = 0; s2 < SQUARE_NB; s2++) {
            // Aligned on a diagonal
            if (testBit(bishopAttacks(s1, 0ull), s2))
                BitsBetweenMasks[s1][s2] = bishopAttacks(s1, 1ull << s2) & bishopAttacks(s2, 1ull << s1);

            // Aligned on a straight
            if (testBit(rookAttacks(s1, 0ull), s2))
                BitsBetweenMasks[s1][s2] = rookAttacks(s1, 1ull << s2) & rookAttacks(s2, 1ull << s1);
        }
    }

    // Initalize ranks above masks
    for (int r = 0; r < RANK_NB; r++) {
        for (int i = r; i < RANK_NB; i++)
            RanksAtOrAboveMasks[WHITE][r] |= Ranks[i];

        for (int i = r; i >= 0; i--)
            RanksAtOrAboveMasks[BLACK][r] |= Ranks[i];
    }

    // Initalize isolated pawn masks
    for (int s = 0; s < SQUARE_NB; s++) {
        const int f = fileOf(s);

        if (f > 0 && f < 7)
            IsolatedPawnMasks[s] = Files[f + 1] | Files[f - 1];
        else if (f > 0)
            IsolatedPawnMasks[s] = Files[f - 1];
        else
            IsolatedPawnMasks[s] = Files[f + 1];
    }

    // Initalize passed pawn masks and outpost masks
    for (int s = 0; s < SQUARE_NB; s++) {
        const uint64_t files = IsolatedPawnMasks[s] | Files[fileOf(s)];

        PassedPawnMasks[WHITE][s] = files;
        for (int r = rankOf(s); r >= 0; r--)
            PassedPawnMasks[WHITE][s] &= ~Ranks[r];

        PassedPawnMasks[BLACK][s] = files;
        for (int r = rankOf(s); r < RANK_NB; r++)
            PassedPawnMasks[BLACK][s] &= ~Ranks[r];

        OutpostSquareMasks[WHITE][s] = PassedPawnMasks[WHITE][s] & ~Files[fileOf(s)];
        OutpostSquareMasks[BLACK][s] = PassedPawnMasks[BLACK][s] & ~Files[fileOf(s)];
    }

    // Initalize relative outpost ranks
    OutpostRanks[WHITE] = RANK_4 | RANK_5 | RANK_6;
    OutpostRanks[BLACK] = RANK_3 | RANK_4 | RANK_5;

    // Initalize pawn connected masks
    for (int s = 8 ; s < 56; s++) {
        PawnConnectedMasks[WHITE][s] = pawnAttacks(BLACK, s) | pawnAttacks(BLACK, s + 8);
        PawnConnectedMasks[BLACK][s] = pawnAttacks(WHITE, s) | pawnAttacks(WHITE, s - 8);
    }
}

uint64_t bitsBetweenMasks(int s1, int s2) {
    assert(0 <= s1 && s1 < SQUARE_NB);
    assert(0 <= s2 && s2 < SQUARE_NB);
    return BitsBetweenMasks[s1][s2];
}

uint64_t ranksAtOrAboveMasks(int c, int r) {
    assert(0 <= c && c < COLOUR_NB);
    assert(0 <= r && r < RANK_NB);
    return RanksAtOrAboveMasks[c][r];
}

uint64_t isolatedPawnMasks(int s) {
    assert(0 <= s && s < SQUARE_NB);
    return IsolatedPawnMasks[s];
}

uint64_t passedPawnMasks(int c, int s) {
    assert(0 <= c && c < COLOUR_NB);
    assert(0 <= s && s < SQUARE_NB);
    return PassedPawnMasks[c][s];
}

uint64_t pawnConnectedMasks(int c, int s) {
    assert(0 <= c && c < COLOUR_NB);
    assert(0 <= s && s < SQUARE_NB);
    return PawnConnectedMasks[c][s];
}

uint64_t outpostSquareMasks(int c, int s) {
    assert(0 <= c && c < COLOUR_NB);
    assert(0 <= s && s < SQUARE_NB);
    return OutpostSquareMasks[c][s];
}

uint64_t outpostRanks(int c) {
    assert(0 <= c && c < COLOUR_NB);
    return OutpostRanks[c];
}
