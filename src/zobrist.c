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

#include <stdlib.h>
#include <stdint.h>

#include "types.h"
#include "zobrist.h"

uint64_t ZobristKeys[32][SQUARE_NB];
uint64_t ZobristEnpassKeys[FILE_NB];
uint64_t ZobristCastleKeys[0x10];
uint64_t ZobristTurnKey;

uint64_t rand64() {

    // http://vigna.di.unimi.it/ftp/papers/xorshift.pdf

    static uint64_t seed = 1070372ull;

    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;

    return seed * 2685821657736338717ull;
}

void initZobrist() {

    // Init the main Zobrist keys for pieces and squares
    for (int pt = PAWN; pt <= KING; pt++) {
        for (int sq = 0; sq < SQUARE_NB; sq++) {
            ZobristKeys[makePiece(pt, WHITE)][sq] = rand64();
            ZobristKeys[makePiece(pt, BLACK)][sq] = rand64();
        }
    }

    // Init the enpass file Zobrist keys
    for (int f = 0; f < FILE_NB; f++)
        ZobristEnpassKeys[f] = rand64();

    // Init the Zobrist castle keys for each castle status
    ZobristCastleKeys[WHITE_OO_RIGHTS ] = rand64();
    ZobristCastleKeys[WHITE_OOO_RIGHTS] = rand64();
    ZobristCastleKeys[BLACK_OO_RIGHTS ] = rand64();
    ZobristCastleKeys[BLACK_OOO_RIGHTS] = rand64();

    // Combine the Zobrist castle keys for all possible castling rights
    for (int cr = 0; cr < 0x10; cr++) {

        if (cr & WHITE_OO_RIGHTS)
            ZobristCastleKeys[cr] ^= ZobristCastleKeys[WHITE_OO_RIGHTS];

        if (cr & WHITE_OOO_RIGHTS)
            ZobristCastleKeys[cr] ^= ZobristCastleKeys[WHITE_OOO_RIGHTS];

        if (cr & BLACK_OO_RIGHTS)
            ZobristCastleKeys[cr] ^= ZobristCastleKeys[BLACK_OO_RIGHTS];

        if (cr & BLACK_OOO_RIGHTS)
            ZobristCastleKeys[cr] ^= ZobristCastleKeys[BLACK_OOO_RIGHTS];
    }

    // Init the Zobrist key for side to move
    ZobristTurnKey = rand64();
}
