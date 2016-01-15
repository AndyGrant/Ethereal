#include <stdint.h>
#include <assert.h>

#include "bitboards.h"
#include "bitutils.h"
#include "castle.h"
#include "magics.h"
#include "move.h"
#include "movegen.h"
#include "piece.h"
#include "types.h"

void gen_all_moves(Board * board, uint16_t * moves, int * size){	
	uint64_t blockers;
	uint64_t attackable;
	
	uint64_t pawnforwardone;
	uint64_t pawnforwardtwo;
	uint64_t pawnleft;
	uint64_t pawnright;
	
	uint64_t pawnpromoforward;
	uint64_t pawnpromoleft;
	uint64_t pawnpromoright;
	
	int bit, lsb, dbindex;
	int forwardshift, leftshift, rightshift;
	
	uint64_t friendly = board->colourBitBoards[board->turn];
	uint64_t enemy = board->colourBitBoards[!board->turn];
	
	uint64_t empty = ~(friendly | enemy);
	uint64_t notempty = ~empty;
	uint64_t notfriendly = ~friendly;
	
	uint64_t mypawns   = friendly & board->pieceBitBoards[0];
	uint64_t myknights = friendly & board->pieceBitBoards[1];
	uint64_t mybishops = friendly & board->pieceBitBoards[2];
	uint64_t myrooks   = friendly & board->pieceBitBoards[3];
	uint64_t myqueens  = friendly & board->pieceBitBoards[4];
	uint64_t mykings   = friendly & board->pieceBitBoards[5];
	
	if (board->turn == ColourWhite){
		forwardshift = 8;
		leftshift = 7;
		rightshift = 9;
	} else {
		forwardshift = -8;
		leftshift = -7;
		rightshift = -9;
	}
	
	// Generate queen moves as if they were rooks and bishops
	mybishops |= myqueens;
	myrooks |= myqueens;
	
	// Generate Pawn BitBoards and Generate Enpass Moves
	if (board->turn == ColourWhite){
		pawnforwardone = (mypawns << 8) & empty;
		pawnforwardtwo = ((pawnforwardone & RANK_3) << 8) & empty;
		pawnleft = ((mypawns << 7) & ~FILE_A) & enemy;
		pawnright = ((mypawns << 9) & ~FILE_H) & enemy;
		
		pawnpromoforward = pawnforwardone & RANK_8;
		pawnpromoleft = pawnleft & RANK_8;
		pawnpromoright = pawnright & RANK_8;
		
		pawnforwardone &= ~RANK_8;
		pawnleft &= ~RANK_8;
		pawnright &= ~RANK_8;
		
		if(board->epsquare >= 40){
			if (board->squares[board->epsquare-7] == WhitePawn && board->epsquare != 47)
				moves[(*size)++] = MOVE_MAKE(board->epsquare-7,board->epsquare,EnpassMove);
			
			if (board->squares[board->epsquare-9] == WhitePawn && board->epsquare != 40)
				moves[(*size)++] = MOVE_MAKE(board->epsquare-9,board->epsquare,EnpassMove);
		}
		
	} else {
		pawnforwardone = (mypawns >> 8) & empty;
		pawnforwardtwo = ((pawnforwardone & RANK_6) >> 8) & empty;
		pawnleft = ((mypawns >> 7) & ~FILE_H) & enemy;
		pawnright = ((mypawns >> 9) & ~FILE_A) & enemy;
		
		pawnpromoforward = pawnforwardone & RANK_1;
		pawnpromoleft = pawnleft & RANK_1;
		pawnpromoright = pawnright & RANK_1;
		
		pawnforwardone &= ~RANK_1;
		pawnleft &= ~RANK_1;
		pawnright &= ~RANK_1;
		
		if(board->epsquare > 0 && board->epsquare < 40){
			if (board->squares[board->epsquare+7] == BlackPawn && board->epsquare != 16)
				moves[(*size)++] = MOVE_MAKE(board->epsquare+7,board->epsquare,EnpassMove);
			
			if (board->squares[board->epsquare+9] == BlackPawn && board->epsquare != 23)
				moves[(*size)++] = MOVE_MAKE(board->epsquare+9,board->epsquare,EnpassMove);
		}
	}
	
	// Generate Pawn Moves
	while(pawnforwardone != 0){
		lsb = get_lsb(pawnforwardone);
		moves[(*size)++] = MOVE_MAKE(lsb-forwardshift,lsb,NormalMove);
		pawnforwardone ^= 1ull << lsb;
	}
	
	while(pawnforwardtwo != 0){
		lsb = get_lsb(pawnforwardtwo);
		moves[(*size)++] = MOVE_MAKE(lsb-(2*forwardshift),lsb,NormalMove);
		pawnforwardtwo ^= 1ull << lsb;
	}
	
	while(pawnleft != 0){
		lsb = get_lsb(pawnleft);
		moves[(*size)++] = MOVE_MAKE(lsb-leftshift,lsb,NormalMove);
		pawnleft ^= 1ull << lsb;
	}
	
	while(pawnright != 0){
		lsb = get_lsb(pawnright);
		moves[(*size)++] = MOVE_MAKE(lsb-rightshift,lsb,NormalMove);
		pawnright ^= 1ull << lsb;
	}
	
	while(pawnpromoforward != 0){
		lsb = get_lsb(pawnpromoforward);
		moves[(*size)++] = MOVE_MAKE(lsb-forwardshift,lsb,PromotionMove|PromoteToQueen);
		moves[(*size)++] = MOVE_MAKE(lsb-forwardshift,lsb,PromotionMove|PromoteToRook);
		moves[(*size)++] = MOVE_MAKE(lsb-forwardshift,lsb,PromotionMove|PromoteToBishop);
		moves[(*size)++] = MOVE_MAKE(lsb-forwardshift,lsb,PromotionMove|PromoteToKnight);
		pawnpromoforward ^= 1ull << lsb;
	}
	
	while(pawnpromoleft != 0){
		lsb = get_lsb(pawnpromoleft);
		moves[(*size)++] = MOVE_MAKE(lsb-leftshift,lsb,PromotionMove|PromoteToQueen);
		moves[(*size)++] = MOVE_MAKE(lsb-leftshift,lsb,PromotionMove|PromoteToRook);
		moves[(*size)++] = MOVE_MAKE(lsb-leftshift,lsb,PromotionMove|PromoteToBishop);
		moves[(*size)++] = MOVE_MAKE(lsb-leftshift,lsb,PromotionMove|PromoteToKnight);
		pawnpromoleft ^= 1ull << lsb;
	}
	
	while(pawnpromoright != 0){
		lsb = get_lsb(pawnpromoright);
		moves[(*size)++] = MOVE_MAKE(lsb-rightshift,lsb,PromotionMove|PromoteToQueen);
		moves[(*size)++] = MOVE_MAKE(lsb-rightshift,lsb,PromotionMove|PromoteToRook);
		moves[(*size)++] = MOVE_MAKE(lsb-rightshift,lsb,PromotionMove|PromoteToBishop);
		moves[(*size)++] = MOVE_MAKE(lsb-rightshift,lsb,PromotionMove|PromoteToKnight);
		pawnpromoright ^= 1ull << lsb;
	}
	
	// Generate Knight Moves
	while(myknights != 0){
		bit = get_lsb(myknights);
		attackable = KnightMap[bit] & notfriendly;
		
		while(attackable != 0){
			lsb = get_lsb(attackable);
			moves[(*size)++] = MOVE_MAKE(bit,lsb,NormalMove);
			attackable ^= 1ull << lsb;
		}
		
		myknights ^= 1ull << bit;
	}
	
	// Generate Bishop & Queen Moves
	while(mybishops != 0){
		bit = get_lsb(mybishops);
		blockers = notempty & OccupancyMaskBishop[bit];
		dbindex = (blockers * MagicNumberBishop[bit]) >> MagicShiftsBishop[bit];
		attackable = MoveDatabaseBishop[bit][dbindex] & notfriendly;
		
		while(attackable != 0){
			lsb = get_lsb(attackable);
			moves[(*size)++] = MOVE_MAKE(bit,lsb,NormalMove);
			attackable ^= 1ull << lsb;
		}
		
		mybishops ^= 1ull << bit;
	}
	
	// Generate Rook & Queen Moves
	while(myrooks != 0){
		bit = get_lsb(myrooks);
		blockers = notempty & OccupancyMaskRook[bit];
		dbindex = (blockers * MagicNumberRook[bit]) >> MagicShiftsRook[bit];
		attackable = MoveDatabaseRook[bit][dbindex] & notfriendly;
		
		while(attackable != 0){
			lsb = get_lsb(attackable);
			moves[(*size)++] = MOVE_MAKE(bit,lsb,NormalMove);
			attackable ^= 1ull << lsb;
		}
		
		myrooks ^= 1ull << bit;
	}
	
	// Generate King Moves
	while(mykings != 0){
		bit = get_lsb(mykings);
		attackable = KingMap[bit] & notfriendly;
		
		while(attackable != 0){
			lsb = get_lsb(attackable);
			moves[(*size)++] = MOVE_MAKE(bit,lsb,NormalMove);
			attackable ^= 1ull << lsb;
		}
		
		mykings ^= 1ull << bit;
	}
	
	// Generate Castles
	if (is_not_in_check(board,board->turn)){
		if (board->turn == ColourWhite){
			
			// King Side Castle
			if ((notempty & WhiteCastleKingSideMap) == 0)
				if (board->castlerights & WhiteKingRights)
					if (board->squares[7] == WhiteRook)
						if (!square_is_attacked(board,ColourWhite,5))
							moves[(*size)++] = MOVE_MAKE(4,6,CastleMove);
						
			// Queen Side Castle
			if ((notempty & WhiteCastleQueenSideMap) == 0)
				if (board->castlerights & WhiteQueenRights)
					if (board->squares[0] == WhiteRook)
						if (!square_is_attacked(board,ColourWhite,3))
							moves[(*size)++] = MOVE_MAKE(4,2,CastleMove);
		}
		
		else {
			
			// King Side Castle
			if ((notempty & BlackCastleKingSideMap) == 0)
				if (board->castlerights & BlackKingRights)
					if (board->squares[63] == BlackRook)
						if (!square_is_attacked(board,ColourBlack,61))
							moves[(*size)++] = MOVE_MAKE(60,62,CastleMove);
						
			// Queen Side Castle
			if ((notempty & BlackCastleQueenSideMap) == 0)
				if (board->castlerights & BlackQueenRights)
					if (board->squares[56] == BlackRook)
						if (!square_is_attacked(board,ColourBlack,59))
							moves[(*size)++] = MOVE_MAKE(60,58,CastleMove);
		}
	}
}

