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
#include <string.h>
#include <time.h>

#include "move.h"
#include "board.h"
#include "types.h"

/**
 * Read all of the test cases found in perfttests.txt. Each
 * test case contains a FEN string for the board setup, as 
 * well as perft counts for various depths. Complete all of
 * these tests and print out some information about how many
 * tests were passed, failed, how many nodes were found, the 
 * frequency of nodes, as well as the total time taken. If
 * any cases fail these tests there are serious problems with
 * the movegen algorithms.
 */
void moveGenTest(){
    
    int i, j, depth;
    int passed = 0, failed = 0;
    uint64_t total = 0, found, nodes[128];
    
    char c, str[128];
    clock_t start = clock(), end;
    FILE * input = fopen("perfttests.txt","r");
    
    while(1){
        
        Board board;
        for(i = 0;; i++){
            j = fgetc(input);
            c = (char)(j);
            if (j == EOF) goto EndOfMainLoop;
            if (c == ';') break;
            str[i] = c;
        }
        
        str[i++] = '\0';
        
        initalizeBoard(&board,str);
        
        depth = 0;
        nodes[0] = 0;
        
        while(1){
            c = fgetc(input);
            
            if (c == 'D'){
                fgetc(input);
                fgetc(input);
            }
            
            if (c >= '0' && c <= '9')
                nodes[depth] = 10 * nodes[depth] + c - '0';
            
            if (c == ' '){
                depth++;
                nodes[depth] = 0;
            }
            
            if (c == '\n'){
                depth++;
                break;
            }
        }
        
        for(i = 1; i <= depth; i++){
            found = perft(&board,i);
            total += found;
            
            if (found == nodes[i-1]){
                printf("PASSED %s %d of %d\n",str,(int)(found),(int)(nodes[i-1]));
                passed++;
            } else {
                printf("FAILED %s %d of %d\n",str,(int)(found),(int)(nodes[i-1]));
                failed++;
            }
        }
    }
    
    EndOfMainLoop:
    
    end = clock();
    
    printf("\n\n");
    printf("Passed : %d\n",passed);
    printf("Failed : %d\n",failed);
    printf("Nodes  : %d\n",(int)(total));
    printf("Seconds: %f\n",(double)(end - start) / CLOCKS_PER_SEC);
    printf("MNPS   : %.3f",(total/(float)(1000000 * ((end-start)/CLOCKS_PER_SEC))));
}
