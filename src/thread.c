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

Thread* createThreadPool(int nthreads, pthread_mutex_t* lock){
    
    int i;
    Thread* threads = malloc(sizeof(Thread) * nthreads);
    
    for (i = 0; i < nthreads; i++){
        threads[i].threads = threads;
        threads[i].nthreads = nthreads;
        threads[i].lock = lock;
    }
    
    resetThreadPool(threads);
    
    return threads;
}

void resetThreadPool(Thread* threads){
    
    int i;
    
    for (i = 0; i < threads[0].nthreads; i++){
        memset(&threads[i].ptable,  0, sizeof(PawnTable   ));
        memset(&threads[i].killers, 0, sizeof(KillerTable ));
        memset(&threads[i].history, 0, sizeof(HistoryTable));
    }
    
}

void newSearchThreadPool(Thread* threads, Board* board, Limits* limits, SearchInfo* info,
                                  double starttime, double* idealusage, double maxusage){                             
    int i;
    
    for (i = 0; i < threads[0].nthreads; i++){
        
        threads[i].initialboard = board;
        memcpy(&threads[i].board, board, sizeof(Board));
        
        threads[i].limits     = limits;
        threads[i].info       = info;
        threads[i].idealusage = idealusage;
        threads[i].starttime  = starttime;
        threads[i].maxusage   = maxusage;
        
        reduceHistory(threads[i].history);
        
        threads[i].depth = 0;
        threads[i].nodes = 0ull;
        
        threads[i].abort = 0;
    }
}
                               
uint64_t nodesSearchedThreadPool(Thread* threads){
    
    int i;
    uint64_t nodes = 0;
    
    for (i = 0; i < threads[0].nthreads; i++)
        nodes += threads[i].nodes;
    
    return nodes;
}