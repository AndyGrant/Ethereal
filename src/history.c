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


/**
 * Reduce the History counters for a new search in the same
 * game. Throwing away history between searches is a waste.
 *
 * @param   history HistoryTable struct to reduce
 */
void reduceHistory(HistoryTable history){
    
    int colour, from, to, counter;
    
    // Divide each history entry by 4
    for (colour = 0; colour < COLOUR_NB; colour++)
        for (from = 0; from < SQUARE_NB; from++)
            for (to = 0; to < SQUARE_NB; to++)
                for (counter = 0; counter < 2; counter++){
                    history[colour][from][to][counter] >>= 2;
                    history[colour][from][to][counter]++;
                }
}


/**
 * Reset the History counters for a new game
 *
 * @param   history HistoryTable struct to clear
 */
void clearHistory(HistoryTable history){
    
    int colour, from, to, counter;
    
    // Fill all History entries with ones. By using
    // ones instead of zeros we avoid division by zero
    for (colour = 0; colour < COLOUR_NB; colour++)
        for (from = 0; from < SQUARE_NB; from++)
            for (to = 0; to < SQUARE_NB; to++)
                for (counter = 0; counter < 2; counter++)
                    history[colour][from][to][counter] = 1;
}

/**
 * Update the History of a particular move
 *
 * @param   history HistoryTable containing the move
 * @param   move    The move we are updating
 * @param   colour  Colour of player who made the move
 * @param   isGood  Whether or not the move was the best
 * @param   delta   Amount to increment by
 */
void updateHistory(HistoryTable history, uint16_t move, int colour, int isGood,
                                                                    int delta){
    
    int from = MoveFrom(move);
    int to = MoveTo(move);
    
    if (isGood) history[colour][from][to][HISTORY_GOOD] += delta;
    history[colour][from][to][HISTORY_TOTAL] += delta;
    
    // Divide counters by two if we have exceeded the max values
    if (history[colour][from][to][HISTORY_TOTAL] >= HISTORY_MAX){
        history[colour][from][to][HISTORY_GOOD] >>= 1;
        history[colour][from][to][HISTORY_TOTAL] >>= 1;
    }
}

/**
 * Fetch the History of particular move
 *
 * @param   history HistoryTable containing the move
 * @param   move    The move we are updating
 * @param   colour  Colour of player who made the move
 * @param   factor  Maximum value of history score
 */
int getHistoryScore(HistoryTable history, uint16_t move, int colour,
                                                        int factor){
    
    int from = MoveFrom(move);
    int to = MoveTo(move);
    
    int good = history[colour][from][to][HISTORY_GOOD];
    int total = history[colour][from][to][HISTORY_TOTAL];
    
    return (factor * good) / total;
}