#include <stdio.h>
#include <stdint.h>

#include "castle.h"
#include "types.h"
#include "move.h"
#include "piece.h"

void apply_move(Board * board, uint16_t move, Undo * undo){
	int to, from;
	int rto, rfrom;
	int fromtype, totype;
	uint64_t shiftfrom, shiftto;
	uint64_t rshiftfrom, rshiftto;
	
	if (MOVE_TYPE(move) == NormalMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		
		fromtype = PIECE_TYPE(board->squares[from]);
		totype = PIECE_TYPE(board->squares[to]);
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
		
		undo->capture_sq = to;
		undo->capture_piece = board->squares[to];
		undo->turn = board->turn;
		undo->castlerights = board->castlerights;
		undo->opening = board->opening;
		undo->endgame = board->endgame;
		undo->hash = board->hash;
	
		board->colourBitBoards[board->turn] ^= shiftfrom | shiftto;
		board->colourBitBoards[PIECE_COLOUR(undo->capture_piece)] ^= shiftto;
		
		board->pieceBitBoards[totype] ^= shiftto;
		board->pieceBitBoards[fromtype] ^= shiftfrom | shiftto;		
		
		board->squares[to] = board->squares[from];
		board->squares[from] = Empty;
		
		board->castlerights &= CastleMask[from];
		board->turn = !board->turn;
		return;
	}
	
	if (MOVE_TYPE(move) == CastleMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		rto = to + (to-from == 2 ? -1 : 1);
		rfrom = from + (to-from == 2 ? 3 : -4);
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
		rshiftfrom = 1ull << rfrom;
		rshiftto = 1ull << rto;
		
		undo->turn = board->turn;
		undo->castlerights = board->castlerights;
		undo->opening = board->opening;
		undo->endgame = board->endgame;
		undo->hash = board->hash;
	
		board->colourBitBoards[board->turn] ^= shiftto | shiftfrom | rshiftto | rshiftfrom;
		
		board->pieceBitBoards[5] ^= shiftfrom | shiftto;
		board->pieceBitBoards[3] ^= rshiftfrom | rshiftto;
		
		board->squares[to] = board->squares[from];
		board->squares[from] = Empty;
		
		board->squares[rto] = board->squares[rfrom];
		board->squares[rfrom] = Empty;
		
		board->castlerights &= CastleMask[from];
		board->turn = !board->turn;
		return;
	}
}

void revert_move(Board * board, uint16_t move, Undo * undo){
	int to, from;
	int rto, rfrom;
	int fromtype, totype;
	uint64_t shiftfrom, shiftto;
	uint64_t rshiftfrom, rshiftto;
	
	if (MOVE_TYPE(move) == NormalMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		
		fromtype = PIECE_TYPE(board->squares[to]);
		totype = PIECE_TYPE(undo->capture_piece);
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
		
		board->colourBitBoards[undo->turn] ^= shiftfrom | shiftto;
		board->colourBitBoards[PIECE_COLOUR(undo->capture_piece)] |= shiftto;
		
		board->pieceBitBoards[fromtype] ^= shiftto | shiftfrom;
		board->pieceBitBoards[totype] |= shiftto;
		
		board->squares[from] = board->squares[to];
		board->squares[to] = undo->capture_piece;
		
		board->castlerights = undo->castlerights;
		board->turn = undo->turn;
		board->opening = undo->opening;
		board->endgame = undo->endgame;
		board->hash = undo->hash;
		return;
	}
	
	if (MOVE_TYPE(move) == CastleMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		rto = to + (to-from == 2 ? -1 : 1);
		rfrom = from + (to-from == 2 ? 3 : -4);
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
		rshiftfrom = 1ull << rfrom;
		rshiftto = 1ull << rto;
	
		board->colourBitBoards[undo->turn] ^= shiftfrom | shiftto | rshiftto | rshiftfrom;
		
		board->pieceBitBoards[5] ^= shiftfrom | shiftto;
		board->pieceBitBoards[3] ^= rshiftfrom | rshiftto;
		
		board->squares[from] = board->squares[to];
		board->squares[to] = Empty;
		
		board->squares[rfrom] = board->squares[rto];
		board->squares[rto] = Empty;
		
		board->castlerights = undo->castlerights;
		board->turn = undo->turn;
		board->opening = undo->opening;
		board->endgame = undo->endgame;
		board->hash = undo->hash;
		return;
	}
}