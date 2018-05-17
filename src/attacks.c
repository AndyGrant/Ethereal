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
    #include <immintrin.h> // for _pext_u64() intrinsic
#endif

#include "attacks.h"
#include "bitboards.h"
#include "piece.h"
#include "types.h"

static uint64_t PawnAttacks[COLOUR_NB][SQUARE_NB];
static uint64_t KnightAttacks[SQUARE_NB];
static uint64_t KingAttacks[SQUARE_NB];

const uint64_t RookMagic[SQUARE_NB] = {
    0xa180022080400230ull, 0x0040100040022000ull, 0x0080088020001002ull, 0x0080080280841000ull,
    0x4200042010460008ull, 0x04800a0003040080ull, 0x0400110082041008ull, 0x008000a041000880ull,
    0x10138001a080c010ull, 0x0000804008200480ull, 0x00010011012000c0ull, 0x0022004128102200ull,
    0x000200081201200cull, 0x202a001048460004ull, 0x0081000100420004ull, 0x4000800380004500ull,
    0x0000208002904001ull, 0x0090004040026008ull, 0x0208808010002001ull, 0x2002020020704940ull,
    0x8048010008110005ull, 0x6820808004002200ull, 0x0a80040008023011ull, 0x00b1460000811044ull,
    0x4204400080008ea0ull, 0xb002400180200184ull, 0x2020200080100380ull, 0x0010080080100080ull,
    0x2204080080800400ull, 0x0000a40080360080ull, 0x02040604002810b1ull, 0x008c218600004104ull,
    0x8180004000402000ull, 0x488c402000401001ull, 0x4018a00080801004ull, 0x1230002105001008ull,
    0x8904800800800400ull, 0x0042000c42003810ull, 0x008408110400b012ull, 0x0018086182000401ull,
    0x2240088020c28000ull, 0x001001201040c004ull, 0x0a02008010420020ull, 0x0010003009010060ull,
    0x0004008008008014ull, 0x0080020004008080ull, 0x0282020001008080ull, 0x50000181204a0004ull,
    0x48fffe99fecfaa00ull, 0x48fffe99fecfaa00ull, 0x497fffadff9c2e00ull, 0x613fffddffce9200ull,
    0xffffffe9ffe7ce00ull, 0xfffffff5fff3e600ull, 0x0010301802830400ull, 0x510ffff5f63c96a0ull,
    0xebffffb9ff9fc526ull, 0x61fffeddfeedaeaeull, 0x53bfffedffdeb1a2ull, 0x127fffb9ffdfb5f6ull,
    0x411fffddffdbf4d6ull, 0x0801000804000603ull, 0x0003ffef27eebe74ull, 0x7645fffecbfea79eull
};

const uint64_t BishopMagic[SQUARE_NB] = {
    0xffedf9fd7cfcffffull, 0xfc0962854a77f576ull, 0x5822022042000000ull, 0x2ca804a100200020ull,
    0x0204042200000900ull, 0x2002121024000002ull, 0xfc0a66c64a7ef576ull, 0x7ffdfdfcbd79ffffull,
    0xfc0846a64a34fff6ull, 0xfc087a874a3cf7f6ull, 0x1001080204002100ull, 0x1810080489021800ull,
    0x0062040420010a00ull, 0x5028043004300020ull, 0xfc0864ae59b4ff76ull, 0x3c0860af4b35ff76ull,
    0x73c01af56cf4cffbull, 0x41a01cfad64aaffcull, 0x040c0422080a0598ull, 0x4228020082004050ull,
    0x0200800400e00100ull, 0x020b001230021040ull, 0x7c0c028f5b34ff76ull, 0xfc0a028e5ab4df76ull,
    0x0020208050a42180ull, 0x001004804b280200ull, 0x2048020024040010ull, 0x0102c04004010200ull,
    0x020408204c002010ull, 0x02411100020080c1ull, 0x102a008084042100ull, 0x0941030000a09846ull,
    0x0244100800400200ull, 0x4000901010080696ull, 0x0000280404180020ull, 0x0800042008240100ull,
    0x0220008400088020ull, 0x04020182000904c9ull, 0x0023010400020600ull, 0x0041040020110302ull,
    0xdcefd9b54bfcc09full, 0xf95ffa765afd602bull, 0x1401210240484800ull, 0x0022244208010080ull,
    0x1105040104000210ull, 0x2040088800c40081ull, 0x43ff9a5cf4ca0c01ull, 0x4bffcd8e7c587601ull,
    0xfc0ff2865334f576ull, 0xfc0bf6ce5924f576ull, 0x80000b0401040402ull, 0x0020004821880a00ull,
    0x8200002022440100ull, 0x0009431801010068ull, 0xc3ffb7dc36ca8c89ull, 0xc3ff8a54f4ca2c89ull,
    0xfffffcfcfd79edffull, 0xfc0863fccb147576ull, 0x040c000022013020ull, 0x2000104000420600ull,
    0x0400000260142410ull, 0x0800633408100500ull, 0xfc087e8e4bb2f736ull, 0x43ff9e4ef4ca2c89ull
};

