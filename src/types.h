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

#include <assert.h>
#include <stdint.h>

enum {WHITE, BLACK};

enum {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

enum {
    WHITE_PAWN   =  0,
    BLACK_PAWN   =  1,
    WHITE_KNIGHT =  4,
    BLACK_KNIGHT =  5,
    WHITE_BISHOP =  8,
    BLACK_BISHOP =  9,
    WHITE_ROOK   = 12,
    BLACK_ROOK   = 13,
    WHITE_QUEEN  = 16,
    BLACK_QUEEN  = 17,
    WHITE_KING   = 20,
    BLACK_KING   = 21,
    EMPTY        = 26
};

static inline int pieceType(int p) {
    assert(0 <= p / 4 && p / 4 <= PIECE_NB);
    assert(p % 4 <= COLOUR_NB);
    return p / 4;
}

static inline int pieceColour(int p) {
    assert(0 <= p / 4 && p / 4 <= PIECE_NB);
    assert(p % 4 <= COLOUR_NB);
    return p % 4;
}

static inline int makePiece(int pt, int c) {
    assert(0 <= pt && pt < PIECE_NB);
    return pt * 4 + c;
}

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

enum {
    MAX_PLY = 128,
    MAX_MOVES = 256
};

enum {
    MATE = 32000,
    MATE_IN_MAX = MATE - MAX_PLY,
    MATED_IN_MAX = MAX_PLY - MATE,
    VALUE_NONE = 32001
};

enum {
    SQUARE_NB = 64,
    COLOUR_NB = 2,
    RANK_NB   = 8,
    FILE_NB   = 8,
    PHASE_NB  = 2,
    PIECE_NB  = 6,
};

enum {
    MG = 0,
    EG = 1,
};

// Declare each structure in the order in which they appear
// as you go through each header file one at a time

typedef struct Board Board;
typedef struct Undo Undo;
typedef struct EvalTrace EvalTrace;
typedef struct EvalInfo EvalInfo;
typedef struct MovePicker MovePicker;
typedef struct SearchInfo SearchInfo;
typedef struct PVariation PVariation;
typedef struct TexelTuple TexelTuple;
typedef struct TexelEntry TexelEntry;
typedef struct Thread Thread;
typedef struct TTEntry TTEntry;
typedef struct TTBucket TTBucket;
typedef struct TTable TTable;
typedef struct PawnKingEntry PawnKingEntry;
typedef struct PawnKingTable PawnKingTable;
typedef struct Limits Limits;
typedef struct ThreadsGo ThreadsGo;

// We define some simple renamings here

typedef uint16_t KillerTable[MAX_PLY][2];
typedef int16_t HistoryTable[COLOUR_NB][SQUARE_NB][SQUARE_NB];
