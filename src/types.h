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

#include "piece.h"

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define MAX_PLY    (128)
#define MAX_MOVES  (256)

#define MATE         (32000)
#define MATE_IN_MAX  (+MATE - 2 * MAX_PLY)
#define MATED_IN_MAX (-MATE + 2 * MAX_PLY)

#define SQUARE_NB (64)
#define COLOUR_NB ( 2)
#define RANK_NB   ( 8)
#define FILE_NB   ( 8)
#define PHASE_NB  ( 2)
#define PIECE_NB  ( 6)

#define PVNODE  (1)
#define CUTNODE (2)
#define ALLNODE (3)

#define BUCKET_SIZE (4)

#define MG (0)
#define EG (1)

struct Board;
typedef struct Board Board;

struct Undo;
typedef struct Undo Undo;

struct SearchInfo;
typedef struct SearchInfo SearchInfo;

struct TransEntry;
typedef struct TransEntry TransEntry;

struct TransBucket;
typedef struct TransBucket TransBucket;

struct TransTable;
typedef struct TransTable TransTable;

struct PawnKingEntry;
typedef struct PawnKingEntry PawnKingEntry;

struct PawnKingTable;
typedef struct PawnKingTable PawnKingTable;

struct MovePicker;
typedef struct MovePicker MovePicker;

struct PVariation;
typedef struct PVariation PVariation;

struct Limits;
typedef struct Limits Limits;

struct Thread;
typedef struct Thread Thread;

typedef uint16_t KillerTable[MAX_PLY][2];

typedef int16_t HistoryTable[COLOUR_NB][SQUARE_NB][SQUARE_NB];

struct EvalInfo;
typedef struct EvalInfo EvalInfo;

#endif
