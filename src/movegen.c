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

/**
 * Generate all of the psudeo legal moves for a given board.
 * The moves will be stored in a pointer passed to the function,
 * and an integer pointer passed to the function will be updated
 * to reflect the total number of psuedo legal moves found.
 *
 * @param   board   Board pointer with the current position
 * @param   moves   Destination for the found psuedo legal moves
 * @param   size    Pointer to keep track of the number of moves
 */
void genAllMoves(Board * board, uint16_t * moves, int * size){
    uint64_t attackable;
    
    uint64_t pawnForwardOne;
    uint64_t pawnForwardTwo;
    uint64_t pawnLeft;
    uint64_t pawnRight;
    
    uint64_t pawnPromoForward;
    uint64_t pawnPromoLeft;
    uint64_t pawnPromoRight;
    
    int bit, lsb;
    int forwardShift, leftShift, rightShift;
    int epSquare = board->epSquare;
    
    uint64_t friendly = board->colourBitBoards[board->turn];
    uint64_t enemy = board->colourBitBoards[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    uint64_t notFriendly = ~friendly;
    
    uint64_t myPawns   = friendly & board->pieceBitBoards[0];
    uint64_t myKnights = friendly & board->pieceBitBoards[1];
    uint64_t myBishops = friendly & board->pieceBitBoards[2];
    uint64_t myRooks   = friendly & board->pieceBitBoards[3];
    uint64_t myQueens  = friendly & board->pieceBitBoards[4];
    uint64_t myKings   = friendly & board->pieceBitBoards[5];
    
    // GENERATE QUEEN MOVES AS IF THEY WERE ROOKS AND BISHOPS
    myBishops |= myQueens;
    myRooks |= myQueens;
    
    // DEFINE PAWN BITBOARDS AND FIND ENPASS MOVES
    if (board->turn == ColourWhite){
        forwardShift = 8;
        leftShift = 7;
        rightShift = 9;
        
        pawnForwardOne = (myPawns << 8) & empty;
        pawnForwardTwo = ((pawnForwardOne & RANK_3) << 8) & empty;
        pawnLeft = ((myPawns << 7) & (~FILE_H)) & enemy;
        pawnRight = ((myPawns << 9) & (~FILE_A)) & enemy;
        
        pawnPromoForward = pawnForwardOne & RANK_8;
        pawnPromoLeft = pawnLeft & RANK_8;
        pawnPromoRight = pawnRight & RANK_8;
        
        pawnForwardOne &= ~RANK_8;
        pawnLeft &= ~RANK_8;
        pawnRight &= ~RANK_8;
        
        if(epSquare >= 40){
            if (board->squares[epSquare-7] == WhitePawn && epSquare != 47)
                moves[(*size)++] = MoveMake(epSquare-7,epSquare,EnpassMove);
            
            if (board->squares[epSquare-9] == WhitePawn && epSquare != 40)
                moves[(*size)++] = MoveMake(epSquare-9,epSquare,EnpassMove);
        }
        
    } else {
        forwardShift = -8;
        leftShift = -7;
        rightShift = -9;
        
        pawnForwardOne = (myPawns >> 8) & empty;
        pawnForwardTwo = ((pawnForwardOne & RANK_6) >> 8) & empty;
        pawnLeft = ((myPawns >> 7) & (~FILE_A)) & enemy;
        pawnRight = ((myPawns >> 9) & (~FILE_H)) & enemy;
        
        pawnPromoForward = pawnForwardOne & RANK_1;
        pawnPromoLeft = pawnLeft & RANK_1;
        pawnPromoRight = pawnRight & RANK_1;
        
        pawnForwardOne &= ~RANK_1;
        pawnLeft &= ~RANK_1;
        pawnRight &= ~RANK_1;
        
        if(epSquare > 0 && epSquare < 40){
            if (board->squares[epSquare+7] == BlackPawn && epSquare != 16)
                moves[(*size)++] = MoveMake(epSquare+7,epSquare,EnpassMove);
            
            if (board->squares[epSquare+9] == BlackPawn && epSquare != 23)
                moves[(*size)++] = MoveMake(epSquare+9,epSquare,EnpassMove);
        }
    }
    
    // GENERATE PAWN MOVES
    while(pawnForwardOne != 0){
        lsb = getLSB(pawnForwardOne);
        moves[(*size)++] = MoveMake(lsb-forwardShift,lsb,NormalMove);
        pawnForwardOne ^= 1ull << lsb;
    }
    
    while(pawnForwardTwo != 0){
        lsb = getLSB(pawnForwardTwo);
        moves[(*size)++] = MoveMake(lsb-(2*forwardShift),lsb,NormalMove);
        pawnForwardTwo ^= 1ull << lsb;
    }
    
    while(pawnLeft != 0){
        lsb = getLSB(pawnLeft);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,NormalMove);
        pawnLeft ^= 1ull << lsb;
    }
    
    while(pawnRight != 0){
        lsb = getLSB(pawnRight);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,NormalMove);
        pawnRight ^= 1ull << lsb;
    }
    
    while(pawnPromoForward != 0){
        lsb = getLSB(pawnPromoForward);
        moves[(*size)++] = MoveMake(lsb-forwardShift,lsb,PromotionMove|PromoteToQueen);
        moves[(*size)++] = MoveMake(lsb-forwardShift,lsb,PromotionMove|PromoteToRook);
        moves[(*size)++] = MoveMake(lsb-forwardShift,lsb,PromotionMove|PromoteToBishop);
        moves[(*size)++] = MoveMake(lsb-forwardShift,lsb,PromotionMove|PromoteToKnight);
        pawnPromoForward ^= 1ull << lsb;
    }
    
    while(pawnPromoLeft != 0){
        lsb = getLSB(pawnPromoLeft);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,PromotionMove|PromoteToQueen);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,PromotionMove|PromoteToRook);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,PromotionMove|PromoteToBishop);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,PromotionMove|PromoteToKnight);
        pawnPromoLeft ^= 1ull << lsb;
    }
    
    while(pawnPromoRight != 0){
        lsb = getLSB(pawnPromoRight);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,PromotionMove|PromoteToQueen);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,PromotionMove|PromoteToRook);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,PromotionMove|PromoteToBishop);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,PromotionMove|PromoteToKnight);
        pawnPromoRight ^= 1ull << lsb;
    }
    
    // GENERATE KNIGHT MOVES
    while(myKnights != 0){
        bit = getLSB(myKnights);
        attackable = KnightAttacks(bit, notFriendly);
        
        while(attackable != 0){
            lsb = getLSB(attackable);
            moves[(*size)++] = MoveMake(bit,lsb,NormalMove);
            attackable ^= 1ull << lsb;
        }
        
        myKnights ^= 1ull << bit;
    }
    
    // GENERATE BISHOP MOVES
    while(myBishops != 0){
        bit = getLSB(myBishops);
        attackable = BishopAttacks(bit, notEmpty, notFriendly);
        
        while(attackable != 0){
            lsb = getLSB(attackable);
            moves[(*size)++] = MoveMake(bit,lsb,NormalMove);
            attackable ^= 1ull << lsb;
        }
        
        myBishops ^= 1ull << bit;
    }
    
    // GENERATE ROOK MOVES
    while(myRooks != 0){
        bit = getLSB(myRooks);
        attackable = RookAttacks(bit, notEmpty, notFriendly);
        
        while(attackable != 0){
            lsb = getLSB(attackable);
            moves[(*size)++] = MoveMake(bit,lsb,NormalMove);
            attackable ^= 1ull << lsb;
        }
        
        myRooks ^= 1ull << bit;
    }
    
    // GENERATE KING MOVES
    while(myKings != 0){
        bit = getLSB(myKings);
        attackable = KingAttacks(bit, notFriendly);
        
        while(attackable != 0){
            lsb = getLSB(attackable);
            moves[(*size)++] = MoveMake(bit,lsb,NormalMove);
            attackable ^= 1ull << lsb;
        }
        
        myKings ^= 1ull << bit;
    }
    
    // GENERATE CASTLES
    if (isNotInCheck(board,board->turn)){
        if (board->turn == ColourWhite){
            
            // KING SIDE
            if ((notEmpty & WhiteCastleKingSideMap) == 0)
                if (board->castleRights & WhiteKingRights)
                    if (board->squares[7] == WhiteRook)
                        if (!squareIsAttacked(board,ColourWhite,5))
                            moves[(*size)++] = MoveMake(4,6,CastleMove);
                        
            // QUEEN SIDE
            if ((notEmpty & WhiteCastleQueenSideMap) == 0)
                if (board->castleRights & WhiteQueenRights)
                    if (board->squares[0] == WhiteRook)
                        if (!squareIsAttacked(board,ColourWhite,3))
                            moves[(*size)++] = MoveMake(4,2,CastleMove);
        }
        
        else {
            
            // KING SIDE
            if ((notEmpty & BlackCastleKingSideMap) == 0)
                if (board->castleRights & BlackKingRights)
                    if (board->squares[63] == BlackRook)
                        if (!squareIsAttacked(board,ColourBlack,61))
                            moves[(*size)++] = MoveMake(60,62,CastleMove);
                        
            // QUEEN SIDE
            if ((notEmpty & BlackCastleQueenSideMap) == 0)
                if (board->castleRights & BlackQueenRights)
                    if (board->squares[56] == BlackRook)
                        if (!squareIsAttacked(board,ColourBlack,59))
                            moves[(*size)++] = MoveMake(60,58,CastleMove);
        }
    }
}


