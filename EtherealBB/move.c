#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "board.h"
#include "castle.h"
#include "types.h"
#include "move.h"
#include "piece.h"
#include "zorbist.h"

void apply_move(Board * board, uint16_t move, Undo * undo){
	int to, from;
	int rto, rfrom;
	int fromtype, totype;
	int promotype, ep;
	
	int frompiece, topiece;
	int rfrompiece, enpasspiece;
	int promopiece;
	
	uint64_t shiftfrom, shiftto;
	uint64_t rshiftfrom, rshiftto;
	uint64_t shiftep;
	
	undo->epsquare = board->epsquare;
	undo->turn = board->turn;
	undo->castlerights = board->castlerights;
	undo->opening = board->opening;
	undo->endgame = board->endgame;
	undo->hash = board->hash;
	
	if (MOVE_TYPE(move) == NormalMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
		
		frompiece = board->squares[from];
		topiece = board->squares[to];
		
		fromtype = PIECE_TYPE(frompiece);
		totype = PIECE_TYPE(topiece);
		
		board->colourBitBoards[board->turn] ^= shiftfrom | shiftto;
		board->colourBitBoards[PIECE_COLOUR(topiece)] ^= shiftto;
		
		board->pieceBitBoards[totype] ^= shiftto;
		board->pieceBitBoards[fromtype] ^= shiftfrom | shiftto;		
		
		undo->capture_piece = topiece;
		board->squares[to] = frompiece;
		board->squares[from] = Empty;
		
		board->castlerights &= CastleMask[from];
		board->turn = !board->turn;
		
		board->hash ^= ZorbistKeys[frompiece][from]
					^  ZorbistKeys[frompiece][to]
					^  ZorbistKeys[topiece][to];
		
		board->epsquare = ((!fromtype) & (abs(to-from) == 16)) * (from + ((to-from)>>1));
		return;
	}
	
	if (MOVE_TYPE(move) == CastleMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		
		rto = CASTLE_GET_ROOK_TO(from,to);
		rfrom = CASTLE_GET_ROOK_FROM(from,to);
		
		frompiece = WhiteKing + board->turn;
		rfrompiece = frompiece - 8;
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
		rshiftfrom = 1ull << rfrom;
		rshiftto = 1ull << rto;
		
		board->colourBitBoards[board->turn] ^= shiftto | shiftfrom | rshiftto | rshiftfrom;
		
		board->pieceBitBoards[5] ^= shiftfrom | shiftto;
		board->pieceBitBoards[3] ^= rshiftfrom | rshiftto;
		
		board->squares[to] = frompiece;
		board->squares[from] = Empty;
		
		board->squares[rto] = rfrompiece;
		board->squares[rfrom] = Empty;
		
		board->castlerights &= CastleMask[from];
		board->turn = !board->turn;
		
		board->hash ^= ZorbistKeys[frompiece][from]
					^  ZorbistKeys[frompiece][to]
					^  ZorbistKeys[rfrompiece][rfrom]
					^  ZorbistKeys[rfrompiece][rto];
		
		board->epsquare = -1;
		return;
	}
	
	if (MOVE_TYPE(move) == PromotionMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		
		fromtype = PIECE_TYPE(board->squares[from]);
		totype = PIECE_TYPE(board->squares[to]);
		promotype = 1 + (move >> 14);
		
		frompiece = board->squares[from];
		topiece = board->squares[to];
		promopiece = (promotype << 2) + board->turn;
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
	
		board->colourBitBoards[board->turn] ^= shiftfrom | shiftto;
		board->colourBitBoards[PIECE_COLOUR(topiece)] ^= shiftto;
		
		board->pieceBitBoards[0] ^= shiftfrom;
		board->pieceBitBoards[promotype] ^= shiftto;
		board->pieceBitBoards[totype] ^= shiftto;
		
		undo->capture_piece = topiece;
		board->squares[to] = promopiece;
		board->squares[from] = Empty;
		
		board->turn = !board->turn;
		
		board->hash ^= ZorbistKeys[frompiece][from]
					^  ZorbistKeys[promopiece][to]
					^  ZorbistKeys[topiece][to];
		
		board->epsquare = -1;		
		return;
	}
	
	if (MOVE_TYPE(move) == EnpassMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		//ep = board->epsquare - (board->turn == ColourWhite ? 8 : -8);
		ep = board->epsquare - 8 + (board->turn << 4);
		
		frompiece = WhitePawn + board->turn;
		enpasspiece = WhitePawn + !board->turn;
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
		shiftep = 1ull << ep;
		
		board->colourBitBoards[!board->turn] ^= shiftep;
		board->pieceBitBoards[0] ^= shiftep;
		
		board->colourBitBoards[board->turn] ^= shiftfrom | shiftto;
		board->pieceBitBoards[0] ^= shiftfrom | shiftto;
		
		board->squares[to] = frompiece;
		board->squares[from] = Empty;
		
		undo->capture_piece = enpasspiece;
		board->squares[ep] = Empty;
		
		board->turn = !board->turn;
		
		board->hash ^= ZorbistKeys[frompiece][from]
					^ ZorbistKeys[frompiece][to]
					^ ZorbistKeys[enpasspiece][ep];
		
		board->epsquare = -1;
		return;
	}
}

