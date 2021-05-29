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
#include <stdalign.h>

enum { MG, EG };

enum { WHITE, BLACK };

enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

enum { MAX_PLY = 128, MAX_MOVES = 256 };

enum {
    WHITE_PAWN   =  0, BLACK_PAWN   =  1,
    WHITE_KNIGHT =  4, BLACK_KNIGHT =  5,
    WHITE_BISHOP =  8, BLACK_BISHOP =  9,
    WHITE_ROOK   = 12, BLACK_ROOK   = 13,
    WHITE_QUEEN  = 16, BLACK_QUEEN  = 17,
    WHITE_KING   = 20, BLACK_KING   = 21,
    EMPTY        = 26
};

enum {
    MATE  = 32000 + MAX_PLY, MATE_IN_MAX  =  MATE - MAX_PLY,
    TBWIN = 31000 + MAX_PLY, TBWIN_IN_MAX = TBWIN - MAX_PLY,
    VALUE_NONE = MATE + 1
};

enum {
    SQUARE_NB = 64, COLOUR_NB = 2,
    RANK_NB   =  8, FILE_NB   = 8,
    PHASE_NB  =  2, PIECE_NB  = 6,
    CONT_NB   =  2
};

static inline int pieceType(int piece) {
    assert(0 <= piece / 4 && piece / 4 <= PIECE_NB);
    assert(piece % 4 <= COLOUR_NB);
    return piece / 4;
}

static inline int pieceColour(int piece) {
    assert(0 <= piece / 4 && piece / 4 <= PIECE_NB);
    assert(piece % 4 <= COLOUR_NB);
    return piece % 4;
}

static inline int makePiece(int type, int colour) {
    assert(0 <= type && type < PIECE_NB);
    assert(0 <= colour && colour <= COLOUR_NB);
    return type * 4 + colour;
}

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

// Forward definition of all structs

typedef struct Magic Magic;
typedef struct Board Board;
typedef struct Undo Undo;
typedef struct EvalTrace EvalTrace;
typedef struct EvalInfo EvalInfo;
typedef struct MovePicker MovePicker;
typedef struct SearchInfo SearchInfo;
typedef struct PVariation PVariation;
typedef struct Thread Thread;
typedef struct TTEntry TTEntry;
typedef struct TTBucket TTBucket;
typedef struct PKEntry PKEntry;
typedef struct TTable TTable;
typedef struct Limits Limits;
typedef struct UCIGoStruct UCIGoStruct;

// Renamings, currently for move ordering

typedef uint16_t KillerTable[MAX_PLY+1][2];
typedef uint16_t CounterMoveTable[COLOUR_NB][PIECE_NB][SQUARE_NB];

typedef int16_t HistoryTable[COLOUR_NB][SQUARE_NB][SQUARE_NB];
typedef int16_t ContinuationTable[CONT_NB][PIECE_NB][SQUARE_NB][PIECE_NB][SQUARE_NB];
typedef int16_t CaptureHistoryTable[PIECE_NB][SQUARE_NB][PIECE_NB-1];

// Trivial alignment macros

#define ALIGN64 alignas(64)
#define INLINE static inline __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))