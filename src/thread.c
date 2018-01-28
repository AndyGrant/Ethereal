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

#include "transposition.h"
#include "history.h"
#include "search.h"
#include "board.h"
#include "thread.h"
#include "types.h"

Thread* createThreadPool(int nthreads){
    
    int i;
    Thread* threads = malloc(sizeof(Thread) * nthreads);
    
    // Provide each thread with a reference to the others,
    // as well as a counter of how many threads there are
    for (i = 0; i < nthreads; i++){
        threads[i].threads = threads;
        threads[i].nthreads = nthreads;
    }
    
    resetThreadPool(threads);
    
    return threads;
}

void resetThreadPool(Thread* threads){
    
    int i;
    
    // Reset each of the tables used by individual Threads. This is only
    // needed between 'ucinewgame's in order to get deterministic results
    // between games. Between individual searches the tables aid us
    for (i = 0; i < threads[0].nthreads; i++){
        memset(&threads[i].ptable,  0, sizeof(PawnTable   ));
        memset(&threads[i].killers, 0, sizeof(KillerTable ));
        memset(&threads[i].history, 0, sizeof(HistoryTable));
    }
}

void newSearchThreadPool(Thread* threads, Board* board, Limits* limits, SearchInfo* info){
    
    int i;
    
    // Initialize each Thread in the Thread Pool
    for (i = 0; i < threads[0].nthreads; i++){
        
        // Save a reference to the original search specifications
        threads[i].limits = limits;
        
        // Save a reference to our displayed search information,
        // as well as the time usage variables that are being used
        threads[i].info = info;
        
        // Make our own copy of the original position
        memcpy(&threads[i].board, board, sizeof(Board));
        
        // Zero our the depth, nodes for the new search
        threads[i].depth = 0;
        threads[i].nodes = 0ull;
        
        // Reset the abort flag for the new search
        threads[i].abort = 0;
    }
}

uint64_t nodesSearchedThreadPool(Thread* threads){
    
    int i; uint64_t nodes;
    
    for (i = 0, nodes = 0ull; i < threads[0].nthreads; i++)
        nodes += threads[i].nodes;
    
    return nodes;
}