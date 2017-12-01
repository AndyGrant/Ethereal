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

#include <stdint.h>

#include "board.h"
#include "history.h"
#include "move.h"
#include "types.h"

enum { HISTORY_GOOD, HISTORY_TOTAL };

const uint64_t HistoryMax = 0x7FFFFFFF;

void reduceHistory(HistoryTable history){
    
    int c, f, t, i;
    
    for (c = 0; c < COLOUR_NB; c++)
        for (f = 0; f < SQUARE_NB; f++)
            for (t = 0; t < SQUARE_NB; t++)
                for (i = 0; i < 2; i++)
                    history[c][f][t][i] = 1 + history[c][f][t][i] / 4;
}

void clearHistory(HistoryTable history){
    
    int c, f, t, i;
    
    for (c = 0; c < COLOUR_NB; c++)
        for (f = 0; f < SQUARE_NB; f++)
            for (t = 0; t < SQUARE_NB; t++)
                for (i = 0; i < 2; i++)
                    history[c][f][t][i] = 1;
}

void updateHistory(HistoryTable history, uint16_t move, int colour, int isGood, int delta){
    
    int from = MoveFrom(move), to = MoveTo(move);
    
    // Update both counters by delta
    if (isGood) history[colour][from][to][HISTORY_GOOD] += delta;
    history[colour][from][to][HISTORY_TOTAL] += delta;
    
    // Divide the counters by two if we have exceeded the max value for history
    if (history[colour][from][to][HISTORY_TOTAL] >= HistoryMax){
        history[colour][from][to][HISTORY_GOOD] >>= 1;
        history[colour][from][to][HISTORY_TOTAL] >>= 1;
    }
}

int getHistoryScore(HistoryTable history, uint16_t move, int colour, int factor){
    
    // History is scored on a scale from 0 to factor, where factor is 100%
    // We should try to choose factor to be a power of two, as to avoid division
    int from  = MoveFrom(move);
    int to    = MoveTo(move);
    int good  = history[colour][from][to][HISTORY_GOOD ];
    int total = history[colour][from][to][HISTORY_TOTAL];
    return (factor * good) / total;
}
