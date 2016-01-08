#include <stdio.h>
#include <stdint.h>

#include "types.h"
#include "move.h"
#include "piece.h"

void apply_move(Board * board, uint16_t move, Undo * undo){
	int to, from, cap;
	int fromtype, totype;
	
	switch(MOVE_TYPE(move)){
		case NormalMove:
			to = MOVE_TO(move);
			from = MOVE_FROM(move);
			
			fromtype = PIECE_TYPE(board->squares[from]);
			totype = PIECE_TYPE(board->squares[to]);
			
			// Initalize Undo
			undo->capture_sq = to;
			undo->capture_piece = board->squares[to];
			undo->turn = board->turn;
			undo->castlerights = board->castlerights;
			undo->opening = board->opening;
			undo->endgame = board->endgame;
			undo->hash = board->hash;
			
			// Adjust Colour BitBoards
			board->colourBitBoards[board->turn] ^= 1ull << from;
			board->colourBitBoards[board->turn] |= 1ull << to;
			board->colourBitBoards[PIECE_COLOUR(undo->capture_piece)] ^= 1ull << to;
			
			// Adjust Piece BitBoards
			board->pieceBitBoards[totype] ^= 1ull << to;
			board->pieceBitBoards[fromtype] ^= 1ull << from;
			board->pieceBitBoards[fromtype] |= 1ull << to;
			
			board->squares[to] = board->squares[from];
			board->squares[from] = Empty;
			
			board->turn = !board->turn;
			return;
		
		case CastleMove: break;
		
		case EnpassMove: break;
		
		case PromotionMove: break;
	}
	
	printf("TYPE = %d\n",MOVE_TYPE(move));
}

void revert_move(Board * board, uint16_t move, Undo * undo){
	int to, from;
	int fromtype, totype;
	
	switch(MOVE_TYPE(move)){
		case NormalMove:
			to = MOVE_TO(move);
			from = MOVE_FROM(move);
			
			fromtype = PIECE_TYPE(board->squares[to]);
			totype = PIECE_TYPE(undo->capture_piece);
			
			board->colourBitBoards[undo->turn] |= 1ull << from;
			board->colourBitBoards[undo->turn] ^= 1ull << to;
			board->colourBitBoards[PIECE_COLOUR(undo->capture_piece)] |= 1ull << to;
			
			board->pieceBitBoards[fromtype] ^= 1ull << to;
			board->pieceBitBoards[fromtype] |= 1ull << from;
			board->pieceBitBoards[totype] |= 1ull << to;
			
			board->squares[from] = board->squares[to];
			board->squares[to] = undo->capture_piece;
			
			board->castlerights = undo->castlerights;
			board->turn = undo->turn;
			board->opening = undo->opening;
			board->endgame = undo->endgame;
			board->hash = undo->hash;
			return;
			
	}
}