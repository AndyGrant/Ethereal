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

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "history.h"
#include "search.h"
#include "thread.h"
#include "transposition.h"
#include "types.h"
#include "windows.h"

Thread* createThreadPool(int nthreads){

    Thread* threads = malloc(sizeof(Thread) * nthreads);

    for (int i = 0; i < nthreads; i++){

        // Threads will know of each other
        threads[i].index = i;
        threads[i].threads = threads;
        threads[i].nthreads = nthreads;

        // Offset stacks so root position can look backwards
        threads[i].evalStack = &(threads[i]._evalStack[4]);
        threads[i].moveStack = &(threads[i]._moveStack[4]);
        threads[i].pieceStack = &(threads[i]._pieceStack[4]);

        // Zero out the stack, most importantly the first four slots
        memset(&threads[i]._evalStack, 0, sizeof(int) * (MAX_PLY + 4));
        memset(&threads[i]._moveStack, 0, sizeof(uint16_t) * (MAX_PLY + 4));
        memset(&threads[i]._pieceStack, 0, sizeof(int) * (MAX_PLY + 4));
    }

    resetThreadPool(threads);

    return threads;
}

void resetThreadPool(Thread* threads){

    // Reset the per-thread tables, used for move ordering,
    // and evaluation caching. This is needed for ucinewgame
    // calls in order to ensure deterministic behaviour

    for (int i = 0; i < threads[0].nthreads; i++){
        memset(&threads[i].pktable, 0, sizeof(PawnKingTable));
        memset(&threads[i].killers, 0, sizeof(KillerTable));
        memset(&threads[i].cmtable, 0, sizeof(CounterMoveTable));
        memset(&threads[i].history, 0, sizeof(HistoryTable));
        memset(&threads[i].continuation, 0, sizeof(ContinuationTable));
    }
}

void newSearchThreadPool(Thread* threads, Board* board, Limits* limits, SearchInfo* info){

    // Initialize each Thread in the Thread Pool
    for (int i = 0; i < threads[0].nthreads; i++){

        // Original search parameters
        threads[i].limits = limits;

        // Tap into time information and iterative deepening data
        threads[i].info = info;

        // Make our own copy of the original position
        memcpy(&threads[i].board, board, sizeof(Board));

        // Zero out our depth and stat tracking
        threads[i].depth  = 0;
        threads[i].nodes  = 0ull;
        threads[i].tbhits = 0ull;
    }
}

uint64_t nodesSearchedThreadPool(Thread* threads){

    uint64_t nodes = 0ull;

    for (int i = 0; i < threads[0].nthreads; i++)
        nodes += threads[i].nodes;

    return nodes;
}

uint64_t tbhitsSearchedThreadPool(Thread* threads){

    uint64_t tbhits = 0ull;

    for (int i = 0; i < threads[0].nthreads; i++)
        tbhits += threads[i].tbhits;

    return tbhits;
}
