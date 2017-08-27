/*
  Ethereal is a UCI chess playing engine authored by Andrew Grant.
  <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>
  
  Ethereal is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  Ethereal is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#include "bitboards.h"
#include "bitutils.h"
#include "board.h"
#include "castle.h"
#include "magics.h"
#include "masks.h"
#include "piece.h"
#include "psqt.h"
#include "search.h"
#include "time.h"
#include "transposition.h"
#include "types.h"
#include "move.h"
#include "movegen.h"
#include "zorbist.h"

#define NUM_BENCHMARKS (21)

char Benchmarks[NUM_BENCHMARKS][256] = { // StockFish:benchmark.cpp
	"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
	"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 10",
	"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 11",
	"4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
	"rq3rk1/ppp2ppp/1bnpb3/3N2B1/3NP3/7P/PPPQ1PP1/2KR3R w - - 7 14",
	"r1bq1r1k/1pp1n1pp/1p1p4/4p2Q/4Pp2/1BNP4/PPP2PPP/3R1RK1 w - - 2 14",
	"r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15",
	"r1bbk1nr/pp3p1p/2n5/1N4p1/2Np1B2/8/PPP2PPP/2KR1B1R w kq - 0 13",
	"r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16",
	"4r1k1/r1q2ppp/ppp2n2/4P3/5Rb1/1N1BQ3/PPP3PP/R5K1 w - - 1 17",
	"2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11",
	"r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16",
	"3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22",
	"r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18",
	"2K5/p7/7P/5pR1/8/5k2/r7/8 w - - 0 1",
	"8/6pk/1p6/8/PP3p1p/5P2/4KP1q/3Q4 w - - 0 1",
	"7k/3p2pp/4q3/8/4Q3/5Kp1/P6b/8 w - - 0 1",
	"5k2/7R/4P2p/5K2/p1r2P1p/8/8/8 b - - 0 1",
	"6k1/6p1/P6p/r1N5/5p2/7P/1b3PP1/4R1K1 w - - 0 1",
	"1r3k2/4q3/2Pp3b/3Bp3/2Q2p2/1p1P2P1/1P2KP2/3N4 w - - 0 1",
	"6k1/4pp1p/3p2p1/P1pPb3/R7/1r2P1PP/3B1P2/6K1 w - - 0 1",
};

/**
 * Initalize a given board struct based on the passed FEN position
 *
 * @param   board   Board pointer to store information
 * @param   fen     FEN string to create position from
 */
void initalizeBoard(Board * board, char * fen){
    
    int i, j, sq;
    char rank, file;
    uint64_t enemyPawns;
    
    // Init board->squares from FEN notation;
    for(i = 0, sq = 56; fen[i] != ' '; i++){
        
        // End of a row of the FEN notation
        if (fen[i] == '/' || fen[i] == '\\'){
            sq -= 16;
            continue;
        }
        
        // Initalize any empty squares
        else if (fen[i] <= '8' && fen[i] >= '1')
            for(j = 0; j < fen[i] - '0'; j++, sq++)
                board->squares[sq] = EMPTY;
            
        // Index contains an actual piece, determine its type
        else {
            switch(fen[i]){
                case 'P': board->squares[sq++] = WHITE_PAWN;   break;
                case 'N': board->squares[sq++] = WHITE_KNIGHT; break;
                case 'B': board->squares[sq++] = WHITE_BISHOP; break;
                case 'R': board->squares[sq++] = WHITE_ROOK;   break;
                case 'Q': board->squares[sq++] = WHITE_QUEEN;  break;
                case 'K': board->squares[sq++] = WHITE_KING;   break;
                case 'p': board->squares[sq++] = BLACK_PAWN;   break;
                case 'n': board->squares[sq++] = BLACK_KNIGHT; break;
                case 'b': board->squares[sq++] = BLACK_BISHOP; break;
                case 'r': board->squares[sq++] = BLACK_ROOK;   break;
                case 'q': board->squares[sq++] = BLACK_QUEEN;  break;
                case 'k': board->squares[sq++] = BLACK_KING;   break;
            }
        }
    }
    
    // Determine turn
    switch(fen[++i]){
        case 'w': board->turn = WHITE; break;
        case 'b': board->turn = BLACK; break;
    }
    
    i++; // Skip over space between turn and rights
    
    // Determine Castle Rights
    board->castleRights = 0;
    while(fen[++i] != ' '){
        switch(fen[i]){
            case 'K' : board->castleRights |= WHITE_KING_RIGHTS;  break;
            case 'Q' : board->castleRights |= WHITE_QUEEN_RIGHTS; break;
            case 'k' : board->castleRights |= BLACK_KING_RIGHTS;  break;
            case 'q' : board->castleRights |= BLACK_QUEEN_RIGHTS; break;
            case '-' : break;
        }
    }
    
    // Determine Enpass Square
    board->epSquare = -1;
    if (fen[++i] != '-'){
        rank = fen[i] - 'a';
        file = fen[++i] - '1';
        
        board->epSquare = rank + (8 * file);
    }
    
    i++; // Skip over space between ensquare and halfmove count
        
    // Determine Number of half moves into the fifty move rule
    board->fiftyMoveRule = 0;
    if (fen[++i] != '-'){
        
        // Two Digit Number
        if (fen[i+1] != ' '){ 
            board->fiftyMoveRule = (10 * (fen[i] - '0')) + fen[i+1] - '0';
            i++;
        }
        
        // One Digit Number
        else
            board->fiftyMoveRule = fen[i] - '0';
    }
    
    // Zero out each of the used BitBoards
    board->colours[WHITE] = 0ull;
    board->colours[BLACK] = 0ull;
    board->pieces[PAWN]   = 0ull;
    board->pieces[KNIGHT] = 0ull;
    board->pieces[BISHOP] = 0ull;
    board->pieces[ROOK]   = 0ull;
    board->pieces[QUEEN]  = 0ull;
    board->pieces[KING]   = 0ull;
    
    // Initalize each of the BitBoards
    for(i = 0; i < 64; i++){
        board->colours[PieceColour(board->squares[i])] |= (1ull << i);
        board->pieces[PieceType(board->squares[i])]    |= (1ull << i);
    }
    
    // Update the enpass square to reflect not only a potential
    // enpass, but that an enpass is actually possible
    if (board->epSquare != -1){
        enemyPawns = board->colours[!board->turn] & board->pieces[PAWN];
        enemyPawns &= IsolatedPawnMasks[board->epSquare];;
        enemyPawns &= (board->turn == BLACK) ? RANK_4 : RANK_5;
        if (enemyPawns == 0ull) board->epSquare = -1;
    }
    
    // Inititalize Zorbist Hash
    for(i = 0, board->hash = 0; i < 64; i++)
        board->hash ^= ZorbistKeys[board->squares[i]][i];
    
    // Factor in the castling rights
    board->hash ^= ZorbistKeys[CASTLE][board->castleRights];
    
    // Factor in the enpass square
    if (board->epSquare != -1)
        board->hash ^= ZorbistKeys[ENPASS][File(board->epSquare)];
    
    // Factor in the turn
    if (board->turn == BLACK)
        board->hash ^= ZorbistKeys[TURN][0];
    
    // Inititalize Pawn Hash
    for (i = 0, board->phash = 0; i < 64; i++)
        board->phash ^= PawnKeys[board->squares[i]][i];
        
    // Initalize Piece Square and Material value counters
    for(i = 0, board->opening = 0, board->endgame = 0; i < 64; i++){
        board->opening += PSQTopening[board->squares[i]][i];
        board->endgame += PSQTendgame[board->squares[i]][i];
    }
    
    // Number of moves since this position
    board->numMoves = 0;
    
    // We cannot actually determine whether or not a castle took
    // place, but we do not care, as we only put value on castles
    // that have occured after the root of a search.
    board->hasCastled[0] = 0;
    board->hasCastled[1] = 0;
}

