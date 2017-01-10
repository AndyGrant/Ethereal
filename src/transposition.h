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

#ifndef _TRANSPOSITON_H
#define _TRANSPOSITON_H

#include <stdint.h>

#include "types.h"

void initalizeTranspositionTable(TransTable * table, uint64_t megabytes);
void updateTranspositionTable(TransTable * table);
void destroyTranspositionTable(TransTable * table);
void clearTranspositionTable(TransTable * table);
TransEntry * getTranspositionEntry(TransTable * table, uint64_t hash);
void storeTranspositionEntry(TransTable * table, uint8_t depth,uint8_t type, int16_t value, uint16_t bestMove, uint64_t hash);
void dumpTranspositionTable(TransTable * table);

void initalizePawnTable(PawnTable * ptable);
void destoryPawnTable(PawnTable * ptable);
PawnEntry * getPawnEntry(PawnTable * ptable, uint64_t phash);
void storePawnEntry(PawnTable * ptable, uint64_t phash, uint64_t passed, int mg, int eg);

extern TransTable Table;
extern PawnTable PTable;

#define PVNODE  (1)
#define CUTNODE (2)
#define ALLNODE (3)

#define EntrySetAge(e,a)    ((e)->data = ((a) << 2) | ((e)->data & 3))
#define EntryDepth(e)       ((e).depth)
#define EntryHash16(e)      ((e).hash16)
#define EntryAge(e)         ((e).data >> 2)
#define EntryType(e)        ((e).data & 3)
#define EntryMove(e)        ((e).bestMove)
#define EntryValue(e)       ((e).value)

#endif 