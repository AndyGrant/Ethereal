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


#include <stdlib.h>
#include <stdio.h>

#include "bitboards.h"
#include "tests.h"
#include "types.h"
#include "board.h"
#include "history.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "search.h"

extern HistoryTable History;

int searchDepth = 5;

int numberOfTests = 126;

int testPositionsNodeCounts[126] = {
    4865609, 193690690,  133987,   145232,   47635,   52710,
     532933,    118882,   37735,    80619,   10485,   20780,
    7594526,   8153719, 7736373,  7878456, 8198901, 7710115,
    7848606,     47635,   52710,   133987,  145232,  118882,
     532933,     10485,   20780,    37735,   80619, 7594526,
    8198901,   7710115, 7848606,  8153719, 7736373, 7878456,
     570726,    223507, 1198299,    38348,   92250,  582642,
     288141,    281190,   92250,    38348, 1320507, 1713368,
     787524,    310862,  530585,  1591064,  310862,  787524,
    2161211,  20506480, 2161211, 20521342,   14893,   14893,
     166741,    105749,   14893,   166741,    1347,    1347,
        342,       342,    7028,      342,     342,    1347,
       1347,      5408,    1814,     1814,    1969,    1969,
       8296,     23599,   21637,     3450,    1969,    1969,
       8296,     21637,   23599,     3309,    4661,    4786,
       6112,      4354,    6112,     4354,    3013,    4271,
       5014,      4658,    6112,     4354,    6112,    4354,
       4337,      4271,    6112,     4354,    6112,    4354,
       7574,      7574,   24122,     6112,    4354,    6112,
       4354,      7574,    7574,    24122,   90606, 2193768,
    1533145,   3605103,   90606,  2193768, 1533145, 3605103,  
};

