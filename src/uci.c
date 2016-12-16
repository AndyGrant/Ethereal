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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "board.h"
#include "magics.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "movegentest.h"
#include "piece.h"
#include "psqt.h"
#include "search.h"
#include "time.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"
#include "zorbist.h"

int main(){
    
    initalizeMagics();
    initalizeZorbist();
    initalizePSQT();
    initalizeMasks();
    
    char * startPos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    char str[2048];
    char * ptr;
    
    char moveStr[6], testStr[6];
    uint16_t moves[MAX_MOVES];
    int size;
    Undo undo;
    
    SearchInfo info;
    initalizeBoard(&(info.board), startPos);
    
    initalizeTranspositionTable(&Table, 22);
    
    while (1){
        
        getInput(str);
        
        if (stringEquals(str, "uci")){
            printf("id name Ethereal7.86\n");
            printf("id author Andrew Grant\n");
            printf("uciok\n");
            fflush(stdout);
        }
        
        else if (stringEquals(str, "movegentest")){
            moveGenTest();
        }
        
        else if (stringStartsWith(str, "debug")){
            // NOT IMPLEMENTED
        } 
        
        else if (stringEquals(str, "isready")){
            printf("readyok\n");
            fflush(stdout);
        } 
        
        else if (stringStartsWith(str, "setoption")){
            // NOT IMPLEMENTED
        } 
        
        else if (stringEquals(str, "register")){
            // NOT NEEDED
        } 
        
        else if (stringEquals(str, "ucinewgame")){
            clearTranspositionTable(&Table);
        } 
        
        else if (stringStartsWith(str, "position")){
            
            if (stringContains(str, "fen"))
                initalizeBoard(&(info.board), strstr(str, "fen") + 4);
            else if (stringContains(str, "startpos"))
                initalizeBoard(&(info.board), startPos);
            else
                exit(EXIT_FAILURE);
            
            ptr = strstr(str, "moves");
            if (ptr != NULL) ptr += 6;
            
            while (ptr != NULL && *ptr != '\0'){
                moveStr[0] = *ptr++;
                moveStr[1] = *ptr++;
                moveStr[2] = *ptr++;
                moveStr[3] = *ptr++;
                
                if (*ptr == '\0' || *ptr == ' ')
                    moveStr[4] = '\0';
                else{
                    moveStr[4] = *ptr++;
                    moveStr[5] = '\0';
                }
                
                size = 0;
                genAllMoves(&(info.board), moves, &size);
                
                for (size -= 1; size >= 0; size--){
                    moveToString(testStr, moves[size]);
                    
                    if (stringEquals(moveStr, testStr)){
                        applyMove(&(info.board), moves[size], &undo);
                        break;
                    }
                }
                
                if (size == -1)
                    exit(EXIT_FAILURE);
                
                while (*ptr == ' ')
                    ptr++;
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
            int mtg = -1;
            int depth = -1;
            double movetime = -1;
            int infinite = -1;
            
            for (ptr = strtok(NULL, " "); ptr != NULL; ptr = strtok(NULL, " ")){
                
                if (stringEquals(ptr, "searchmoves")){
                    // NOT IMPLEMENTED
                }
                
                else if (stringEquals(ptr, "ponder")){
                    // NOT IMPLEMENTED
                }
                
                else if (stringEquals(ptr, "wtime")){
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
                    mtg = atoi(ptr);
                }
                
                else if (stringEquals(ptr, "depth")){
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL) exit(EXIT_FAILURE);
                    depth = atoi(ptr);
                }
                
                else if (stringEquals(ptr, "nodes")){
                    // NOT IMPLEMENTED
                }
                
                else if (stringEquals(ptr, "mate")){
                    // NOT IMPLEMENTED
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
            
            if (info.board.turn == WHITE){
                time = wtime;
                inc = winc;
            } else {
                time = btime;
                inc = binc;
            }
            
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
                    info.endTime1 = info.startTime + .5 * (time / (double)(30));
                    info.endTime2 = info.startTime + (time / (double)(30)) + inc;
                }
                
                // USING REPEATING TIME CONTROL
                else {
                    info.endTime1 = info.startTime + .5 * (time / (double)(mtg+2));
                    info.endTime2 = info.startTime + (time / (double)(mtg+2));
                }
            }
            
            moveToString(moveStr, getBestMove(&info));
            printf("bestmove %s\n",moveStr);
            fflush(stdout);
        }
        
        else if (stringEquals(str, "stop")){
            info.terminateSearch = 1;
        } 
        
        else if (stringEquals(str, "ponderhit")){
            // NOT IMPLEMENTED
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
    
    if (fgets(str, 2048, stdin) == NULL)
        exit(EXIT_FAILURE);
    
    char * ptr = strchr(str, '\n');
    if (ptr != NULL) *ptr = '\0';
    
    ptr = strchr(str, '\r');
    if (ptr != NULL) *ptr = '\0';
}

void moveToString(char * str, uint16_t move){
    
    static char promoteDict[4] = {'n', 'b', 'r', 'q'};
    
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
        str[4] = promoteDict[move >> 14];
        str[5] = '\0';
    } else
        str[4] = '\0';
}