/**
 * Print a given board in a user-friendly manner.
 *
 * @param   board   Board pointer to board to print
 */
void printBoard(Board * board){
    
    int i, j, f, c, t;
    
    static const char table[3][7] = {
        {'P','N','B','R','Q','K'},
        {'p','n','b','r','q','k'},
        {' ',' ',' ',' ',' ',' '} 
    };
    
    // Print each row of the board, starting from the top
    for(i = 56, f = 8; i >= 0; i-=8, f--){
        
        printf("\n     |----|----|----|----|----|----|----|----|\n");
        printf("    %d",f);
        
        // Print each square in a row, starting from the left
        for(j = 0; j < 8; j++){
            c = PieceColour(board->squares[i+j]);
            t = PieceType(board->squares[i+j]);
            
            switch(c){
                case WHITE: printf("| *%c ", table[c][t]); break;
                case BLACK: printf("|  %c ", table[c][t]); break;
                default   : printf("|    "); break;
            }
        }
        
        printf("|");
    }
    
    printf("\n     |----|----|----|----|----|----|----|----|");
    printf("\n        A    B    C    D    E    F    G    H\n");
}

/**
 * Perform the Performance test move path enumeration recursivly.
 * This is only used for testing the move generation algorithm.
 *
 * @param   board   Board pointer to current position
 * @param   depth   Counter for recursive cut-off
 *
 * @return          Number of positions found
 */
uint64_t perft(Board * board, int depth){
    
    Undo undo[1];
    int size = 0;
    uint64_t found = 0ull;
    uint16_t moves[MAX_MOVES];
    
    if (depth == 0) return 1ull;
    
    genAllMoves(board, moves, &size);
    
    // Recurse on all valid moves
    for(size -= 1; size >= 0; size--){
        applyMove(board, moves[size], undo);
        if (isNotInCheck(board, !board->turn))
            found += perft(board, depth-1);        
        revertMove(board, moves[size], undo);
    }
    
    return found;
}

/**
 * Search each of the benchmark positions to a given depth.
 * Quick way of checking non functional speedups.
 *
 * @param   depth   Search depth for each position
 */
void runBenchmark(int depth){

    int i;
    double start, end;
    
    SearchInfo info;
    info.searchIsInfinite = 0;
    info.searchIsDepthLimited = 1;
    info.searchIsTimeLimited = 0;
    info.depthLimit = depth;
    info.terminateSearch = 0;

    start = getRealTime();
    
    // Search each benchmark position
    for (i = 0; i < NUM_BENCHMARKS; i++){
        info.startTime = getRealTime();
        initalizeBoard((&info.board), Benchmarks[i]);
        clearTranspositionTable(&Table);
        getBestMove(&info);
    }
    
    end = getRealTime();
    
    printf("Benchtime = %dms\n", (int)(end - start));    
}