#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "movegen.h"
#include "magics.h"
#include "types.h"
#include "bitboards.h"
#include "bitutils.h"
#include "evaluate.h"
#include "piece.h"

int evaluate_board(Board * board){
    
    int i, num, mid = 0, end = 0;
    int curPhase, mid_eval, end_eval, eval;
    
    uint64_t white    = board->colourBitBoards[ColourWhite];
    uint64_t black    = board->colourBitBoards[ColourBlack];
    
    uint64_t pawns    = board->pieceBitBoards[0];
    uint64_t knights  = board->pieceBitBoards[1];
    uint64_t bishops  = board->pieceBitBoards[2];
    uint64_t rooks    = board->pieceBitBoards[3];
    uint64_t queens   = board->pieceBitBoards[4];
    
    // Check for Recognized Draws
    if (pawns == 0 && rooks == 0 && queens == 0){
        
        // K v K
        if (board->num_pieces == 2)
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
    
    uint64_t whitePawns = white & pawns;
    uint64_t blackPawns = black & pawns;
    
    uint64_t whiteBishops = white & bishops;
    uint64_t blackBishops = black & bishops;
    
    uint64_t whiteRooks = white & rooks;
    uint64_t blackRooks = black & rooks;
    
    uint64_t wa = whitePawns & FILE_A, wb = whitePawns & FILE_B;
    uint64_t wc = whitePawns & FILE_C, wd = whitePawns & FILE_D;
    uint64_t we = whitePawns & FILE_E, wf = whitePawns & FILE_F;
    uint64_t wg = whitePawns & FILE_G, wh = whitePawns & FILE_H;
    
    uint64_t ba = blackPawns & FILE_A, bb = blackPawns & FILE_B;
    uint64_t bc = blackPawns & FILE_C, bd = blackPawns & FILE_D;
    uint64_t be = blackPawns & FILE_E, bf = blackPawns & FILE_F;
    uint64_t bg = blackPawns & FILE_G, bh = blackPawns & FILE_H;
    
    
    
    // STACKED PAWNS
    int net_stacked = (
        ((wa & (wa-1)) != 0) +
        ((wb & (wb-1)) != 0) +
        ((wc & (wc-1)) != 0) +
        ((wd & (wd-1)) != 0) +
        ((we & (we-1)) != 0) +
        ((wf & (wf-1)) != 0) +
        ((wg & (wg-1)) != 0) +
        ((wh & (wh-1)) != 0) -
    
        ((ba & (ba-1)) != 0) -
        ((bb & (bb-1)) != 0) -
        ((bc & (bc-1)) != 0) -
        ((bd & (bd-1)) != 0) -
        ((be & (be-1)) != 0) -
        ((bf & (bf-1)) != 0) -
        ((bg & (bg-1)) != 0) -
        ((bh & (bh-1)) != 0)
    );
    
    mid -= net_stacked * PAWN_STACKED_MID;
    end -= net_stacked * PAWN_STACKED_END;
    
    
    // ISOLATED PAWNS
    int net_isolated = (
        (wa && !wb       ) +
        (wb && !wa && !wc) +
        (wc && !wb && !wd) +
        (wd && !wc && !we) +
        (we && !wd && !wf) +
        (wf && !we && !wg) +
        (wg && !wf && !wh) +
        (wh && !wg       ) -
        
        (ba && !bb       ) -
        (bb && !ba && !bc) -
        (bc && !bb && !bd) -
        (bd && !bc && !be) -
        (be && !bd && !bf) -
        (bf && !be && !bg) -
        (bg && !bf && !bh) -
        (bh && !bg       )
    );
    
    mid -= net_isolated * PAWN_ISOLATED_MID;
    end -= net_isolated * PAWN_ISOLATED_END;
    
    int net_7th = (
        count_set_bits(whitePawns & RANK_7) -
        count_set_bits(blackPawns & RANK_2)
    );
    
    mid += net_7th * PAWN_7TH_RANK_MID;
    end += net_7th * PAWN_7TH_RANK_END;
    
    /* PASSED PAWNS */
    /* BACKWARDS PAWNS */    
    
    
    int wrooks[16], brooks[16];
    get_set_bits(whiteRooks, wrooks);
    get_set_bits(blackRooks, brooks);
    
    for (i = 0; wrooks[i] != -1; i++){
        
        // Rooks Open File and Semi Open file
        if (whitePawns & FILES[wrooks[i] % 8] == 0){
            if (blackPawns & FILES[wrooks[i] % 8] == 0){
                mid += ROOK_OPEN_FILE_MID;
                end += ROOK_OPEN_FILE_END;
            }
            else{
                mid += ROOK_SEMI_FILE_MID;
                end += ROOK_SEMI_FILE_END;
            }
        }
        
        // Rook on 7th
        if ((wrooks[i] >> 3) == 6){
            mid += ROOK_ON_7TH_MID;
            end += ROOK_ON_7TH_END;
        }
    }
    
    for (i = 0; brooks[i] != -1; i++){
        
        // Rooks Open File and Semi Open file
        if (blackPawns & FILES[brooks[i] % 8] == 0){
            if (whitePawns & FILES[brooks[i] % 8] == 0){
                mid -= ROOK_OPEN_FILE_MID;
                end -= ROOK_OPEN_FILE_END;
            }
            else{
                mid -= ROOK_SEMI_FILE_MID;
                end -= ROOK_SEMI_FILE_END;
            }
        }
        
        // Rook on 7th
        if ((brooks[i] >> 3) == 1){
            mid -= ROOK_ON_7TH_MID;
            end -= ROOK_ON_7TH_END;
        }
    }
    
    
    int wbishops[16], bbishops[16];
    get_set_bits(whiteBishops, wbishops);
    get_set_bits(blackBishops, bbishops);
    
    // Bishop Pair
    if (wbishops[0] != -1 && wbishops[1] != -1){
        mid += BISHOP_PAIR_MID;
        end += BISHOP_PAIR_END;
    }
    if (bbishops[0] != -1 && bbishops[1] != -1){
        mid -= BISHOP_PAIR_MID;
        end -= BISHOP_PAIR_END;
    }
    
    // Bishop has Pawn Wings
    if (whiteBishops != 0 && 
       (whitePawns & (FILE_A|FILE_B)) != 0 &&
       (whitePawns & (FILE_G|FILE_H)) != 0){
       
       mid += BISHOP_HAS_WINGS_MID;
       end += BISHOP_HAS_WINGS_END;
    }
       
    if (blackBishops != 0 && 
       (blackPawns & (FILE_A|FILE_B)) != 0 &&
       (blackPawns & (FILE_G|FILE_H)) != 0){
           
       mid -= BISHOP_HAS_WINGS_MID;
       end -= BISHOP_HAS_WINGS_END;
    }
    
    curPhase = 24 - (1 * count_set_bits(knights | bishops))
                  - (2 * count_set_bits(rooks))
                  - (4 * count_set_bits(queens));
    curPhase = (curPhase * 256 + 12) / 24;
    
    mid_eval = board->opening + mid;
    end_eval = board->endgame + end;
    
    eval = ((mid_eval * (256 - curPhase)) + (end_eval * curPhase)) / 256;
    
    return board->turn == ColourWhite ? eval+20 : -(eval+10);    
}