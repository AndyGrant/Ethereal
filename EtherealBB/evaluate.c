#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "movegen.h"
#include "types.h"
#include "bitboards.h"
#include "bitutils.h"
#include "evaluate.h"
#include "piece.h"

int evaluate_board(Board * board){
    
    uint64_t pieces = board->colourBitBoards[0] | board->colourBitBoards[1];
    //int num = count_set_bits(pieces);
    int num = board->num_pieces;
    int value = 0;
    
    uint64_t white    = board->colourBitBoards[ColourWhite];
    uint64_t black    = board->colourBitBoards[ColourBlack];
    uint64_t empty    = ~ (white | black);
    
    uint64_t pawns    = board->pieceBitBoards[0];
    uint64_t knights  = board->pieceBitBoards[1];
    uint64_t bishops  = board->pieceBitBoards[2];
    uint64_t rooks    = board->pieceBitBoards[3];
    uint64_t queens   = board->pieceBitBoards[4];
    
    uint64_t whitePawn = white & pawns;
    uint64_t blackPawn = black & pawns;
    
    // Check for recognized draws. This includes the most basic
    // King versus King endgame, as well as impossible mates with
    // only a knight, or only a bishop, versus a sole king. Also,
    // Situations like K+B+N v K in which mate is possible, but not 
    // forced. Finally, the similar situation of K+2N in which mate
    // cannot be forced. This check does not find sitations where
    // one side has two bishops, but both on the same colour, which
    // would result in a draw, and also dead-lock situations
    if (pawns == 0 && rooks == 0 && queens == 0){
        
        // K v K
        if (num == 2)
            return 0;
        
        // K+B v K and K+N v K
        if (count_set_bits(white & (knights | bishops)) <= 1 && 
            count_set_bits(black) == 1)
            return 0;
        else if (count_set_bits(black & (knights | bishops)) <= 1 && 
            count_set_bits(white) == 1)
            return 0;
         
        // K+B+N v K 
        if (count_set_bits(black) == 1 &&
            count_set_bits(white & bishops) == 1 &&
            count_set_bits(white & knights) == 1)
            return 0;
        else if (count_set_bits(white) == 1 &&
            count_set_bits(black & bishops) == 1 &&
            count_set_bits(black & knights) == 1)
            return 0;
        
        // K+N+N v K
        if (count_set_bits(black) == 1 &&
            count_set_bits(white & knights) == 2 &&
            count_set_bits(white & bishops) == 0)
            return 0;
        else if (count_set_bits(white) == 1 &&
            count_set_bits(black & knights) == 2 &&
            count_set_bits(black & bishops) == 0)
            return 0;
    }
    
    uint64_t whiteRook = white & rooks;
    uint64_t blackRook = black & rooks;
    
    uint64_t wa = whitePawn & FILE_A;
    uint64_t wb = whitePawn & FILE_B;
    uint64_t wc = whitePawn & FILE_C;
    uint64_t wd = whitePawn & FILE_D;
    uint64_t we = whitePawn & FILE_E;
    uint64_t wf = whitePawn & FILE_F;
    uint64_t wg = whitePawn & FILE_G;
    uint64_t wh = whitePawn & FILE_H;
    
    uint64_t ba = blackPawn & FILE_A;
    uint64_t bb = blackPawn & FILE_B;
    uint64_t bc = blackPawn & FILE_C;
    uint64_t bd = blackPawn & FILE_D;
    uint64_t be = blackPawn & FILE_E;
    uint64_t bf = blackPawn & FILE_F;
    uint64_t bg = blackPawn & FILE_G;
    uint64_t bh = blackPawn & FILE_H;
    
    value -= PAWN_STACKED_PENALTY * (
        ((wa & (wa-1)) != 0) + ((wb & (wb-1)) != 0) + 
        ((wc & (wc-1)) != 0) + ((wd & (wd-1)) != 0) +
        ((we & (we-1)) != 0) + ((wf & (wf-1)) != 0) +
        ((wg & (wg-1)) != 0) + ((wh & (wh-1)) != 0) -
        
        ((ba & (ba-1)) != 0) - ((bb & (bb-1)) != 0) - 
        ((bc & (bc-1)) != 0) - ((bd & (bd-1)) != 0) -
        ((be & (be-1)) != 0) - ((bf & (bf-1)) != 0) -
        ((bg & (bg-1)) != 0) - ((bh & (bh-1)) != 0)
    );
    
    value -= PAWN_ISOLATED_PENALTY * (
        (wa && !wb)        + (wb && !wa && !wc) +
        (wc && !wb && !wd) + (wd && !wc && !we) +
        (we && !wd && !wf) + (wf && !we && !wg) +
        (wg && !wf && !wh) + (wh && !wg)        -
        
        (ba && !bb)        - (bb && !ba && !bc) -
        (bc && !bb && !bd) - (bd && !bc && !be) -
        (be && !bd && !bf) - (bf && !be && !bg) -
        (bg && !bf && !bh) - (bh && !bg)
    );
    
    value += PAWN_7TH_RANK_VALUE * (
        count_set_bits(whitePawn & RANK_7) -
        count_set_bits(blackPawn & RANK_2)
    );
    
    value += ROOK_7TH_RANK_VALUE * (
        count_set_bits(whiteRook & RANK_7) - 
        count_set_bits(blackRook & RANK_2) 
    );
    
    value += ROOK_8TH_RANK_VALUE * (
        count_set_bits(whiteRook & RANK_8) - 
        count_set_bits(blackRook & RANK_1) 
    );
    
    
    if (num > 14)
        return board->turn == ColourWhite ? board->opening + value : -board->opening - value;
    else
        return board->turn == ColourWhite ? board->endgame + value : -board->endgame - value;
    
}