/**
 * Generate all of the psudeo legal moves which are considered
 * non quiet, IE captures, promotions, enpassant, for a given board
 * The moves will be stored in a pointer passed to the function,
 * and an integer pointer passed to the function will be updated
 * to reflect the total number of psuedo legal moves found.
 *
 * @param   board   Board pointer with the current position
 * @param   moves   Destination for the found psuedo legal moves
 * @param   size    Pointer to keep track of the number of moves
 */
void genAllNonQuiet(Board * board, uint16_t * moves, int * size){
    uint64_t attackable;
    
    uint64_t pawnForwardOne;
    uint64_t pawnLeft;
    uint64_t pawnRight;
    
    uint64_t pawnPromoForward;
    uint64_t pawnPromoLeft;
    uint64_t pawnPromoRight;
    
    int bit, lsb;
    int forwardShift, leftShift, rightShift;
    int epSquare = board->epSquare;
    
    uint64_t friendly = board->colourBitBoards[board->turn];
    uint64_t enemy = board->colourBitBoards[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    
    uint64_t myPawns   = friendly & board->pieceBitBoards[0];
    uint64_t myKnights = friendly & board->pieceBitBoards[1];
    uint64_t myBishops = friendly & board->pieceBitBoards[2];
    uint64_t myRooks   = friendly & board->pieceBitBoards[3];
    uint64_t myQueens  = friendly & board->pieceBitBoards[4];
    uint64_t myKings   = friendly & board->pieceBitBoards[5];
    
    // Generate queen moves as if they were rooks and bishops
    myBishops |= myQueens;
    myRooks |= myQueens;
    
    // Generate Pawn BitBoards and Generate Enpass Moves
    if (board->turn == ColourWhite){
        forwardShift = 8;
        leftShift = 7;
        rightShift = 9;
        
        pawnForwardOne = (myPawns << 8) & empty;
        pawnLeft = ((myPawns << 7) & ~FILE_H) & enemy;
        pawnRight = ((myPawns << 9) & ~FILE_A) & enemy;
        
        pawnPromoForward = pawnForwardOne & RANK_8;
        pawnPromoLeft = pawnLeft & RANK_8;
        pawnPromoRight = pawnRight & RANK_8;
        
        pawnLeft &= ~RANK_8;
        pawnRight &= ~RANK_8;
        
        if(epSquare >= 40){
            if (board->squares[epSquare-7] == WhitePawn && epSquare != 47)
                moves[(*size)++] = MoveMake(epSquare-7,epSquare,EnpassMove);
            
            if (board->squares[epSquare-9] == WhitePawn && epSquare != 40)
                moves[(*size)++] = MoveMake(epSquare-9,epSquare,EnpassMove);
        }
        
    } else {
        forwardShift = -8;
        leftShift = -7;
        rightShift = -9;
        
        pawnForwardOne = (myPawns >> 8) & empty;
        pawnLeft = ((myPawns >> 7) & ~FILE_A) & enemy;
        pawnRight = ((myPawns >> 9) & ~FILE_H) & enemy;
        
        pawnPromoForward = pawnForwardOne & RANK_1;
        pawnPromoLeft = pawnLeft & RANK_1;
        pawnPromoRight = pawnRight & RANK_1;
        
        pawnLeft &= ~RANK_1;
        pawnRight &= ~RANK_1;
        
        if(epSquare > 0 && epSquare < 40){
            if (board->squares[epSquare+7] == BlackPawn && epSquare != 16)
                moves[(*size)++] = MoveMake(epSquare+7,epSquare,EnpassMove);
            
            if (board->squares[epSquare+9] == BlackPawn && epSquare != 23)
                moves[(*size)++] = MoveMake(epSquare+9,epSquare,EnpassMove);
        }
    }
    
    // Generate Pawn Moves
    while(pawnLeft != 0){
        lsb = getLSB(pawnLeft);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,NormalMove);
        pawnLeft ^= 1ull << lsb;
    }
    
    while(pawnRight != 0){
        lsb = getLSB(pawnRight);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,NormalMove);
        pawnRight ^= 1ull << lsb;
    }
    
    while(pawnPromoForward != 0){
        lsb = getLSB(pawnPromoForward);
        moves[(*size)++] = MoveMake(lsb-forwardShift,lsb,PromotionMove|PromoteToQueen);
        moves[(*size)++] = MoveMake(lsb-forwardShift,lsb,PromotionMove|PromoteToRook);
        moves[(*size)++] = MoveMake(lsb-forwardShift,lsb,PromotionMove|PromoteToBishop);
        moves[(*size)++] = MoveMake(lsb-forwardShift,lsb,PromotionMove|PromoteToKnight);
        pawnPromoForward ^= 1ull << lsb;
    }
    
    while(pawnPromoLeft != 0){
        lsb = getLSB(pawnPromoLeft);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,PromotionMove|PromoteToQueen);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,PromotionMove|PromoteToRook);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,PromotionMove|PromoteToBishop);
        moves[(*size)++] = MoveMake(lsb-leftShift,lsb,PromotionMove|PromoteToKnight);
        pawnPromoLeft ^= 1ull << lsb;
    }
    
    while(pawnPromoRight != 0){
        lsb = getLSB(pawnPromoRight);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,PromotionMove|PromoteToQueen);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,PromotionMove|PromoteToRook);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,PromotionMove|PromoteToBishop);
        moves[(*size)++] = MoveMake(lsb-rightShift,lsb,PromotionMove|PromoteToKnight);
        pawnPromoRight ^= 1ull << lsb;
    }
    
    // Generate Knight Moves
    while(myKnights != 0){
        bit = getLSB(myKnights);
        attackable = KnightAttacks(bit, enemy);
        
        while(attackable != 0){
            lsb = getLSB(attackable);
            moves[(*size)++] = MoveMake(bit,lsb,NormalMove);
            attackable ^= 1ull << lsb;
        }
        
        myKnights ^= 1ull << bit;
    }
    
    // Generate Bishop & Queen Moves
    while(myBishops != 0){
        bit = getLSB(myBishops);
        attackable = BishopAttacks(bit, notEmpty, enemy);
        
        while(attackable != 0){
            lsb = getLSB(attackable);
            moves[(*size)++] = MoveMake(bit,lsb,NormalMove);
            attackable ^= 1ull << lsb;
        }
        
        myBishops ^= 1ull << bit;
    }
    
    // Generate Rook & Queen Moves
    while(myRooks != 0){
        bit = getLSB(myRooks);
        attackable = RookAttacks(bit, notEmpty, enemy);
        
        while(attackable != 0){
            lsb = getLSB(attackable);
            moves[(*size)++] = MoveMake(bit,lsb,NormalMove);
            attackable ^= 1ull << lsb;
        }
        
        myRooks ^= 1ull << bit;
    }
    
    // Generate King Moves
    while(myKings != 0){
        bit = getLSB(myKings);
        attackable = KingAttacks(bit, enemy);
        
        while(attackable != 0){
            lsb = getLSB(attackable);
            moves[(*size)++] = MoveMake(bit,lsb,NormalMove);
            attackable ^= 1ull << lsb;
        }
        
        myKings ^= 1ull << bit;
    }
}

