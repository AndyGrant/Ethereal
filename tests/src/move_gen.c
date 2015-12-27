#include <stdio.h>
#include <time.h>

#include "../../src/board.h"
#include "../../src/util.h"

int main(){
	
	int i, depth;
	int passed = 0, failed = 0;
	unsigned long long total = 0, found, nodes[128];
	char c, str[73];
	board_t board;
	clock_t start = clock(), end;
	FILE * input = fopen("src/PerftTests.txt","r");
	
	while(1){
		
		// Copy the Encoded Board
		if (fgets(str, 73, input) == NULL)
			break;
		
		// Initialze Board
		init_board_t(&board,str);
		
		// Throw away extra space
		fgetc(input);
		
		// Reset node 0
		nodes[0] = 0;
		
		// Fill nodes until newline char is reached
		// Skip to next node on space character
		// Zero node++ before writing to it
		// Update Current node on numerical character
		for (i = 0;;){
			c = (char)fgetc(input);
			if (c == '\n')			break;
			else if (c == ' ')		nodes[++i] = 0;
			else					nodes[i] = (nodes[i]*10) + (c - '0');
		}
		
		// Perft on range depth -> i+2
		// Output success or failure
		// Update passed,failed counters
		for(depth = 1; depth <= i+1; depth++){
			found = perft(&board,depth);
			total += found;
			
			if (found == nodes[depth-1]){
				printf("PASSED %s %llu of %llu\n",str,found,nodes[depth-1]);
				passed++;
			} else {
				printf("FAILED %s %llu of %llu\n",str,found,nodes[depth-1]);
				failed++;
			}
		}
	}
	
	end = clock();
	
	printf("\n\n");
	printf("Passed : %d\n",passed);
	printf("Failed : %d\n",failed);
	printf("Nodes  : %llu\n",total);
	printf("Seconds: %d\n",(end - start) / CLOCKS_PER_SEC);
	printf("MNPS   : %.3f",(total/(float)(1000000 * ((end-start)/CLOCKS_PER_SEC))));
	
	return 0;
}