int is_not_in_check(Board * board, int turn){
	int kingsq = get_lsb(board->colourBitBoards[turn] & board->pieceBitBoards[5]);
	assert(board->squares[kingsq] == WhiteKing + turn);
	return !square_is_attacked(board,turn,kingsq);
}

int square_is_attacked(Board * board, int turn, int sq){
	int kingbit, dbindex;
	uint64_t square, blockers;
	
	uint64_t friendly = board->colourBitBoards[turn];
	uint64_t enemy = board->colourBitBoards[!turn];
	uint64_t notempty = friendly | enemy;
	
	uint64_t enemypawns = enemy & board->pieceBitBoards[0];
	uint64_t enemyknights = enemy & board->pieceBitBoards[1];
	uint64_t enemybishops = enemy & board->pieceBitBoards[2];
	uint64_t enemyrooks = enemy & board->pieceBitBoards[3];
	uint64_t enemyqueens = enemy & board->pieceBitBoards[4];
	uint64_t enemykings = enemy & board->pieceBitBoards[5];
	
	enemybishops |= enemyqueens;
	enemyrooks |= enemyqueens;
	square = (1ull << sq);
	
	// Pawns
	if (turn == ColourWhite){
		if ((((square << 7) & ~FILE_A) & enemypawns) != 0) return 1;
		if ((((square << 9) & ~FILE_H) & enemypawns) != 0) return 1;
	} else {
		if ((((square >> 7) & ~FILE_H) & enemypawns) != 0) return 1;
		if ((((square >> 9) & ~FILE_A) & enemypawns) != 0) return 1;
	}
	
	// Knights
	if ((KnightMap[sq] & enemyknights) != 0) return 1;
	
	// Bishops and Queens
	blockers = notempty & OccupancyMaskBishop[sq];
	dbindex = (blockers * MagicNumberBishop[sq]) >> MagicShiftsBishop[sq];
	if ((MoveDatabaseBishop[sq][dbindex] & enemybishops) != 0) return 1;
	
	// Rooks and Queens
	blockers = notempty & OccupancyMaskRook[sq];
	dbindex = (blockers * MagicNumberRook[sq]) >> MagicShiftsRook[sq];
	if ((MoveDatabaseRook[sq][dbindex] & enemyrooks) != 0) return 1;
	
	// King
	if ((KingMap[sq] & enemykings) != 0) return 1;
	
	return 0;
}