char * testPositions[126] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ",
    "4k3/8/8/8/8/8/8/4K2R w K - 0 1 ",
    "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1 ",
    "4k2r/8/8/8/8/8/8/4K3 w k - 0 1 ",
    "r3k3/8/8/8/8/8/8/4K3 w q - 0 1 ",
    "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1 ",
    "r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1 ",
    "8/8/8/8/8/8/6k1/4K2R w K - 0 1 ",
    "8/8/8/8/8/8/1k6/R3K3 w Q - 0 1 ",
    "4k2r/6K1/8/8/8/8/8/8 w k - 0 1 ",
    "r3k3/1K6/8/8/8/8/8/8 w q - 0 1 ",
    "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1 ",
    "r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1 ",
    "r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1 ",
    "r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1 ",
    "1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1 ",
    "2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1 ",
    "r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1 ",
    "4k3/8/8/8/8/8/8/4K2R b K - 0 1 ",
    "4k3/8/8/8/8/8/8/R3K3 b Q - 0 1 ",
    "4k2r/8/8/8/8/8/8/4K3 b k - 0 1 ",
    "r3k3/8/8/8/8/8/8/4K3 b q - 0 1 ",
    "4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1 ",
    "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1 ",
    "8/8/8/8/8/8/6k1/4K2R b K - 0 1 ",
    "8/8/8/8/8/8/1k6/R3K3 b Q - 0 1 ",
    "4k2r/6K1/8/8/8/8/8/8 b k - 0 1 ",
    "r3k3/1K6/8/8/8/8/8/8 b q - 0 1 ",
    "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1 ",
    "r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1 ",
    "r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1 ",
    "r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1 ",
    "1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1 ",
    "2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1 ",
    "r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1 ",
    "8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1 ",
    "8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1 ",
    "8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1 ",
    "K7/8/2n5/1n6/8/8/8/k6N w - - 0 1 ",
    "k7/8/2N5/1N6/8/8/8/K6n w - - 0 1 ",
    "8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1 ",
    "8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1 ",
    "8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1 ",
    "K7/8/2n5/1n6/8/8/8/k6N b - - 0 1 ",
    "k7/8/2N5/1N6/8/8/8/K6n b - - 0 1 ",
    "B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1 ",
    "8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1 ",
    "k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1 ",
    "K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1 ",
    "B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1 ",
    "8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1 ",
    "k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1 ",
    "K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1 ",
    "7k/RR6/8/8/8/8/rr6/7K w - - 0 1 ",
    "R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1 ",
    "7k/RR6/8/8/8/8/rr6/7K b - - 0 1 ",
    "R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1 ",
    "6kq/8/8/8/8/8/8/7K w - - 0 1 ",
    "6KQ/8/8/8/8/8/8/7k b - - 0 1 ",
    "K7/8/8/3Q4/4q3/8/8/7k w - - 0 1 ",
    "6qk/8/8/8/8/8/8/7K b - - 0 1 ",
    "6KQ/8/8/8/8/8/8/7k b - - 0 1 ",
    "K7/8/8/3Q4/4q3/8/8/7k b - - 0 1 ",
    "8/8/8/8/8/K7/P7/k7 w - - 0 1 ",
    "8/8/8/8/8/7K/7P/7k w - - 0 1 ",
    "K7/p7/k7/8/8/8/8/8 w - - 0 1 ",
    "7K/7p/7k/8/8/8/8/8 w - - 0 1 ",
    "8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1 ",
    "8/8/8/8/8/K7/P7/k7 b - - 0 1 ",
    "8/8/8/8/8/7K/7P/7k b - - 0 1 ",
    "K7/p7/k7/8/8/8/8/8 b - - 0 1 ",
    "7K/7p/7k/8/8/8/8/8 b - - 0 1 ",
    "8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1 ",
    "8/8/8/8/8/4k3/4P3/4K3 w - - 0 1 ",
    "4k3/4p3/4K3/8/8/8/8/8 b - - 0 1 ",
    "8/8/7k/7p/7P/7K/8/8 w - - 0 1 ",
    "8/8/k7/p7/P7/K7/8/8 w - - 0 1 ",
    "8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1 ",
    "8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1 ",
    "8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1 ",
    "k7/8/3p4/8/3P4/8/8/7K w - - 0 1 ",
    "8/8/7k/7p/7P/7K/8/8 b - - 0 1 ",
    "8/8/k7/p7/P7/K7/8/8 b - - 0 1 ",
    "8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1 ",
    "8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1 ",
    "8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1 ",
    "k7/8/3p4/8/3P4/8/8/7K b - - 0 1 ",
    "7k/3p4/8/8/3P4/8/8/K7 w - - 0 1 ",
    "7k/8/8/3p4/8/8/3P4/K7 w - - 0 1 ",
    "k7/8/8/7p/6P1/8/8/K7 w - - 0 1 ",
    "k7/8/7p/8/8/6P1/8/K7 w - - 0 1 ",
    "k7/8/8/6p1/7P/8/8/K7 w - - 0 1 ",
    "k7/8/6p1/8/8/7P/8/K7 w - - 0 1 ",
    "k7/8/8/3p4/4p3/8/8/7K w - - 0 1 ",
    "k7/8/3p4/8/8/4P3/8/7K w - - 0 1 ",
    "7k/3p4/8/8/3P4/8/8/K7 b - - 0 1 ",
    "7k/8/8/3p4/8/8/3P4/K7 b - - 0 1 ",
    "k7/8/8/7p/6P1/8/8/K7 b - - 0 1 ",
    "k7/8/7p/8/8/6P1/8/K7 b - - 0 1 ",
    "k7/8/8/6p1/7P/8/8/K7 b - - 0 1 ",
    "k7/8/6p1/8/8/7P/8/K7 b - - 0 1 ",
    "k7/8/8/3p4/4p3/8/8/7K b - - 0 1 ",
    "k7/8/3p4/8/8/4P3/8/7K b - - 0 1 ",
    "7k/8/8/p7/1P6/8/8/7K w - - 0 1 ",
    "7k/8/p7/8/8/1P6/8/7K w - - 0 1 ",
    "7k/8/8/1p6/P7/8/8/7K w - - 0 1 ",
    "7k/8/1p6/8/8/P7/8/7K w - - 0 1 ",
    "k7/7p/8/8/8/8/6P1/K7 w - - 0 1 ",
    "k7/6p1/8/8/8/8/7P/K7 w - - 0 1 ",
    "3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1 ",
    "7k/8/8/p7/1P6/8/8/7K b - - 0 1 ",
    "7k/8/p7/8/8/1P6/8/7K b - - 0 1 ",
    "7k/8/8/1p6/P7/8/8/7K b - - 0 1 ",
    "7k/8/1p6/8/8/P7/8/7K b - - 0 1 ",
    "k7/7p/8/8/8/8/6P1/K7 b - - 0 1 ",
    "k7/6p1/8/8/8/8/7P/K7 b - - 0 1 ",
    "3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1 ",
    "8/Pk6/8/8/8/8/6Kp/8 w - - 0 1 ",
    "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1 ",
    "8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1 ",
    "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1 ",
    "8/Pk6/8/8/8/8/6Kp/8 b - - 0 1 ",
    "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1 ",
    "8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1 ",
    "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1 ",
};