void revert_move(Board * board, uint16_t move, Undo * undo){
	int to, from;
	int rto, rfrom;
	int fromtype, totype;
	int promotype, ep;
	uint64_t shiftfrom, shiftto;
	uint64_t rshiftfrom, rshiftto;
	uint64_t shiftep;
	
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
		board->epsquare = undo->epsquare;
		board->opening = undo->opening;
		board->endgame = undo->endgame;
		board->hash = undo->hash;
		return;
	}
	
	if (MOVE_TYPE(move) == CastleMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);

		rto = CASTLE_GET_ROOK_TO(from,to);
		rfrom = CASTLE_GET_ROOK_FROM(from,to);
		
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
		board->epsquare = undo->epsquare;
		board->opening = undo->opening;
		board->endgame = undo->endgame;
		board->hash = undo->hash;
		return;
	}
	
	if (MOVE_TYPE(move) == PromotionMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		
		fromtype = WhitePawn + undo->turn;
		totype = PIECE_TYPE(undo->capture_piece);
		promotype = 1 + (move >> 14);
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
	
		board->colourBitBoards[undo->turn] ^= shiftfrom | shiftto;
		board->colourBitBoards[PIECE_COLOUR(undo->capture_piece)] ^= shiftto;
		
		board->pieceBitBoards[0] ^= shiftfrom;
		board->pieceBitBoards[promotype] ^= shiftto;
		board->pieceBitBoards[totype] ^= shiftto;
		
		board->squares[to] = undo->capture_piece;
		board->squares[from] = WhitePawn + undo->turn;
		
		board->turn = undo->turn;
		board->epsquare = undo->epsquare;
		board->opening = undo->opening;
		board->endgame = undo->endgame;
		board->hash = undo->hash;
		return;
	}
	
	if (MOVE_TYPE(move) == EnpassMove){
		to = MOVE_TO(move);
		from = MOVE_FROM(move);
		ep = undo->epsquare - (undo->turn == ColourWhite ? 8 : -8);
		
		shiftfrom = 1ull << from;
		shiftto = 1ull << to;
		shiftep = 1ull << ep;
		
		
		board->colourBitBoards[!undo->turn] ^= shiftep;
		board->pieceBitBoards[0] ^= shiftep;
		
		board->colourBitBoards[undo->turn] ^= shiftfrom | shiftto;
		board->pieceBitBoards[0] ^= shiftfrom | shiftto;
		
		board->squares[from] = board->squares[to];
		board->squares[to] = Empty;
		board->squares[ep] = undo->capture_piece;
		
		board->turn = undo->turn;
		board->epsquare = undo->epsquare;
		board->opening = undo->opening;
		board->endgame = undo->endgame;
		board->hash = undo->hash;
		return;
	}
}