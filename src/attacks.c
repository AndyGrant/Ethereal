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

#ifdef USE_PEXT
#include <immintrin.h>
#endif

#include "attacks.h"
#include "bitboards.h"
#include "types.h"


uint64_t PawnAttacks[COLOUR_NB][SQUARE_NB];
uint64_t KnightAttacks[SQUARE_NB];
uint64_t BishopAttacks[0x1480];
uint64_t RookAttacks[0x19000];
uint64_t KingAttacks[SQUARE_NB];

Magic BishopTable[SQUARE_NB];
Magic RookTable[SQUARE_NB];


static int validCoordinate(int rank, int file) {
    return 0 <= rank && rank < 8
        && 0 <= file && file < 8;
}

static void setSquare(uint64_t *bb, int rank, int file){
    if (validCoordinate(rank, file))
        *bb |= 1ull << square(rank, file);
}

static int sliderIndex(uint64_t occupied, Magic *table) {
#ifdef USE_PEXT
    return _pext_u64(occupied, table->mask);
#else
    return ((occupied & table->mask) * table->magic) >> table->shift;
#endif
}

static uint64_t sliderAttacks(int sq, uint64_t occupied, const int delta[4][2]) {

    int rank, file, dr, df;
    uint64_t result = 0ull;

    for (int i = 0; i < 4; i++) {

        dr = delta[i][0], df = delta[i][1];

        for (rank = rankOf(sq) + dr, file = fileOf(sq) + df;
             validCoordinate(rank, file);
             rank += dr, file += df) {

            setBit(&result, square(rank, file));
            if (testBit(occupied, square(rank, file)))
                break;
        }
    }

    return result;
}

static void initSliderAttacks(int sq, Magic *table, uint64_t magic, const int delta[4][2]) {

    const uint64_t edges = ((RANK_1 | RANK_8) & ~Ranks[rankOf(sq)])
                         | ((FILE_A | FILE_H) & ~Files[fileOf(sq)]);

    uint64_t occupied = 0ull;

    table[sq].magic = magic;
    table[sq].mask  = sliderAttacks(sq, 0, delta) & ~edges;
    table[sq].shift = 64 - popcount(table[sq].mask);

    if (sq != SQUARE_NB - 1)
        table[sq+1].offset = table[sq].offset + (1 << popcount(table[sq].mask));

    do {
        int index = sliderIndex(occupied, &table[sq]);
        table[sq].offset[index] = sliderAttacks(sq, occupied, delta);
        occupied = (occupied - table[sq].mask) & table[sq].mask;
    } while (occupied);
}

void initAttacks() {

    const int PawnDelta[2][2]   = {{ 1,-1}, { 1, 1}};
    const int KnightDelta[8][2] = {{-2,-1}, {-2, 1}, {-1,-2}, {-1, 2},{ 1,-2}, { 1, 2}, { 2,-1}, { 2, 1}};
    const int BishopDelta[4][2] = {{-1,-1}, {-1, 1}, { 1,-1}, { 1, 1}};
    const int RookDelta[4][2]   = {{-1, 0}, { 0,-1}, { 0, 1}, { 1, 0}};
    const int KingDelta[8][2]   = {{-1,-1}, {-1, 0}, {-1, 1}, { 0,-1},{ 0, 1}, { 1,-1}, { 1, 0}, { 1, 1}};

    // First square has initial offset
    BishopTable[0].offset = BishopAttacks;
    RookTable[0].offset = RookAttacks;

    // Init attack tables for Pawns
    for (int sq = 0; sq < 64; sq++) {
        for (int dir = 0; dir < 2; dir++) {
            setSquare(&PawnAttacks[WHITE][sq], rankOf(sq) + PawnDelta[dir][0], fileOf(sq) + PawnDelta[dir][1]);
            setSquare(&PawnAttacks[BLACK][sq], rankOf(sq) - PawnDelta[dir][0], fileOf(sq) - PawnDelta[dir][1]);
        }
    }

    // Init attack tables for Knights & Kings
    for (int sq = 0; sq < 64; sq++) {
        for (int dir = 0; dir < 8; dir++) {
            setSquare(&KnightAttacks[sq], rankOf(sq) + KnightDelta[dir][0], fileOf(sq) + KnightDelta[dir][1]);
            setSquare(  &KingAttacks[sq], rankOf(sq) +   KingDelta[dir][0], fileOf(sq) +   KingDelta[dir][1]);
        }
    }

    // Init attack tables for sliding pieces
    for (int sq = 0; sq < 64; sq++) {
        initSliderAttacks(sq, BishopTable, BishopMagics[sq], BishopDelta);
        initSliderAttacks(sq,   RookTable,   RookMagics[sq],   RookDelta);
    }
}

uint64_t pawnAttacks(int colour, int sq) {
    assert(0 <= colour && colour < COLOUR_NB);
    assert(0 <= sq && sq < SQUARE_NB);
    return PawnAttacks[colour][sq];
}

uint64_t knightAttacks(int sq) {
    assert(0 <= sq && sq < SQUARE_NB);
    return KnightAttacks[sq];
}

uint64_t bishopAttacks(int sq, uint64_t occupied) {
    assert(0 <= sq && sq < SQUARE_NB);
    return BishopTable[sq].offset[sliderIndex(occupied, &BishopTable[sq])];
}

uint64_t rookAttacks(int sq, uint64_t occupied) {
    assert(0 <= sq && sq < SQUARE_NB);
    return RookTable[sq].offset[sliderIndex(occupied, &RookTable[sq])];
}

uint64_t queenAttacks(int sq, uint64_t occupied) {
    assert(0 <= sq && sq < SQUARE_NB);
    return bishopAttacks(sq, occupied) | rookAttacks(sq, occupied);
}

uint64_t kingAttacks(int sq) {
    assert(0 <= sq && sq < SQUARE_NB);
    return KingAttacks[sq];
}
