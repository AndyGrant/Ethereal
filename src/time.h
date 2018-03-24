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

#ifndef _MY_TIME_H
#define _MY_TIME_H

#include "types.h"

typedef struct Manager {
    
    // History of Iterative Deepening
    int values[MAX_DEPTH];
    uint16_t bestMoves[MAX_DEPTH];
    double timeUsage[MAX_DEPTH];
    int depth;
    
    // Base time of the search
    double startTime;
    
    // 3-Point Time Managment
    double idealUsage;
    double maxAlloc;
    double maxUsage;
    
    // PV stability factor for time changes
    double pvStability;
    
    // Time control UCI limits
    int depthLimit;
    double timeLimit;
    
    // Time control types
    int limitedByNone;
    int limitedByTime;
    int limitedByDepth;
    int limitedBySelf;
    
} Manager;

double getRealTime();

double getElapsedTime(double reference);

void initializeManager(Manager* manager, Limits* limits, double time, double mtg, double inc);

void updateManager(Manager* manager, int depth, int value, uint16_t bestMove);

int terminateSearchHere(Manager* manager);

int searchTimeHasExpired(Thread* thread);

#endif
