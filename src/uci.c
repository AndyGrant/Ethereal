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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "history.h"
#include "magics.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "piece.h"
#include "psqt.h"
#include "search.h"
#include "tests.h"
#include "texel.h"
#include "time.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"
#include "zorbist.h"

extern TransTable Table;
extern HistoryTable History;

char * startPos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int main(){
    
    int size, megabytes;
    Undo undo[1];
    SearchInfo info;
    uint16_t moves[MAX_MOVES];
    char str[8192], moveStr[6], testStr[6], * ptr;
    
    // Initalze all components of the chess engine
    initalizeMagics();
    initalizeZorbist();
    initalizePSQT();
    initalizeMasks();
    initalizeBoard(&(info.board), startPos);
    initalizeTranspositionTable(&Table, 16);
    clearHistory(History);
    
    #ifdef TUNE
        runTexelTuning();
        exit(0);
    #endif
    
    while (1){
        
        getInput(str);
        
        /* Non Universal Chess Interface commands */
        
        if (stringEquals(str, "runTestSuite")){
            runTestSuite();
        }
        
        else if (stringStartsWith(str, "perft")){
            printf("%"PRIu64"\n", perft(&info.board, atoi(str + 6)));
            fflush(stdout);
        }
        
        else if (stringStartsWith(str, "bench")){
            runBenchmark(atoi(str + 6));
        }
        
        /* Universal Chess Interface commands
           Full documentation can be found here:
           http://wbec-ridderkerk.nl/html/UCIProtocol.html */
            
        if (stringEquals(str, "uci")){
            printf("id name Ethereal 8.40\n");
            printf("id author Andrew Grant\n");
            printf("option name Hash type spin default 16 min 1 max 2048\n");
            printf("uciok\n");
            fflush(stdout);
        }
        
        else if (stringEquals(str, "isready")){
            printf("readyok\n");
            fflush(stdout);
        } 
        
        else if (stringStartsWith(str, "setoption")){
            
            if (stringStartsWith(str, "setoption name Hash value")){
                megabytes = atoi(str + strlen("setoption name Hash value"));
                destroyTranspositionTable(&Table);
                initalizeTranspositionTable(&Table, megabytes);
            }
        }
        
        else if (stringEquals(str, "ucinewgame")){
            clearHistory(History);
            clearTranspositionTable(&Table);
        } 
        
        else if (stringStartsWith(str, "position")){
            
            // Determine the form of the position command
            if (stringContains(str, "fen"))
                initalizeBoard(&info.board, strstr(str, "fen") + 4);
            else if (stringContains(str, "startpos"))
                initalizeBoard(&info.board, startPos);
            else
                exit(EXIT_FAILURE);
            
            ptr = strstr(str, "moves");
            if (ptr != NULL) ptr += 6;
            
            while (ptr != NULL && *ptr != '\0'){
                
                // Move is in long algebraic notation
                moveStr[0] = *ptr++;
                moveStr[1] = *ptr++;
                moveStr[2] = *ptr++;
                moveStr[3] = *ptr++;
                
                // NULL terminate a non promotional move
                if (*ptr == '\0' || *ptr == ' ')
                    moveStr[4] = '\0';
                
                // Move is a promotion move
                else{
                    moveStr[4] = *ptr++;
                    moveStr[5] = '\0';
                }
                
                size = 0;
                genAllMoves(&info.board, moves, &size);
                
                // Search for the matching move
                for (size -= 1; size >= 0; size--){
                    moveToString(testStr, moves[size]);
                    if (stringEquals(moveStr, testStr)){
                        applyMove(&info.board, moves[size], undo);
                        break;
                    }
                }
                
                // Move was unable to be found on the current board
                if (size == -1)
                    exit(EXIT_FAILURE);
                
                // Skip over all white space
                while (*ptr == ' ')
                    ptr++;
                
                // Reset the history of hashes if we just reset
                // the fifty move rule. This way, the numMoves
                // can never be greated than ~100 (128)
                if (info.board.fiftyMoveRule == 0)
                    info.board.numMoves = 0;
            }
        }
        
        else if (stringStartsWith(str, "go")){
            
            ptr = strtok(str, " ");
            
            double time = 0;
            double inc = 0;
            double wtime = -1;
            double btime = -1;
            double winc = 0;
            double binc = 0;
            double mtg = -1;
            int depth = -1;
            double movetime = -1;
            int infinite = -1;
            
            // Parse all of the parameters in the go command
            for (ptr = strtok(NULL, " "); ptr != NULL; ptr = strtok(NULL, " ")){
                
                if (stringEquals(ptr, "wtime")){
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL) exit(EXIT_FAILURE);
                    wtime = (double)(atoi(ptr));
                }
                
                else if (stringEquals(ptr, "btime")){
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL) exit(EXIT_FAILURE);
                    btime = (double)(atoi(ptr));
                }
                
                else if (stringEquals(ptr, "winc")){
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL) exit(EXIT_FAILURE);
                    winc = (double)(atoi(ptr));
                }
                
                else if (stringEquals(ptr, "binc")){
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL) exit(EXIT_FAILURE);
                    binc = (double)(atoi(ptr));
                }
                
                else if (stringEquals(ptr, "movestogo")){
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL) exit(EXIT_FAILURE);
                    mtg = (double)(atoi(ptr));
                }
                
                else if (stringEquals(ptr, "depth")){
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL) exit(EXIT_FAILURE);
                    depth = atoi(ptr);
                }
                
                else if (stringEquals(ptr, "movetime")){
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL) exit(EXIT_FAILURE);
                    movetime = (double)(atoi(ptr));
                }
                
                else if (stringEquals(ptr, "infinite")){
                    infinite = 1;
                }
            }
            
            
            // Pick the Time Controls for the colour we are playing
            time = (info.board.turn == WHITE) ? wtime : btime;
            inc  = (info.board.turn == WHITE) ?  winc :  binc;
            
            // Setup the default search information
            info.searchIsInfinite = 0;
            info.searchIsDepthLimited = 0;
            info.searchIsTimeLimited = 0;
            info.depthLimit = 0;
            info.terminateSearch = 0;
            info.startTime = getRealTime();
            
            if (infinite == 1){
                info.searchIsInfinite = 1;
            }
            
            else if (depth != -1){
                info.searchIsDepthLimited = 1;
                info.depthLimit = depth;
            }
            
            else if (movetime != -1){
                info.searchIsTimeLimited = 1;
                info.endTime1 = info.startTime + 20*movetime;
                info.endTime2 = info.startTime + movetime;
            }
            
            else {
                
                info.searchIsTimeLimited = 1;
                
                // NOT USING REPEATING TIME CONTROL
                if (mtg == -1){
                    info.endTime1 = info.startTime + .5 * (time / 30);
                    info.endTime2 = info.startTime + (time / 30) + inc;
                }
                
                // USING REPEATING TIME CONTROL
                else {
                    info.endTime1 = info.startTime + .5 * (time / (mtg+2));
                    info.endTime2 = info.startTime + (time / (mtg+2));
                }
            }
            
            // Execute the search and report the best move
            moveToString(moveStr, getBestMove(&info));
            printf("bestmove %s\n",moveStr);
            fflush(stdout);
        }
        
        else if (stringEquals(str, "quit")){
            destroyTranspositionTable(&Table);
            break;
        }
    }
    
    return 1;
}

