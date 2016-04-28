#include <stdio.h>
#include <string.h>
#include <time.h>

#include "move.h"
#include "board.h"
#include "types.h"

void move_gen_test(){
    int i, depth;
    int passed = 0, failed = 0;
    uint64_t total = 0, found, nodes[128];
    
    char c, str[128];
    clock_t start = clock(), end;
    FILE * input = fopen("perfttests.txt","r");
    
    while(1){
        
        Board board;
        for(i = 0;; i++){
            c = fgetc(input);
            if (c == EOF) goto END_OF_MAIN_LOOP;
            if (c == ';') break;
            str[i] = c;
            
        }
        
        str[i++] = '\0';
        
        init_board(&board,str);
        
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
                printf("PASSED %s %llu of %llu\n",str,found,nodes[i-1]);
                passed++;
            } else {
                printf("FAILED %s %llu of %llu\n",str,found,nodes[i-1]);
                failed++;
            }
        }
    }
    
    END_OF_MAIN_LOOP:
    
    end = clock();
    
    printf("\n\n");
    printf("Passed : %d\n",passed);
    printf("Failed : %d\n",failed);
    printf("Nodes  : %llu\n",total);
    printf("Seconds: %d\n",(end - start) / CLOCKS_PER_SEC);
    printf("MNPS   : %.3f",(total/(float)(1000000 * ((end-start)/CLOCKS_PER_SEC))));
}