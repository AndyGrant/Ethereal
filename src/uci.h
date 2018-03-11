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

#ifndef _UCI_H
#define _UCI_H

#include "types.h"

typedef struct Limits {
    int limitedByNone;
    int limitedByTime;
    int limitedByDepth;
    int limitedBySelf;
    double timeLimit;
    int depthLimit;
} Limits;

typedef struct ThreadsGo {
    char str[512];
    Thread* threads;
    Board* board;
} ThreadsGo;

void getInput(char* str);
int stringEquals(char* s1, char* s2);
int stringStartsWith(char* str, char* key);
int stringContains(char* str, char* key);
void moveToString(char* str, uint16_t move);

void* uciGo(void* vthreadgo);
void uciPosition(char* str, Board* board);
void uciReport(Thread* threads, double startTime, int depth, int value, PVariation* pv);

#endif