static uint64_t RookAttacks[0x19000], BishopAttacks[0x1480];
static uint64_t *BishopAttacksPtr[SQUARE_NB], *RookAttacksPtr[SQUARE_NB];

static uint64_t BishopMask[SQUARE_NB], RookMask[SQUARE_NB];
static unsigned BishopShift[SQUARE_NB], RookShift[SQUARE_NB];

static uint64_t sliderAttacks(int s, uint64_t occ, const int dir[4][2]) {

    uint64_t result = 0;

    for (int i = 0; i < 4; i++) {
        int dr = dir[i][0], df = dir[i][1];
        int r, f;

        for (r = rankOf(s) + dr, f = fileOf(s) + df;
                0 <= r && r < RANK_NB && 0 <= f && f < FILE_NB;
                r += dr, f += df) {
            const int sq = square(r, f);
            setBit(&result, sq);

            if (testBit(occ, sq))
                break;
        }
    }

    return result;
}

static int sliderIndex(uint64_t occ, uint64_t mask, uint64_t magic, unsigned shift) {

#ifdef USE_PEXT
    (void)magic, (void)shift;  // Silence compiler warnings (unused variables)
    return _pext_u64(occ, mask);
#else
    return ((occ & mask) * magic) >> shift;
#endif
}

static void initSliderAttacks(int s, uint64_t mask[SQUARE_NB], const uint64_t magic[SQUARE_NB],
    unsigned shift[SQUARE_NB], uint64_t *attacksPtr[SQUARE_NB], const int dir[4][2]) {

    const uint64_t edges = ((RANK_1 | RANK_8) & ~Ranks[rankOf(s)])
                         | ((FILE_A | FILE_H) & ~Files[fileOf(s)]);
    mask[s] = sliderAttacks(s, 0, dir) & ~edges;
    shift[s] = 64 - popcount(mask[s]);

    if (s < SQUARE_NB - 1)
        attacksPtr[s + 1] = attacksPtr[s] + (1 << popcount(mask[s]));

    // Use the Carry-Rippler trick to loop over the subsets of mask[s]
    uint64_t occ = 0;

    do {
        attacksPtr[s][sliderIndex(occ, mask[s], magic[s], shift[s])] = sliderAttacks(s, occ, dir);
        occ = (occ - mask[s]) & mask[s];
    } while (occ);
}

static void setSquare(uint64_t *bb, int r, int f) {
    if (0 <= r && r < RANK_NB && 0 <= f && f < FILE_NB)
        setBit(bb, square(r, f));
}

void initAttacks() {

    const int PawnDir[2][2] = {{1,-1}, {1,1}};
    const int KnightDir[8][2] = {{-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1}};
    const int KingDir[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};
    const int BishopDir[4][2] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}};
    const int RookDir[4][2] = {{-1,0}, {0,-1}, {0,1}, {1,0}};

    // Initialise leaper attacks
    for (int s = 0; s < SQUARE_NB; s++) {
        const int r = rankOf(s), f = fileOf(s);

        for (int d = 0; d < 2; d++) {
            setSquare(&PawnAttacks[WHITE][s], r + PawnDir[d][0], f + PawnDir[d][1]);
            setSquare(&PawnAttacks[BLACK][s], r - PawnDir[d][0], f - PawnDir[d][1]);
        }

        for (int d = 0; d < 8; d++) {
            setSquare(&KnightAttacks[s], r + KnightDir[d][0], f + KnightDir[d][1]);
            setSquare(&KingAttacks[s], r + KingDir[d][0], f + KingDir[d][1]);
        }
    }

    // Initialise slider attacks
    BishopAttacksPtr[0] = BishopAttacks;
    RookAttacksPtr[0] = RookAttacks;

    for (int s = 0; s < SQUARE_NB; s++) {
        initSliderAttacks(s, BishopMask, BishopMagic, BishopShift, BishopAttacksPtr, BishopDir);
        initSliderAttacks(s, RookMask, RookMagic, RookShift, RookAttacksPtr, RookDir);
    }
}

uint64_t pawnAttacks(int c, int s) {
    assert(0 <= c && c < COLOUR_NB);
    assert(0 <= s && s < SQUARE_NB);
    return PawnAttacks[c][s];
}

uint64_t knightAttacks(int s) {
    assert(0 <= s && s < SQUARE_NB);
    return KnightAttacks[s];
}

uint64_t kingAttacks(int s) {
    assert(0 <= s && s < SQUARE_NB);
    return KingAttacks[s];
}

uint64_t bishopAttacks(int s, uint64_t occ) {
    assert(0 <= s && s < SQUARE_NB);
    return BishopAttacksPtr[s][sliderIndex(occ, BishopMask[s], BishopMagic[s], BishopShift[s])];
}

uint64_t rookAttacks(int s, uint64_t occ) {
    assert(0 <= s && s < SQUARE_NB);
    return RookAttacksPtr[s][sliderIndex(occ, RookMask[s], RookMagic[s], RookShift[s])];
}

uint64_t queenAttacks(int s, uint64_t occ) {
    assert(0 <= s && s < SQUARE_NB);
    return rookAttacks(s, occ) | bishopAttacks(s, occ);
}