/**
 * Determine if the king of a certain colour is in check
 * on the board passed to the function
 *
 * @param   board   Board pointer to current position
 * @param   turn    Colour of the king to determine check status for
 *
 * @return          1 for not in check, 0 for in check
 */
int isNotInCheck(Board * board, int turn){
    int kingsq = getLSB(board->colourBitBoards[turn] & board->pieceBitBoards[5]);
    assert(board->squares[kingsq] == WhiteKing + turn);
    return !squareIsAttacked(board, turn, kingsq);
}

/**
 * Determine if the enemy pieces are able to attacked
 * a given square. This is used by isNotInCheck.
 *
 * @param   board   Board pointer for current position
 * @param   turn    Colour of friendly side
 * @param   sq      Square to be attacked
 *
 * @return          1 if can be attacked, 0 if cannot
 */
int squareIsAttacked(Board * board, int turn, int sq){
    
    uint64_t square;
    
    uint64_t friendly = board->colourBitBoards[turn];
    uint64_t enemy = board->colourBitBoards[!turn];
    uint64_t notEmpty = friendly | enemy;
    
    uint64_t enemyPawns   = enemy & board->pieceBitBoards[0];
    uint64_t enemyKnights = enemy & board->pieceBitBoards[1];
    uint64_t enemyBishops = enemy & board->pieceBitBoards[2];
    uint64_t enemyRooks   = enemy & board->pieceBitBoards[3];
    uint64_t enemyQueens  = enemy & board->pieceBitBoards[4];
    uint64_t enemyKings   = enemy & board->pieceBitBoards[5];
    
    enemyBishops |= enemyQueens;
    enemyRooks |= enemyQueens;
    square = (1ull << sq);
    
    // Pawns
    if (turn == ColourWhite){
        if (((square << 7) & ~FILE_H) & enemyPawns) return 1;
        if (((square << 9) & ~FILE_A) & enemyPawns) return 1;
    } else {
        if (((square >> 7) & ~FILE_A) & enemyPawns) return 1;
        if (((square >> 9) & ~FILE_H) & enemyPawns) return 1;
    }
    
    // Knights
    if (enemyKnights && KnightAttacks(sq, enemyKnights)) return 1;
    
    // Bishops and Queens
    if (enemyBishops && BishopAttacks(sq, notEmpty, enemyBishops)) return 1;
    
    // Rooks and Queens
    if (enemyRooks && RookAttacks(sq, notEmpty, enemyRooks)) return 1;
    
    // King
    if (KingAttacks(sq, enemyKings)) return 1;
    
    return 0;
}