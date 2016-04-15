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
    if ((wa & (wa-1)) != 0){ mid -= 15; end -= 14;}
    if ((wb & (wb-1)) != 0){ mid -= 19; end -= 31;} 
    if ((wc & (wc-1)) != 0){ mid -= 21; end -= 31;}
    if ((wd & (wd-1)) != 0){ mid -= 21; end -= 31;}
    if ((we & (we-1)) != 0){ mid -= 21; end -= 31;} 
    if ((wf & (wf-1)) != 0){ mid -= 21; end -= 31;}
    if ((wg & (wg-1)) != 0){ mid -= 19; end -= 31;} 
    if ((wh & (wh-1)) != 0){ mid -= 15; end -= 14;}
    
    if ((ba & (ba-1)) != 0){ mid += 15; end += 14;}
    if ((bb & (bb-1)) != 0){ mid += 19; end += 31;} 
    if ((bc & (bc-1)) != 0){ mid += 21; end += 31;}
    if ((bd & (bd-1)) != 0){ mid += 21; end += 31;}
    if ((be & (be-1)) != 0){ mid += 21; end += 31;}
    if ((bf & (bf-1)) != 0){ mid += 21; end += 31;}
    if ((bg & (bg-1)) != 0){ mid += 19; end += 31;}
    if ((bh & (bh-1)) != 0){ mid += 15; end += 14;}
    
    
    // ISOLATED PAWNS
    if (wa && !wb       ){ mid -= 14; end -= 10;}       
    if (wb && !wa && !wc){ mid -= 21; end -= 13;}
    if (wc && !wb && !wd){ mid -= 27; end -= 16;}
    if (wd && !wc && !we){ mid -= 27; end -= 16;}
    if (we && !wd && !wf){ mid -= 27; end -= 16;}
    if (wf && !we && !wg){ mid -= 27; end -= 16;}
    if (wg && !wf && !wh){ mid -= 21; end -= 13;}
    if (wh && !wg       ){ mid -= 14; end -= 10;}       
    
    if (ba && !bb       ){ mid += 14; end += 10;}       
    if (bb && !ba && !bc){ mid += 21; end += 13;}
    if (bc && !bb && !bd){ mid += 27; end += 16;}
    if (bd && !bc && !be){ mid += 27; end += 16;}
    if (be && !bd && !bf){ mid += 27; end += 16;}
    if (bf && !be && !bg){ mid += 27; end += 16;}
    if (bg && !bf && !bh){ mid += 21; end += 13;}
    if (bh && !bg       ){ mid += 14; end += 10;}
    ;
    
    /* PASSED PAWNS */
    /* BACKWARDS PAWNS */    
    
    
    int wrooks[16], brooks[16];
    get_set_bits(whiteRooks, wrooks);
    get_set_bits(blackRooks, brooks);
    
    for (i = 0; wrooks[i] != -1; i++){
        
        // Rooks Open File and Semi Open file
        if (whitePawns & FILES[wrooks[i] % 8] == 0){
            if (blackPawns & FILES[wrooks[i] % 8] == 0){
                mid += 35;
                end += 20;
            }
            else{
                mid += 10;
                end += 10;
            }
        }
        
        // Rook on 7th
        if ((wrooks[i] >> 3) == 6){
            mid += 37;
            end += 25;
        }
    }
    
    for (i = 0; brooks[i] != -1; i++){
        
        // Rooks Open File and Semi Open file
        if (blackPawns & FILES[brooks[i] % 8] == 0){
            if (whitePawns & FILES[brooks[i] % 8] == 0){
                mid -= 35;
                end -= 20;
            }
            else{
                mid -= 10;
                end -= 10;
            }
        }
        
        // Rook on 7th
        if ((brooks[i] >> 3) == 1){
            mid -= 37;
            end -= 25;
        }
    }
    
    
    int wbishops[16], bbishops[16];
    get_set_bits(whiteBishops, wbishops);
    get_set_bits(blackBishops, bbishops);
    
    // Bishop Pair
    if (wbishops[0] != -1 && wbishops[1] != -1){
        mid += 18;
        end += 55;
    }
    if (bbishops[0] != -1 && bbishops[1] != -1){
        mid -= 18;
        end -= 55;
    }
    
    // Bishop has Pawn Wings
    if (whiteBishops != 0 && 
       (whitePawns & (FILE_A|FILE_B|FILE_C)) != 0 &&
       (whitePawns & (FILE_F|FILE_G|FILE_H)) != 0){
       
       mid += 11;
       end += 32;
    }
       
    if (blackBishops != 0 && 
       (blackPawns & (FILE_A|FILE_B|FILE_C)) != 0 &&
       (blackPawns & (FILE_F|FILE_G|FILE_H)) != 0){
           
       mid -= 11;
       end -= 32;
    }
       
       
    // Lone bishop with pawn blocking
    if (wbishops[0] != -1 && wbishops[1] == -1){
        if (whiteBishops & WHITE_SQUARES)
            num = count_set_bits(whitePawns & WHITE_SQUARES);
        else
            num = count_set_bits(whitePawns & BLACK_SQUARES);
        
        mid -= 4 * num;
        end -= 3 * num;
    }
    
    if (bbishops[0] != -1 && bbishops[1] == -1){
        if (blackBishops & WHITE_SQUARES)
            num = count_set_bits(blackPawns & WHITE_SQUARES);
        else
            num = count_set_bits(blackPawns & BLACK_SQUARES);
        
        mid += 4 * num;
        end += 3 * num;
    }
    
    curPhase = 24 - (1 * count_set_bits(knights | bishops))
                  - (2 * count_set_bits(rooks))
                  - (4 * count_set_bits(queens));
    curPhase = (curPhase * 256 + 12) / 24;
    
    mid_eval = board->opening + mid;
    end_eval = board->endgame + end;
    
    eval = ((mid_eval * (256 - curPhase)) + (end_eval * curPhase)) / 256;
    
    return board->turn == ColourWhite ? eval : -eval;    
}