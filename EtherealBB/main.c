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
    //move_gen_test();
    //return 0;
    
    Board board;
    init_board(&board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    //init_board(&board,"4k3/3ppp2/8/8/8/8/3PPP2/4K3 w - - 0 1");
    //init_board(&board,"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w - - 0 1");
    //init_board(&board,"1k6/5Q2/1K6/8/8/8/8/8 w - - 0 1");
    
    
    // TEST FOR FINDIND MATES EARLIER   
    //init_board(&board,"8/7p/7P/6p1/8/4kr2/8/4K3 b - - 1 63");
    
    
    get_best_move(&board,12);
}