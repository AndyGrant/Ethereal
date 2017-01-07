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

#include "bitboards.h"
#include "bitutils.h"
#include "board.h"
#include "castle.h"
#include "magics.h"
#include "masks.h"
#include "piece.h"
#include "psqt.h"
#include "types.h"
#include "move.h"
#include "movegen.h"
#include "zorbist.h"

/**
 * Initalize a given board struct based on the passed
 * FEN position. Once the position has been setup in 
 * the BitBoards, determine the hash-signature of the 
 * current board as well as the opening and endgame values.
 *
 * @param   board   Board pointer to store information
 * @param   fen     FEN string to create position from
 */
void initalizeBoard(Board * board, char * fen){
    
    int i, j, sq;
    char r, f;
    
    // Init board->squares from fen notation;
    for(i = 0, sq = 56; fen[i] != ' '; i++){
        if (fen[i] == '/' || fen[i] == '\\'){
            sq -= 16;
            continue;
        }
        
        else if (fen[i] <= '8' && fen[i] >= '1')
            for(j = 0; j < fen[i] - '0'; j++, sq++)
                board->squares[sq] = EMPTY;
            
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
            case 'K' : board->castleRights |= WHITE_KING_RIGHTS; break;
            case 'Q' : board->castleRights |= WHITE_QUEEN_RIGHTS; break;
            case 'k' : board->castleRights |= BLACK_KING_RIGHTS; break;
            case 'q' : board->castleRights |= BLACK_QUEEN_RIGHTS; break;
            case '-' : break;
        }
    }
    
    // Determine Enpass Square
    board->epSquare = -1;
    if (fen[++i] != '-'){
        r = fen[i];
        f = fen[++i];
        
        board->epSquare = (r - 'a') + (8 * (f - '1'));
    }
    
    i++; // Skip over space between ensquare and halfmove count
        
    // Determine Number of half moves into fiftymoverule
    board->fiftyMoveRule = 0;
    if (fen[++i] != '-'){
        
        // Two Digit Number
        if (fen[i+1] != ' '){ 
            board->fiftyMoveRule = (10 * (fen[i] - '0')) + fen[1+i] - '0';
            i++;
        }
        
        // One Digit Number
        else
            board->fiftyMoveRule = fen[i] - '0';
    }
    
    // Set BitBoards to default values
    board->colours[WHITE] = 0ull;
    board->colours[BLACK] = 0ull;
    board->pieces[PAWN]   = 0ull;
    board->pieces[KNIGHT] = 0ull;
    board->pieces[BISHOP] = 0ull;
    board->pieces[ROOK]   = 0ull;
    board->pieces[QUEEN]  = 0ull;
    board->pieces[KING]   = 0ull;
    
    // Set Empty BitBoards to filled
    board->colours[2] = 0xFFFFFFFFFFFFFFFFull;
    board->pieces[6]  = 0xFFFFFFFFFFFFFFFFull;
    
    // Fill BitBoards
    for(i = 0; i < 64; i++){
        board->colours[PieceColour(board->squares[i])] |= (1ull << i);
        board->pieces[PieceType(board->squares[i])]    |= (1ull << i);
    }
    
    // Update the enpass square to reflect not only a potential
    // enpass, but that an enpass may actuall be possible
    if (board->epSquare != -1){
        
        uint64_t enemyPawns;
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
    
    // Inititalize Pawn Hash
    for (i = 0, board->phash = 0; i < 64; i++)
        board->phash ^= PawnKeys[board->squares[i]][i];
    
    board->opening = 0;
    board->endgame = 0;
    
    for(i = 0; i < 64; i++){
        board->opening += PSQTopening[board->squares[i]][i];
        board->endgame += PSQTendgame[board->squares[i]][i];
    }
    
    board->numMoves = 0;
    
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
    
    for(i = 56, f = 8; i >= 0; i-=8, f--){
        
        printf("\n     |----|----|----|----|----|----|----|----|\n");
        printf("    %d",f);
        
        for(j = 0; j < 8; j++){
            c = PieceColour(board->squares[i+j]);
            t = PieceType(board->squares[i+j]);
            
            switch(c){
                case WHITE: printf("| *%c ",table[c][t]); break;
                case BLACK: printf("|  %c ",table[c][t]); break;
                default   : printf("|    "); break;
            }
        }
        
        printf("|");
    }
    
    printf("\n     |----|----|----|----|----|----|----|----|");
    printf("\n        A    B    C    D    E    F    G    H\n");
}

/**
 * Perform the Performance test move path enumeration
 * recursivly. This is only used for testing the algorithm
 * in movegen.c.
 *
 * @param   board   Board pointer to current position
 * @param   depth   Counter for recursive cut-off
 *
 * @return          Number of positions found
 */
int perft(Board * board, int depth){
    
    Undo undo;
    int size = 0, found = 0;
    uint16_t moves[MAX_MOVES];
    
    if (depth == 0)
        return 1;
    
    genAllMoves(board,moves,&size);
    
    for(size -= 1; size >= 0; size--){
        applyMove(board,moves[size],&undo);
        if (isNotInCheck(board,!board->turn))
            found += perft(board,depth-1);        
        revertMove(board,moves[size],&undo);
    }
    
    return found;
}