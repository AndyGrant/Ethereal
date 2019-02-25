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

#ifndef _THREAD_H
#define _THREAD_H

#include <setjmp.h>

#include "board.h"
#include "search.h"
#include "transposition.h"
#include "types.h"

struct Thread {

    Limits* limits;
    SearchInfo* info;

    Board board;
    PVariation pv;

    int value;
    int depth;
    int seldepth;
    uint64_t nodes;
    uint64_t tbhits;

    int *evalStack;
    int _evalStack[MAX_PLY+4];

    uint16_t *moveStack;
    uint16_t _moveStack[MAX_PLY+4];

    int *pieceStack;
    int _pieceStack[MAX_PLY+4];

    Undo undoStack[MAX_PLY];

    jmp_buf jbuffer;

    int index;
    int nthreads;
    Thread* threads;

    PawnKingTable pktable;
    KillerTable killers;
    CounterMoveTable cmtable;
    HistoryTable history;
    ContinuationTable continuation;
};


Thread* createThreadPool(int nthreads);
void resetThreadPool(Thread* threads);
void newSearchThreadPool(Thread* threads, Board* board, Limits* limits, SearchInfo* info);

uint64_t nodesSearchedThreadPool(Thread* threads);
uint64_t tbhitsSearchedThreadPool(Thread* threads);

#endif
