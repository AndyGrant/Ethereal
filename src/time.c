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

#include "search.h"
#include "time.h"
#include "types.h"
#include "uci.h"


int MoveOverhead = 100; // Set by UCI options


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

double elapsedTime(SearchInfo* info){

    return getRealTime() - info->startTime;

}

double estimatedUsage(SearchInfo* info){

    const int depth = info->depth;

    double timeFactor = info->timeUsage[depth] / MAX(1, info->timeUsage[depth-1]);

    return info->timeUsage[depth] * (timeFactor + .40);

}

void initializeTimeManagment(SearchInfo* info, Limits* limits){

    info->startTime = limits->start; // Save off the start time of the search

    info->bestMoveChanges = 0; // Clear our stability time usage heuristic

    // Allocate time if Ethereal is handling the clock
    if (limits->limitedBySelf){

        // Playing using X / Y or X / Y + Z time controls
        if (limits->mtg >= 0){
            info->idealUsage =  0.75 * limits->time / (limits->mtg +  5) + limits->inc;
            info->maxAlloc   =  4.00 * limits->time / (limits->mtg +  7) + limits->inc;
            info->maxUsage   = 10.00 * limits->time / (limits->mtg + 10) + limits->inc;
        }

        // Playing using X + Y or X time controls
        else {
            info->idealUsage =  0.52 * (limits->time + 23 * limits->inc) / 25;
            info->maxAlloc   =  4.00 * (limits->time + 23 * limits->inc) / 25;
            info->maxUsage   = 10.00 * (limits->time + 23 * limits->inc) / 25;
        }

        // Cap all time allocations using the move time buffer
        info->idealUsage = MIN(info->idealUsage, limits->time - MoveOverhead);
        info->maxAlloc   = MIN(info->maxAlloc,   limits->time - MoveOverhead);
        info->maxUsage   = MIN(info->maxUsage,   limits->time - MoveOverhead);
    }

    // Interface told us to search for a predefined duration
    if (limits->limitedByTime){
        info->idealUsage = limits->timeLimit;
        info->maxAlloc   = limits->timeLimit;
        info->maxUsage   = limits->timeLimit;
    }
}
