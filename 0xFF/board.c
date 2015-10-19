#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "castle.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "types.h"
#include "util.h"

char BaseBoard[73] = "rbnqknbrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRBNQKNBR11110000";

void init_board_t(board_t * board, char setup[73]){
	int i, sq, flag;
	
	for(sq = 0; sq < 256; sq++){
		board->positions[sq] = -1;
		board->squares[sq] = Wall;
	}

	for(sq = 0; sq < 32; sq++){
		board->piece_locations[ColourWhite][sq] = -1;
		board->piece_locations[ColourBlack][sq] = -1;
	}
	
	board->piece_counts[ColourWhite] = 0;
	board->piece_counts[ColourBlack] = 0;

	for(sq = 0; sq < 16; sq++){
		board->pawn_locations[ColourWhite][sq] = -1;
		board->pawn_locations[ColourBlack][sq] = -1;
	}
	
	board->pawn_counts[ColourWhite] = 0;
	board->pawn_counts[ColourBlack] = 0;
	
	board->castle_rights = 0;
	
	for(i = 0; i < 64; i++){
		sq = CONVERT_64_TO_256(i);
		board->squares[sq] = char_to_piece(setup[i]);
	}
	
	for(flag = KingFlag; flag >= KnightFlag; flag = flag >> 1){
		for(i = 0; i < 64; i++){
			sq = CONVERT_64_TO_256(i);
			if (board->squares[sq] & flag){
				int colour = PIECE_COLOUR(board->squares[sq]);
				int num = board->piece_counts[colour];
				board->piece_locations[colour][num] = sq;
				board->positions[sq] = num;
				board->piece_counts[colour]++;
			}
		}
	}
	
	flag = PawnFlag;
	for(i = 0; i < 64; i++){
		sq = CONVERT_64_TO_256(i);
		if (board->squares[sq] & flag){
			int colour = PIECE_COLOUR(board->squares[sq]);
			int num = board->pawn_counts[colour];
			board->pawn_locations[colour][num] = sq;
			board->positions[sq] = num;
			board->pawn_counts[colour]++;
		}
	}
	
	board->castle_rights = CREATE_CASTLE_RIGHTS(setup[64]-'0',setup[65]-'0',setup[66]-'0',setup[67]-'0');
	board->ep_square = (100*(setup[68]-'0'))+(10 * (setup[69]-'0'))+(setup[70]-'0');
	board->turn = (setup[71] - '0');
}