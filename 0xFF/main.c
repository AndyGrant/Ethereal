#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "board.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "util.h"


int debug(board_t board){
	int pos[256];
	int sq[256];
	int piecel[2][32];
	int pawnl[2][16];
	int piecec[2];
	int pawnc[2];
	
	memcpy(&pos,board.positions,sizeof(board.positions));
	memcpy(&sq,board.squares,sizeof(board.squares));
	memcpy(&piecel,board.piece_locations,sizeof(board.piece_locations));
	memcpy(&pawnl,board.pawn_locations,sizeof(board.pawn_locations));
	memcpy(&piecec,board.piece_counts,sizeof(board.piece_counts));
	memcpy(&pawnc,board.pawn_counts,sizeof(board.pawn_counts));
	
	int size = 0; 
	move_t moves[MaxMoves];
	gen_all_moves(&board,&(moves[0]),&size);
	
	
	int i;
	for(i = 0; i < size; i++){
		print_move_t(moves[i]);

		apply_move(&board,moves[i]);		
		revert_move(&board,moves[i]);	
	
		if (memcmp(&sq,board.squares,sizeof(board.squares)) != 0) printf("squares\n");
		if (memcmp(&piecel,board.piece_locations,sizeof(board.piece_locations)) != 0) printf("piece_locations\n");
		if (memcmp(&pawnl,board.pawn_locations,sizeof(board.pawn_locations)) != 0) printf("pawn_locations\n");
		if (memcmp(&piecec,board.piece_counts,sizeof(board.piece_counts)) != 0) printf("piece_counts\n");
		if (memcmp(&pawnc,board.pawn_counts,sizeof(board.pawn_counts)) != 0) printf("pawn_counts\n");
		if (memcmp(&pos,board.positions,sizeof(board.positions)) != 0) printf("positions\n");
	}
}

int main(){
	clock_t start = clock();
	
	board_t board;
	//init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	//init_board_t(&board,"reeekeereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeKeee11110000");
	//init_board_t(&board,"reeekeereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeReeQKeeR11110000");
	//init_board_t(&board,"breeeeekPPPeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeepppKeeeeeeN00000000");
	//init_board_t(&board,"rnbqkbnrppppppppeeeeeeeeeeePePeeeeepepeeeeeeeeeePPPPPPPPRNBQKBNR11110000");
	init_board_t(&board,"reeekeerpeppqpbebneepnpeeeePNeeeepeePeeeeeNeeQepPPPBBPPPReeeKeeR11110000");
	
	//  init_board_t(&board,"rnbqkbnrppppepppeeeeeeeeeeePpeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11111200");
	
	/*int size = 0;
	move_t moves[MaxMoves];
	gen_all_moves(&board,&(moves[0]),&size);
	print_board_t(&board);
	
	
	int x, y;
	printf("PAWN BEFORE\n");
	for(x = 0; x < 2; x++){
		printf("Player %d : ",x);
		for(y = 0; y < 16; y++)
			printf("%d ",board.pawn_locations[x][y]);
		printf("\n");
	}
	
	printf("PIECE BEFORE\n");
	for(x = 0; x < 2; x++){
		printf("Player %d : ",x);
		for(y = 0; y < 16; y++)
			printf("%d ",board.piece_locations[x][y]);
		printf("\n");
	}
	
	apply_move(&board,moves[5]);
	
	printf("PAWN After\n");
	for(x = 0; x < 2; x++){
		printf("Player %d : ",x);
		for(y = 0; y < 16; y++)
			printf("%d ",board.pawn_locations[x][y]);
		printf("\n");
	}
	
	printf("PIECE After\n");
	for(x = 0; x < 2; x++){
		printf("Player %d : ",x);
		for(y = 0; y < 16; y++)
			printf("%d ",board.piece_locations[x][y]);
		printf("\n");
	}

				
	print_board_t(&board);
	
	return;
	revert_move(&board,moves[5]);
	print_board_t(&board);
	
	return;*/
	int i;
	for(i = 0; i < 10; i++)
		printf("Depth %d : %d\n",i,perft(&board,i));
	
	
	printf("Time Taken=%d\n",(int)((clock()-start)/CLOCKS_PER_SEC));
}