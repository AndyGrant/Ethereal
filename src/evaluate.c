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
    int curPhase, midEval, endEval, eval;
    
    // EXTRACT BASIC BITBOARDS FROM BOARD
    uint64_t white    = board->colourBitBoards[ColourWhite];
    uint64_t black    = board->colourBitBoards[ColourBlack];
    uint64_t pawns    = board->pieceBitBoards[0];
    uint64_t knights  = board->pieceBitBoards[1];
    uint64_t bishops  = board->pieceBitBoards[2];
    uint64_t rooks    = board->pieceBitBoards[3];
    uint64_t queens   = board->pieceBitBoards[4];
    
    // CREATE BITBOARD FOR PIECES OF EACH COLOUR
    uint64_t whitePawns   = white & pawns;
    uint64_t blackPawns   = black & pawns;
    uint64_t whiteBishops = white & bishops;
    uint64_t blackBishops = black & bishops;
    uint64_t whiteRooks   = white & rooks;
    uint64_t blackRooks   = black & rooks;

    // CREATE BITBOARD FOR EACH FILE, OF EACH COLOUR, FOR PAWNS
    uint64_t wa = whitePawns & FILE_A, wb = whitePawns & FILE_B;
    uint64_t wc = whitePawns & FILE_C, wd = whitePawns & FILE_D;
    uint64_t we = whitePawns & FILE_E, wf = whitePawns & FILE_F;
    uint64_t wg = whitePawns & FILE_G, wh = whitePawns & FILE_H;
    uint64_t ba = blackPawns & FILE_A, bb = blackPawns & FILE_B;
    uint64_t bc = blackPawns & FILE_C, bd = blackPawns & FILE_D;
    uint64_t be = blackPawns & FILE_E, bf = blackPawns & FILE_F;
    uint64_t bg = blackPawns & FILE_G, bh = blackPawns & FILE_H;
    
    // GET LSB FOR EACH FILE OF PAWNS
    int lwa = getLSB(wa), lwb = getLSB(wb);
    int lwc = getLSB(wc), lwd = getLSB(wd);
    int lwe = getLSB(we), lwf = getLSB(wf);
    int lwg = getLSB(wg), lwh = getLSB(wh);
    
    int lba = getLSB(ba), lbb = getLSB(bb);
    int lbc = getLSB(bc), lbd = getLSB(bd);
    int lbe = getLSB(be), lbf = getLSB(bf);
    int lbg = getLSB(bg), lbh = getLSB(bh);
    
    // FOR PAWN EVALUATIONS
    int pawnIsStacked[2][8];
    int pawnIsIsolated[2][8];
    int pawnIsPassed[2][8];
    
    // GET INDEXES OF EACH COLOUR'S ROOKS
    int wRooks[16], bRooks[16];
    getSetBits(whiteRooks, wRooks);
    getSetBits(blackRooks, bRooks);
    
    // GET INDEXES OF EACH COLOUR'S BISHOPS
    int wBishops[16], bBishops[16];
    getSetBits(whiteBishops, wBishops);
    getSetBits(blackBishops, bBishops);
    
    // CHECK FOR RECOGNIZED DRAWS
    if (pawns == 0 && rooks == 0 && queens == 0){
        
        // K v K
        if (board->numPieces == 2)
            return 0;
        
        // K+B v K and K+N v K
        if (countSetBits(white & (knights | bishops)) <= 1 && 
            countSetBits(black) == 1)
            return 0;
        else if (countSetBits(black & (knights | bishops)) <= 1 && 
            countSetBits(white) == 1)
            return 0;
         
        // K+B+N v K 
        if (countSetBits(black) == 1 &&
            countSetBits(white & bishops) == 1 &&
            countSetBits(white & knights) == 1)
            return 0;
        else if (countSetBits(white) == 1 &&
            countSetBits(black & bishops) == 1 &&
            countSetBits(black & knights) == 1)
            return 0;
        
        // K+N+N v K
        if (countSetBits(black) == 1 &&
            countSetBits(white & knights) == 2 &&
            countSetBits(white & bishops) == 0)
            return 0;
        else if (countSetBits(white) == 1 &&
            countSetBits(black & knights) == 2 &&
            countSetBits(black & bishops) == 0)
            return 0;
    }    
    
    // DETERMINE IF STACKED PAWN ON GIVEN FILE
    pawnIsStacked[0][0] = ((wa & (wa-1)) != 0);
    pawnIsStacked[0][1] = ((wb & (wb-1)) != 0);
    pawnIsStacked[0][2] = ((wc & (wc-1)) != 0);
    pawnIsStacked[0][3] = ((wd & (wd-1)) != 0);
    pawnIsStacked[0][4] = ((we & (we-1)) != 0);
    pawnIsStacked[0][5] = ((wf & (wf-1)) != 0);
    pawnIsStacked[0][6] = ((wg & (wg-1)) != 0);
    pawnIsStacked[0][7] = ((wh & (wh-1)) != 0);
    pawnIsStacked[1][0] = ((ba & (ba-1)) != 0);
    pawnIsStacked[1][1] = ((bb & (bb-1)) != 0);
    pawnIsStacked[1][2] = ((bc & (bc-1)) != 0);
    pawnIsStacked[1][3] = ((bd & (bd-1)) != 0);
    pawnIsStacked[1][4] = ((be & (be-1)) != 0);
    pawnIsStacked[1][5] = ((bf & (bf-1)) != 0);
    pawnIsStacked[1][6] = ((bg & (bg-1)) != 0);
    pawnIsStacked[1][7] = ((bh & (bh-1)) != 0);
    
    // DETERMINE IF ISOLATED PAWN ON GIVEN FILE
    pawnIsIsolated[0][0] = (wa && !wb       );
    pawnIsIsolated[0][1] = (wb && !wa && !wc);
    pawnIsIsolated[0][2] = (wc && !wb && !wd);
    pawnIsIsolated[0][3] = (wd && !wc && !we);
    pawnIsIsolated[0][4] = (we && !wd && !wf);
    pawnIsIsolated[0][5] = (wf && !we && !wg);
    pawnIsIsolated[0][6] = (wg && !wf && !wh);
    pawnIsIsolated[0][7] = (wh && !wg       );
    pawnIsIsolated[1][0] = (ba && !bb       );
    pawnIsIsolated[1][1] = (bb && !ba && !bc);
    pawnIsIsolated[1][2] = (bc && !bb && !bd);
    pawnIsIsolated[1][3] = (bd && !bc && !be);
    pawnIsIsolated[1][4] = (be && !bd && !bf);
    pawnIsIsolated[1][5] = (bf && !be && !bg);
    pawnIsIsolated[1][6] = (bg && !bf && !bh);
    pawnIsIsolated[1][7] = (bh && !bg       );
    
    // DETERMINE IF PASSED PAWN ON GIVEN FILE
    pawnIsPassed[0][0] = (wa > ba) && (wa << 1 >= bb)                   ;
    pawnIsPassed[0][1] = (wb > bb) && (wb << 1 >= bc) && (wb >> 1 >= ba);
    pawnIsPassed[0][2] = (wc > bc) && (wc << 1 >= bd) && (wc >> 1 >= bb);
    pawnIsPassed[0][3] = (wd > bd) && (wd << 1 >= be) && (wd >> 1 >= bc);
    pawnIsPassed[0][4] = (we > be) && (we << 1 >= bf) && (we >> 1 >= bd);
    pawnIsPassed[0][5] = (wf > bf) && (wf << 1 >= bg) && (wf >> 1 >= be);
    pawnIsPassed[0][6] = (wg > bg) && (wg << 1 >= bh) && (wg >> 1 >= bf);
    pawnIsPassed[0][7] = (wh > bh)                    && (wh >> 1 >= bg);
    pawnIsPassed[1][0] = (lba < lwa) && (lba + 1 <= lwb)                    ;
    pawnIsPassed[1][1] = (lbb < lwb) && (lbb + 1 <= lwc) && (lbb - 1 <= lwa);
    pawnIsPassed[1][2] = (lbc < lwc) && (lbc + 1 <= lwd) && (lbc - 1 <= lwb);
    pawnIsPassed[1][3] = (lbd < lwd) && (lbd + 1 <= lwe) && (lbd - 1 <= lwc);
    pawnIsPassed[1][4] = (lbe < lwe) && (lbe + 1 <= lwf) && (lbe - 1 <= lwd);
    pawnIsPassed[1][5] = (lbf < lwf) && (lbf + 1 <= lwg) && (lbf - 1 <= lwe);
    pawnIsPassed[1][6] = (lbg < lwg) && (lbg + 1 <= lwh) && (lbg - 1 <= lwf);
    pawnIsPassed[1][7] = (lbh < lwh)                     && (lbh - 1 <= lwg);
    
    // APPLY BONUSES AND PENALTIES FOR WHITE'S PAWN STRUCTURE
    for (i = 0; i < 8; i++){
        if (pawnIsStacked[0][i]){
            mid -= PawnStackedMid[i];
            end -= PawnStackedEnd[i];
        }
        
        if (pawnIsIsolated[0][i]){
            mid -= PawnIsolatedMid[i];
            end -= PawnIsolatedEnd[i];
        }
        
        if (pawnIsPassed[0][i]){
            int msb = getMSBSpecial(whitePawns & FILES[i]);
            mid += PawnPassedMid[msb>>3];
            end += PawnPassedEnd[msb>>3];
        }
    }
    
    
    // APPLY BONUSES AND PENALTIES FOR BLACK'S PAWN STRUCTURE
    for (i = 0; i < 8; i++){
        if (pawnIsStacked[1][i]){
            mid += PawnStackedMid[i];
            end += PawnStackedEnd[i];
        }
        
        if (pawnIsIsolated[1][i]){
            mid += PawnIsolatedMid[i];
            end += PawnIsolatedEnd[i];
        }
        
        if (pawnIsPassed[1][i]){
            int lsb = getLSB(blackPawns & FILES[i]);
            mid -= PawnPassedMid[7-(lsb>>3)];
            end -= PawnPassedEnd[7-(lsb>>3)];
        }
    }
    
    // EVALUATE WHITE ROOKS FOR FILE AND RANK BONUSES
    for (i = 0; wRooks[i] != -1; i++){
        if ((whitePawns & FILES[wRooks[i] % 8]) == 0ull){
            if ((blackPawns & FILES[wRooks[i] % 8]) == 0ull){
                mid += ROOK_OPEN_FILE_MID;
                end += ROOK_OPEN_FILE_END;
            }
            else{
                mid += ROOK_SEMI_FILE_MID;
                end += ROOK_SEMI_FILE_END;
            }
        }
        
        if ((wRooks[i] >> 3) == 6){
            mid += ROOK_ON_7TH_MID;
            end += ROOK_ON_7TH_END;
        }
    }
    
    // EVALUATE STACKED WHITE ROOKS
    if (i == 2){
        if (wRooks[0] % 8 == wRooks[1] % 8
            && ((wRooks[0] >> 3) == 6 || (wRooks[1] >> 3) == 6)){
            mid += ROOK_STACKED_MID;
            end += ROOK_STACKED_END;
        }
    }
    
    // EVALUATE BLACK ROOKS FOR FILE AND RANK BONUSES
    for (i = 0; bRooks[i] != -1; i++){
        if ((blackPawns & FILES[bRooks[i] % 8]) == 0ull){
            if ((whitePawns & FILES[bRooks[i] % 8]) == 0ull){
                mid -= ROOK_OPEN_FILE_MID;
                end -= ROOK_OPEN_FILE_END;
            }
            else{
                mid -= ROOK_SEMI_FILE_MID;
                end -= ROOK_SEMI_FILE_END;
            }
        }
        
        if ((bRooks[i] >> 3) == 1){
            mid -= ROOK_ON_7TH_MID;
            end -= ROOK_ON_7TH_END;
        }
    }
    
    // EVALUATE STACKED BLACK ROOKS
    if (i == 2){
        if (bRooks[0] % 8 == bRooks[1] % 8
            && ((bRooks[0] >> 3) == 1 || (bRooks[1] >> 3) == 1)){
            mid -= ROOK_STACKED_MID;
            end -= ROOK_STACKED_END;
        }
    }
    
    // WHITE HAS A BISHOP PAIR
    if (wBishops[0] != -1 && wBishops[1] != -1){
        mid += BISHOP_PAIR_MID;
        end += BISHOP_PAIR_END;
    }
    
    // BLACK HAS A BISHOP PAIR
    if (bBishops[0] != -1 && bBishops[1] != -1){
        mid -= BISHOP_PAIR_MID;
        end -= BISHOP_PAIR_END;
    }
    
    // WHITE HAS A SOLE BISHOP WITH WINGED PAWNS
    if (whiteBishops != 0 && 
       (whitePawns & (FILE_A|FILE_B)) != 0 &&
       (whitePawns & (FILE_G|FILE_H)) != 0){
           
       mid += BISHOP_HAS_WINGS_MID;
       end += BISHOP_HAS_WINGS_END;
    }
    
    // BLACK HAS A SOLE BISHOP WITH WINGED PAWNS
    if (blackBishops != 0 && 
       (blackPawns & (FILE_A|FILE_B)) != 0 &&
       (blackPawns & (FILE_G|FILE_H)) != 0){
           
       mid -= BISHOP_HAS_WINGS_MID;
       end -= BISHOP_HAS_WINGS_END;
    }
    
    curPhase = 24 - (1 * countSetBits(knights | bishops))
                  - (2 * countSetBits(rooks))
                  - (4 * countSetBits(queens));
    curPhase = (curPhase * 256 + 12) / 24;
    
    midEval = board->opening + mid + TEMPO_MID;
    endEval = board->endgame + end + TEMPO_END;
    
    eval = ((midEval * (256 - curPhase)) + (endEval * curPhase)) / 256;
    
    return board->turn == ColourWhite ? eval : -eval;
}