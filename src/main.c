#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "castle.h"
#include "magics.h"
#include "piece.h"
#include "types.h"
#include "search.h"
#include "move.h"
#include "movegen.h"
#include "movegentest.h"
#include "zorbist.h"

int main(){
    //moveGenTest();
    //return 0;
    
    Board board;
    //initalizeBoard(&board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    //initalizeBoard(&board,"6k1/pp4b1/b6p/2n2Np1/1PP3R1/8/P2rBPPP/6K1 w - - 1 31");
    
    //initalizeBoard(&board,"4k3/3ppp2/8/8/8/8/3PPP2/4K3 w - - 0 1");
    //initalizeBoard(&board,"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w - - 0 1");
    
    initalizeBoard(&board,"rnq3k1/p3b1pp/1pp1p3/3n1r2/1PNP4/P4N2/4QPPP/R1B2RK1 w - - 6 19");
    //initalizeBoard(&board,"1k6/5Q2/1K6/8/8/8/8/8 w - - 0 1");
    
    // STALEMATE
    //initalizeBoard(&board,"8/1P2R3/7k/3N1K2/8/6P1/8/8 w - - 9 62");
    
    // TEST FOR FINDIND MATES EARLIER   
    //initalizeBoard(&board,"8/7p/7P/6p1/8/4kr2/8/4K3 b - - 1 63");
    
    // TEST FOR THROWING SUFFICIENT MATERIAL
    //initalizeBoard(&board,"8/2k5/3N4/PP1K4/8/4b3/8/8 w - - 7 104");
    
    // TESTING FOR PROMOTION
    //initalizeBoard(&board,"8/8/2PPP3/K7/8/8/8/k7 w - - 0 0");
    
    // FAILED ENDGAME MATE WITH KRvR
    //initalizeBoard(&board,"8/2K5/1r6/8/8/3k4/8/8 b - - 51 119");
    
    getBestMove(&board,10,0);
    //getBestMove(&board,10,0);
}