void runTestSuite(){
    
    int i;
    Board board;
    int found, expected;
    
    // Run through each test position
    for (i = 0; i < numberOfTests; i++){
        printf("Running %s\n", testPositions[i]);
        initalizeBoard(&board, testPositions[i]);
        
        found = perftTesting(&board, searchDepth);
        expected = testPositionsNodeCounts[i];
        
        if (found != expected){
            printf("Invalid PERFT counts [%9d of %9d]\n", found, expected);
            printBoard(&board);
            printf("\n\n");
        }
    }
    
    printf("\nALL TEST POSITIONS FINISHED\n");
}

int perftTesting(Board * board, int depth){
    
    Undo undo[1];
    int i, j, legality, contains, found = 0;
    int size = 0, noisySize = 0, quietSize = 0;
    uint16_t move, moves[MAX_MOVES], noisy[MAX_MOVES], quiet[MAX_MOVES];
    int selectionSize = 0;
    uint16_t selectionMoves[MAX_MOVES];
    MovePicker mp;

    if (depth == 0) return 1;
    
    genAllMoves(board, moves, &size);
    genAllNoisyMoves(board, noisy, &noisySize);
    genAllQuietMoves(board, quiet, &quietSize);
    
    // Verify that Noisy + Quiet has the same size as All Moves
    if (size != noisySize + quietSize){
        printf("Quiet and Noisy do not make up All Moves");
        printBoard(board);
        printf("\n\n");
    }
    
    // Verify that Noisy moves is a subset of All Moves
    for (i = 0; i < noisySize; i++){
        for (j = 0; j < size; j++)
            if (noisy[i] == moves[j])
                break;
            
        if (j == size)
            printMoveErrorMessage(board, noisy[i], "Noisy not in All Moves");
    }
    
    // Verify that Quiet moves is a subset of All Moves
    for (i = 0; i < quietSize; i++){
        for (j = 0; j < size; j++)
            if (quiet[i] == moves[j])
                break;
            
        if (j == size)
            printMoveErrorMessage(board, quiet[i], "Quiet not in All Moves");
    }
        
    // Exhaustivly test every single move for validity
    // Note, we only need to test promotions of one type
    for (i = 0; i < 0x10000; i++){
        
        legality = moveIsPsuedoLegal(board, i);
        
        move = (uint16_t)i;
        
        for (contains = 0, j = 0; j < size; j++){
            if (moves[j] == move){
                contains = 1; 
                break;
            }
        }
            
        if (legality && !contains)
            printMoveErrorMessage(board, move, "Wrongly marked as Legal");
        
        if (!legality && contains)
            printMoveErrorMessage(board, move, "Wrongly marked as Illegal");
    }
    
    
    /* Verification that the move picker will go through
       every move presented for a given position */
       
    initalizeMovePicker(&mp, 0, NULL_MOVE, NULL_MOVE, NULL_MOVE);
    while ((move = selectNextMove(&mp, board)) != NONE_MOVE)
        selectionMoves[selectionSize++] = move;
    
    for (i = 0; i < selectionSize; i++)
        for (j = 0; j < selectionSize; j++)
            if (i != j && selectionMoves[i] == selectionMoves[j])
                printf("Double selection");
            
    if (selectionSize > size)
        printf("Extra Moves");
    
    if (selectionSize < size)
        printf("Missed Moves");
    
    
    // Perform the perft calculations
    for(size -= 1; size >= 0; size--){
        applyMove(board, moves[size], undo);
        if (isNotInCheck(board, !board->turn))
            found += perft(board, depth-1);
        revertMove(board, moves[size], undo);
    }
    
    return found;
}

void printMoveErrorMessage(Board * board, uint16_t move, char * msg){
    
    static char * table[4] = {
        "Normal", "Castle", "Enpass", "Promotion"
    };
    
    int moveType = MoveType(move);
    int from = MoveFrom(move);
    int to = MoveTo(move);
    
    printf("%s\nMove: Type=%s From=%c%c To=%c%c\n",
           msg, table[moveType >> 12],
           'A' + File(from), '1' + Rank(from),
           'A' + File(to), '1' + Rank(to));
    
    printBoard(board);
    printf("\n\n");
}
