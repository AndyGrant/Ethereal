#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "board.h"
#include "castle.h"
#include "magics.h"
#include "piece.h"
#include "psqt.h"
#include "types.h"
#include "move.h"
#include "movegen.h"
#include "zorbist.h"


/*
 *  Initialize a Board struct from a given FEN-string.
 
 *  Initializes the PieceSquareTable for updated-by-the-move
 *  scoring for opening and endgame material and positinal values.
 *  
 *  Initializes MoveDatabases for Rooks and Bishops by calling
 *  init_magics() (magics.h)
 *
 *  Initializes ZorbistKeys for the game board
 *
 *  Attempts to Validate given FEN String, throws errors when
 *  Parsing Unexpected FEN-strings
 */
void init_board(Board * board, char * fen){
    int i, j, sq;
    char r, f;
    
    init_magics();
    init_zorbist();
    init_psqt();
    
    // Init board->squares from fen notation;
    for(i = 0, sq = 56; fen[i] != ' '; i++){
        if (fen[i] == '/' || fen[i] == '\\'){
            sq -= 16;
            continue;
        }
        
        else if (fen[i] <= '8' && fen[i] >= '1')
            for(j = 0; j < fen[i] - '0'; j++, sq++)
                board->squares[sq] = Empty;
            
        else {
            switch(fen[i]){
                case 'P': board->squares[sq++] = WhitePawn;   break;
                case 'N': board->squares[sq++] = WhiteKnight; break;
                case 'B': board->squares[sq++] = WhiteBishop; break;
                case 'R': board->squares[sq++] = WhiteRook;   break;
                case 'Q': board->squares[sq++] = WhiteQueen;  break;
                case 'K': board->squares[sq++] = WhiteKing;   break;
                case 'p': board->squares[sq++] = BlackPawn;   break;
                case 'n': board->squares[sq++] = BlackKnight; break;
                case 'b': board->squares[sq++] = BlackBishop; break;
                case 'r': board->squares[sq++] = BlackRook;   break;
                case 'q': board->squares[sq++] = BlackQueen;  break;
                case 'k': board->squares[sq++] = BlackKing;   break;
                default : assert("Unable to decode piece in init_board()" && 0);
            }
        }
    }
    
    // Determine turn
    switch(fen[++i]){
        case 'w': board->turn = ColourWhite; break;
        case 'b': board->turn = ColourBlack; break;
        default : assert("Unable to determine player to move in init_board()" && 0);
    }
    
    i++; // Skip over space between turn and rights
    
    // Determine Castle Rights
    board->castlerights = 0;
    while(fen[++i] != ' '){
        switch(fen[i]){
            case 'K' : board->castlerights |= WhiteKingRights; break;
            case 'Q' : board->castlerights |= WhiteQueenRights; break;
            case 'k' : board->castlerights |= BlackKingRights; break;
            case 'q' : board->castlerights |= BlackQueenRights; break;
            case '-' : break;
            default  : assert("Unable to decode castle rights in init_board()" && 0);
        }
    }
    
    // Determine Enpass Square
    board->epsquare = -1;
    if (fen[++i] != '-'){
        r = fen[i];
        f = fen[++i];
        assert("Unable to decode enpass rank in init_board()" && r>='a' && r<='h');
        assert("Unable to decode enpass file in init_board()" && f>='1' && f<='8');
        
        board->epsquare = (r - 'a') + (8 * (f - '1'));
        
        assert("Incorrectly decoded enpass square in init_board()" && board->epsquare >= 0 && board->epsquare <= 63);
    }
    
    i++; // Skip over space between ensquare and halfmove count
        
    // Determine Number of half moves into fiftymoverule
    board->fiftymoverule = 0;
    if (fen[++i] != '-'){
        // Two Digit Number
        if (fen[i+1] != ' ') 
            board->fiftymoverule = (10 * (fen[i] - '0')) + fen[1+i++] - '0';
        
        // One Digit Number
        else
            board->fiftymoverule = fen[i] - '0';
        
        assert("Incorrectly decoded half-move count in init_board()" && (board->fiftymoverule >=0 && board->fiftymoverule < 100));
    }
    
    // Set BitBoards to default values
    board->colourBitBoards[0] = 0;
    board->colourBitBoards[1] = 0;
    board->pieceBitBoards[0] = 0;
    board->pieceBitBoards[1] = 0;
    board->pieceBitBoards[2] = 0;
    board->pieceBitBoards[3] = 0;
    board->pieceBitBoards[4] = 0;
    board->pieceBitBoards[5] = 0;
    
    // Set Empty BitBoards to filled
    board->colourBitBoards[2] = 0xFFFFFFFFFFFFFFFF;
    board->pieceBitBoards[6] = 0xFFFFFFFFFFFFFFFF;
    
    // Fill BitBoards
    for(i = 0; i < 64; i++){
        board->colourBitBoards[PIECE_COLOUR(board->squares[i])] |= (1ull << i);
        board->pieceBitBoards[PIECE_TYPE(board->squares[i])]    |= (1ull << i);
    }
    
    // Init Zorbist Hash
    for(i = 0, board->hash = 0; i < 64; i++)
        board->hash ^= ZorbistKeys[board->squares[i]][i];
    
    board->opening = 0;
    board->endgame = 0;
    
    for(i = 0; i < 64; i++){
        board->opening += PSQTopening[board->squares[i]][i];
        board->endgame += PSQTendgame[board->squares[i]][i];
    }
    
    board->move_num = 0;
}

/*
 *  Outputs the board to stdout in a human friendly manor,
 *  Including an ascii grid, filled with pieces, as well
 *  as Rank and File markings
 */
void print_board(Board * board){
    int i, j, f, c, t;
    
    char table[3][7] = {
        {'P','N','B','R','Q','K'},
        {'p','n','b','r','q','k'},
        {' ',' ',' ',' ',' ',' '} 
    };
    
    for(i = 56, f = 8; i >= 0; i-=8, f--){
        printf("\n     |----|----|----|----|----|----|----|----|\n");
        printf("    %d",f);
        for(j = 0; j < 8; j++){
            c = PIECE_COLOUR(board->squares[i+j]);
            t = PIECE_TYPE(board->squares[i+j]);
            
            switch(c){
                case ColourWhite: printf("| *%c ",table[c][t]); break;
                case ColourBlack: printf("|  %c ",table[c][t]); break;
                default         : printf("|    "); break;
            }
        }
        
        printf("|");
    }
    
    printf("\n     |----|----|----|----|----|----|----|----|");
    printf("\n        A    B    C    D    E    F    G    H\n");
}

int perft(Board * board, int depth){
    if (depth == 0)
        return 1;
    
    uint16_t moves[256];
    int size = 0;
    gen_all_moves(board,moves,&size);
    Undo undo;
    
    int found = 0;
    for(size -= 1; size >= 0; size--){
        apply_move(board,moves[size],&undo);
        if (is_not_in_check(board,!board->turn))
            found += perft(board,depth-1);
        revert_move(board,moves[size],&undo);
    }
    
    return found;
}