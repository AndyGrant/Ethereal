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

#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define MAX_PLY    (128)
#define MAX_MOVES  (256)

#define MATE         (32000)
#define MATE_IN_MAX  (+MATE - MAX_PLY)
#define MATED_IN_MAX (-MATE + MAX_PLY)
#define VALUE_NONE   (32001)

#define SQUARE_NB (64)
#define COLOUR_NB ( 2)
#define RANK_NB   ( 8)
#define FILE_NB   ( 8)
#define PHASE_NB  ( 2)
#define PIECE_NB  ( 6)

#define PVNODE  (1)
#define CUTNODE (2)
#define ALLNODE (3)

enum {
    BOUND_NONE  = 0,
    BOUND_LOWER = 1,
    BOUND_UPPER = 2,
    BOUND_EXACT = 3,
};

#define MG (0)
#define EG (1)

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

#endif
