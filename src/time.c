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

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "thread.h"
#include "time.h"
#include "types.h"
#include "uci.h"

double getRealTime(){
#if defined(_WIN32) || defined(_WIN64)
    return (double)(GetTickCount());
#else
    struct timeval tv;
    double secsInMilli, usecsInMilli;
    
    gettimeofday(&tv, NULL);
    secsInMilli = ((double)tv.tv_sec) * 1000;
    usecsInMilli = tv.tv_usec / 1000;
    
    return secsInMilli + usecsInMilli;
#endif
}

double getElapsedTime(double reference){
    return getRealTime() - reference;
}

void initializeManager(Manager* manager, Limits* limits, double time, double mtg, double inc){
    
    // Zero out score, move, and time history for the Iterative
    // Deepening loop within the main Thread. Also, the max depth
    // reached by the search must be set back to zero
    memset(manager, 0, sizeof(Manager));
    
    // Establish start time of our search
    manager->startTime = getRealTime();
    
    // Ethereal is responsible for choosing how much time to spend searching
    if (limits->limitedBySelf){
        
        // Allocate when using X/Y or X/Y+Z
        if (mtg >= 0){
            manager->idealUsage =  0.65 * time / (mtg +  5) + inc;
            manager->maxAlloc   =  4.00 * time / (mtg +  7) + inc;
            manager->maxUsage   = 10.00 * time / (mtg + 10) + inc;
        }
        
        // Allocate when using X+Y, or simply X
        else {
            manager->idealUsage =  0.45 * (time + 23 * inc) / 25;
            manager->maxAlloc   =  4.00 * (time + 23 * inc) / 25;
            manager->maxUsage   = 10.00 * (time + 23 * inc) / 25;
        }
        
        // Cap our allocation to ensure we leave some time to report
        manager->idealUsage = MIN(manager->idealUsage, time - 100);
        manager->maxAlloc   = MIN(manager->maxAlloc,   time -  75);
        manager->maxUsage   = MIN(manager->maxUsage,   time -  50);
    }
    
    // UCI command told us to look for exactly X seconds
    else if (limits->limitedByTime){
        manager->idealUsage = limits->timeLimit;
        manager->maxAlloc   = limits->timeLimit;
        manager->maxUsage   = limits->timeLimit;
    }
    
    // Initialize our stability factor to 1.00
    manager->pvStability = 1.00;
    
    // Finally, copy over the information in Limits
    manager->depthLimit     = limits->depthLimit;
    manager->timeLimit      = limits->timeLimit;
    manager->limitedByNone  = limits->limitedByNone;
    manager->limitedByTime  = limits->limitedByTime;
    manager->limitedByDepth = limits->limitedByDepth;
    manager->limitedBySelf  = limits->limitedBySelf;
}

void updateManager(Manager* manager, int depth, int value, uint16_t bestMove){
    
    // Save the results of this root search
    manager->depth            = depth;
    manager->values[depth]    = value;
    manager->bestMoves[depth] = bestMove;
    manager->timeUsage[depth] = getElapsedTime(manager->startTime + manager->timeUsage[depth-1]);
    
    // If Ethereal is managing the clock, determine if we should be spending
    // more time on this search, based on the score difference between iterations
    // and any changes in the principle variation since the last iteration
    if (manager->limitedBySelf && depth >= 4){
        
        // Increase our time if the score suddently dropped by eight centipawns
        if (manager->values[depth-1] > value + 10)
            manager->idealUsage *= 1.050;
        
        // Decrease our time if the score suddently jumped by eight centipawns
        if (manager->values[depth-1] < value - 10)
            manager->idealUsage *= 0.975;
        
        // Increase our time if the pv has changed across the last two iterations
        if (manager->bestMoves[depth-1] != bestMove)
            manager->idealUsage *= MAX(manager->pvStability, 1.30);
        
        // Decrease our time if the pv has stayed the same between iterations
        if (manager->bestMoves[depth-1] == bestMove)
            manager->idealUsage *= MAX(0.95, MIN(manager->pvStability, 1.00));
        
        // Cap our ideal usage at the max allocation of time
        manager->idealUsage = MIN(manager->idealUsage, manager->maxAlloc);
        
        // Update the PV Stability depending on the best move changing. If the best move is
        // holding stable, we increase the pv stability. This way, if the best move changes
        // after holding for many iterations, more time will be allocated for the search, and
        // less time if the best move is in a constant flucation.
        manager->pvStability *= (manager->bestMoves[depth-1] != bestMove) ? 0.95 : 1.05;
    }
}

int terminateSearchHere(Manager* manager){
    
    int depth;
    double timeFactor, estimatedUsage;
    
    // Check for termination by the regular search limitations. This is our
    // depth limited, time limited, and Ethereal limited searches
    if (   (manager->limitedByDepth && manager->depth >= manager->depthLimit)
        || (manager->limitedByTime  && getElapsedTime(manager->startTime) > manager->timeLimit)
        || (manager->limitedBySelf  && getElapsedTime(manager->startTime) > manager->idealUsage)
        || (manager->limitedBySelf  && getElapsedTime(manager->startTime) > manager->maxUsage))
        return 1;

    // Check to see if we expect to be able to complete the next depth
    if (manager->limitedBySelf && manager->depth >= 4){
        
        // Look at the last completed depth
        depth = manager->depth;
        
        // Time factor between the last completed search iterations
        timeFactor = manager->timeUsage[depth] / MAX(50, manager->timeUsage[depth-1]);
        
        // Assume the factor (+ a buffer) is a good estimate for the next search time
        estimatedUsage = manager->timeUsage[depth] * (timeFactor + .40);
        
        // If the assumed time usage would exceed our maximum, terminate search
        if (getElapsedTime(manager->startTime) + estimatedUsage > manager->maxUsage)
            return 1;
    }
    
    // No conditions have been met to terminate search
    return 0;
}

int searchTimeHasExpired(Thread* thread){
    
    // Check to see if search time has expired. We will force the search
    // to continue after the search time has been used in the event that we have
    // not yet completed our depth one search, and therefore would have no best move
    return (thread->manager->limitedBySelf || thread->manager->limitedByTime)
        && (thread->nodes & 4095) == 4095
        &&  getElapsedTime(thread->manager->startTime) > thread->manager->maxUsage
        &&  thread->depth > 1;
}

