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
#include <pthread.h>

#include "types.h"
#include "transposition.h"
#include "search.h"

enum { ABORT_NONE, ABORT_DEPTH, ABORT_ALL };

typedef struct Thread {
    
    Board board;
    Board* initialboard;
    
    PVariation pv;
    Limits* limits;
    SearchInfo* info;
    double* idealusage;
    double starttime;
    double maxusage;
    
    int depth;
    int value;
    int lower;
    int upper;
    uint64_t nodes;
    
    int abort;
    jmp_buf jbuffer;
    
    int nthreads;
    Thread* threads;
    pthread_mutex_t* lock;
    
    PawnTable ptable;
    KillerTable killers;
    HistoryTable history;
    
} Thread;


Thread* createThreadPool(int nthreads, pthread_mutex_t* lock);

void resetThreadPool(Thread* threads);

void newSearchThreadPool(Thread* threads, Board* board, Limits* limits, SearchInfo* info,
                                  double starttime, double* idealusage, double maxusage);
                               
uint64_t nodesSearchedThreadPool(Thread* threads);

#endif