int stringEquals(char * s1, char * s2){
    return strcmp(s1, s2) == 0;
}

int stringStartsWith(char * str, char * key){
    return strstr(str, key) == str;
}

int stringContains(char * str, char * key){
    return strstr(str, key) != NULL;
}

void getInput(char * str){
    
    char * ptr;
    
    if (fgets(str, 8192, stdin) == NULL)
        exit(EXIT_FAILURE);
    
    ptr = strchr(str, '\n');
    if (ptr != NULL) *ptr = '\0';
    
    ptr = strchr(str, '\r');
    if (ptr != NULL) *ptr = '\0';
}

void moveToString(char * str, uint16_t move){
    
    static char table[4] = {'n', 'b', 'r', 'q'};
    
    int from = MoveFrom(move);
    int to = MoveTo(move);
    
    char fromFile = '1' + (from / 8);
    char toFile = '1' + (to / 8);
    char fromRank = 'a' + (from % 8);
    char toRank = 'a' + (to % 8);
    
    str[0] = fromRank;
    str[1] = fromFile;
    str[2] = toRank;
    str[3] = toFile;
    
    if (MoveType(move) == PROMOTION_MOVE){
        str[4] = table[move >> 14];
        str[5] = '\0';
    } else
        str[4] = '\0';
}
