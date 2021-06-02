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

#include <stdlib.h>

#include "search.h"
#include "thread.h"
#include "time.h"
#include "types.h"
#include "uci.h"

int MoveOverhead = 300; // Set by UCI options

double getRealTime() {
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

double elapsedTime(SearchInfo *info) {
    return getRealTime() - info->startTime;
}

void initTimeManagment(SearchInfo *info, Limits *limits) {

    info->startTime = limits->start; // Save off the start time of the search

    info->pvFactor = 0; // Clear our stability time usage heuristic

    // Allocate time if Ethereal is handling the clock
    if (limits->limitedBySelf) {

        // Playing using X / Y + Z time control
        if (limits->mtg >= 0) {
            info->idealUsage =  0.67 * (limits->time - MoveOverhead) / (limits->mtg +  5) + limits->inc;
            info->maxAlloc   =  4.00 * (limits->time - MoveOverhead) / (limits->mtg +  7) + limits->inc;
            info->maxUsage   = 10.00 * (limits->time - MoveOverhead) / (limits->mtg + 10) + limits->inc;
        }

        // Playing using X + Y time controls
        else {
            info->idealUsage =  0.90 * ((limits->time - MoveOverhead) + 25 * limits->inc) / 50;
            info->maxAlloc   =  5.00 * ((limits->time - MoveOverhead) + 25 * limits->inc) / 50;
            info->maxUsage   = 10.00 * ((limits->time - MoveOverhead) + 25 * limits->inc) / 50;
        }

        // Cap time allocations using the move overhead
        info->idealUsage = MIN(info->idealUsage, limits->time - MoveOverhead);
        info->maxAlloc   = MIN(info->maxAlloc,   limits->time - MoveOverhead);
        info->maxUsage   = MIN(info->maxUsage,   limits->time - MoveOverhead);
    }

    // Interface told us to search for a predefined duration
    if (limits->limitedByTime) {
        info->idealUsage = limits->timeLimit;
        info->maxAlloc   = limits->timeLimit;
        info->maxUsage   = limits->timeLimit;
    }
}

void updateTimeManagment(SearchInfo *info, Limits *limits) {

    const int thisValue = info->values[info->depth];
    const int lastValue = info->values[info->depth-1];

    // Don't adjust time when we are at low depths, or if
    // we simply are not in control of our own time usage
    if (!limits->limitedBySelf || info->depth < 4) return;

    // Increase our time if the score suddenly dropped
    if (lastValue > thisValue + 10) info->idealUsage *= 1.050;
    if (lastValue > thisValue + 20) info->idealUsage *= 1.050;
    if (lastValue > thisValue + 40) info->idealUsage *= 1.050;

    // Increase our time if the score suddenly jumped
    if (lastValue + 15 < thisValue) info->idealUsage *= 1.025;
    if (lastValue + 30 < thisValue) info->idealUsage *= 1.050;

    // Always scale back the PV time factor, but also look
    // to reset the PV time factor if the best move changed
    info->pvFactor = MAX(0, info->pvFactor - 1);
    if (info->bestMoves[info->depth] != info->bestMoves[info->depth-1])
        info->pvFactor = PVFactorCount;
}

int terminateTimeManagment(SearchInfo *info) {

    // Adjust our ideal usage based on variance in the best move
    // between iterations of the search. We won't allow the new
    // usage value to exceed our maximum allocation. The cutoff
    // is reached if the elapsed time exceeds the ideal usage

    double cutoff = info->idealUsage;
    cutoff *= 1.00 + info->pvFactor * PVFactorWeight;
    return elapsedTime(info) > MIN(cutoff, info->maxAlloc);
}

int terminateSearchEarly(Thread *thread) {

    // Terminate the search early if the max usage time has passed.
    // Only check this once for every 1024 nodes examined, in case
    // the system calls are quite slow. Always be sure to avoid an
    // early exit during a depth 1 search, to ensure a best move

    const Limits *limits = thread->limits;

    if (limits->limitedByNodes)
        return thread->depth > 1
            && thread->nodes >= limits->nodeLimit / thread->nthreads;

    return  thread->depth > 1
        && (thread->nodes & 1023) == 1023
        && (limits->limitedBySelf || limits->limitedByTime)
        &&  elapsedTime(thread->info) >= thread->info->maxUsage;
}
