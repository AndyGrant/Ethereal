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
#include "types.h"

static uint64_t PawnAttacks[COLOUR_NB][SQUARE_NB];
static uint64_t KnightAttacks[SQUARE_NB];
static uint64_t KingAttacks[SQUARE_NB];

const uint64_t RookMagic[SQUARE_NB] = {
    0xA180022080400230ull, 0x0040100040022000ull, 0x0080088020001002ull, 0x0080080280841000ull,
    0x4200042010460008ull, 0x04800A0003040080ull, 0x0400110082041008ull, 0x008000A041000880ull,
    0x10138001A080C010ull, 0x0000804008200480ull, 0x00010011012000C0ull, 0x0022004128102200ull,
    0x000200081201200Cull, 0x202A001048460004ull, 0x0081000100420004ull, 0x4000800380004500ull,
    0x0000208002904001ull, 0x0090004040026008ull, 0x0208808010002001ull, 0x2002020020704940ull,
    0x8048010008110005ull, 0x6820808004002200ull, 0x0A80040008023011ull, 0x00B1460000811044ull,
    0x4204400080008EA0ull, 0xB002400180200184ull, 0x2020200080100380ull, 0x0010080080100080ull,
    0x2204080080800400ull, 0x0000A40080360080ull, 0x02040604002810B1ull, 0x008C218600004104ull,
    0x8180004000402000ull, 0x488C402000401001ull, 0x4018A00080801004ull, 0x1230002105001008ull,
    0x8904800800800400ull, 0x0042000C42003810ull, 0x008408110400B012ull, 0x0018086182000401ull,
    0x2240088020C28000ull, 0x001001201040C004ull, 0x0A02008010420020ull, 0x0010003009010060ull,
    0x0004008008008014ull, 0x0080020004008080ull, 0x0282020001008080ull, 0x50000181204A0004ull,
    0x48FFFE99FECFAA00ull, 0x48FFFE99FECFAA00ull, 0x497FFFADFF9C2E00ull, 0x613FFFDDFFCE9200ull,
    0xFFFFFFE9FFE7CE00ull, 0xFFFFFFF5FFF3E600ull, 0x0010301802830400ull, 0x510FFFF5F63C96A0ull,
    0xEBFFFFB9FF9FC526ull, 0x61FFFEDDFEEDAEAEull, 0x53BFFFEDFFDEB1A2ull, 0x127FFFB9FFDFB5F6ull,
    0x411FFFDDFFDBF4D6ull, 0x0801000804000603ull, 0x0003FFEF27EEBE74ull, 0x7645FFFECBFEA79Eull
};

const uint64_t BishopMagic[SQUARE_NB] = {
    0xFFEDF9FD7CFCFFFFull, 0xFC0962854A77F576ull, 0x5822022042000000ull, 0x2CA804A100200020ull,
    0x0204042200000900ull, 0x2002121024000002ull, 0xFC0A66C64A7EF576ull, 0x7FFDFDFCBD79FFFFull,
    0xFC0846A64A34FFF6ull, 0xFC087A874A3CF7F6ull, 0x1001080204002100ull, 0x1810080489021800ull,
    0x0062040420010A00ull, 0x5028043004300020ull, 0xFC0864AE59B4FF76ull, 0x3C0860AF4B35FF76ull,
    0x73C01AF56CF4CFFBull, 0x41A01CFAD64AAFFCull, 0x040C0422080A0598ull, 0x4228020082004050ull,
    0x0200800400E00100ull, 0x020B001230021040ull, 0x7C0C028F5B34FF76ull, 0xFC0A028E5AB4DF76ull,
    0x0020208050A42180ull, 0x001004804B280200ull, 0x2048020024040010ull, 0x0102C04004010200ull,
    0x020408204C002010ull, 0x02411100020080C1ull, 0x102A008084042100ull, 0x0941030000A09846ull,
    0x0244100800400200ull, 0x4000901010080696ull, 0x0000280404180020ull, 0x0800042008240100ull,
    0x0220008400088020ull, 0x04020182000904C9ull, 0x0023010400020600ull, 0x0041040020110302ull,
    0xDCEFD9B54BFCC09Full, 0xF95FFA765AFD602Bull, 0x1401210240484800ull, 0x0022244208010080ull,
    0x1105040104000210ull, 0x2040088800C40081ull, 0x43FF9A5CF4CA0C01ull, 0x4BFFCD8E7C587601ull,
    0xFC0FF2865334F576ull, 0xFC0BF6CE5924F576ull, 0x80000B0401040402ull, 0x0020004821880A00ull,
    0x8200002022440100ull, 0x0009431801010068ull, 0xC3FFB7DC36CA8C89ull, 0xC3FF8A54F4CA2C89ull,
    0xFFFFFCFCFD79EDFFull, 0xFC0863FCCB147576ull, 0x040C000022013020ull, 0x2000104000420600ull,
    0x0400000260142410ull, 0x0800633408100500ull, 0xFC087E8E4BB2F736ull, 0x43FF9E4EF4CA2C89ull
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
    const int BishopDir[4][2] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}};
    const int RookDir[4][2] = {{-1,0}, {0,-1}, {0,1}, {1,0}};
    const int KingDir[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};

    // Initialise Pawn, Knight, and King attacks
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

    // Initialise Bishop, Rook, and Queen attacks
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

uint64_t kingAttacks(int s) {
    assert(0 <= s && s < SQUARE_NB);
    return KingAttacks[s];
}
