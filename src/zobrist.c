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

#include <stdint.h>

#include "types.h"
#include "zobrist.h"

uint64_t ZobristKeys[32][SQUARE_NB];
uint64_t ZobristEnpassKeys[FILE_NB];
uint64_t ZobristCastleKeys[SQUARE_NB];
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

    // Init the main Zobrist keys for all pieces
    for (int piece = PAWN; piece <= KING; piece++)
        for (int sq = 0; sq < SQUARE_NB; sq++)
            for (int colour = WHITE; colour <= BLACK; colour++)
                ZobristKeys[makePiece(piece, colour)][sq] = rand64();

    // Init the Zobrist keys for each enpass file
    for (int file = 0; file < FILE_NB; file++)
        ZobristEnpassKeys[file] = rand64();

    // Init the Zobrist keys for each castle rook
    for (int sq = 0; sq < SQUARE_NB; sq++)
        ZobristCastleKeys[sq] = rand64();

    // Init the Zobrist key for side to move
    ZobristTurnKey = rand64();
}
