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

#include "board.h"
#include "history.h"
#include "move.h"
#include "types.h"


void clearHistory(HistoryTable history){
    
    int colour, from, to, counter;
    
    for (colour = 0; colour < COLOUR_NB; colour++)
        for (from = 0; from < SQUARE_NB; from++)
            for (to = 0; to < SQUARE_NB; to++)
                for (counter = 0; counter < 2; counter++)
                    history[colour][from][to][counter] = 1;
}

void updateHistory(HistoryTable history, uint16_t move, int colour, int isGood, int delta){
    
    int from = MoveFrom(move);
    int to = MoveTo(move);
    
    if (isGood) history[colour][from][to][HISTORY_GOOD] += delta;
    history[colour][from][to][HISTORY_TOTAL] += delta;
    
    if (history[colour][from][to][HISTORY_TOTAL] >= HISTORY_MAX){
        history[colour][from][to][HISTORY_GOOD] >>= 1;
        history[colour][from][to][HISTORY_TOTAL] >>= 1;
    }
}

int getHistoryScore(HistoryTable history, uint16_t move, int colour, int factor){
    
    int from = MoveFrom(move);
    int to = MoveTo(move);
    
    int good = history[colour][from][to][HISTORY_GOOD];
    int total = history[colour][from][to][HISTORY_TOTAL];
    
    return (factor * good) / total;
}