#include "evaluate.h"
#include "piece.h"
#include "psqt.h"

void init_psqt(){
    int i, j;
    
    for(i = 0; i < 32; i++){
        for(j = 0; j < 64; j++){
            PSQTopening[i][j] = 0;
            PSQTendgame[i][j] = 0;
        }
    }
    
    for(i = 0, j = 0; i < 64; i += 8, j += 4){
        
        // Opening Pawn
        PSQTopening[WhitePawn][i+0] = PawnValue + PawnOpeningMap32[j+0];
        PSQTopening[WhitePawn][i+7] = PawnValue + PawnOpeningMap32[j+0];
        PSQTopening[WhitePawn][i+1] = PawnValue + PawnOpeningMap32[j+1];
        PSQTopening[WhitePawn][i+6] = PawnValue + PawnOpeningMap32[j+1];
        PSQTopening[WhitePawn][i+2] = PawnValue + PawnOpeningMap32[j+2];
        PSQTopening[WhitePawn][i+5] = PawnValue + PawnOpeningMap32[j+2];
        PSQTopening[WhitePawn][i+3] = PawnValue + PawnOpeningMap32[j+3];
        PSQTopening[WhitePawn][i+4] = PawnValue + PawnOpeningMap32[j+3];

        // Endgame Pawn                                
        PSQTendgame[WhitePawn][i+0] = PawnValue + PawnEndgameMap32[j+0];
        PSQTendgame[WhitePawn][i+7] = PawnValue + PawnEndgameMap32[j+0];
        PSQTendgame[WhitePawn][i+1] = PawnValue + PawnEndgameMap32[j+1];
        PSQTendgame[WhitePawn][i+6] = PawnValue + PawnEndgameMap32[j+1];
        PSQTendgame[WhitePawn][i+2] = PawnValue + PawnEndgameMap32[j+2];
        PSQTendgame[WhitePawn][i+5] = PawnValue + PawnEndgameMap32[j+2];
        PSQTendgame[WhitePawn][i+3] = PawnValue + PawnEndgameMap32[j+3];
        PSQTendgame[WhitePawn][i+4] = PawnValue + PawnEndgameMap32[j+3];
        
        // Opening Knight
        PSQTopening[WhiteKnight][i+0] = KnightValue + KnightOpeningMap32[j+0];
        PSQTopening[WhiteKnight][i+7] = KnightValue + KnightOpeningMap32[j+0];
        PSQTopening[WhiteKnight][i+1] = KnightValue + KnightOpeningMap32[j+1];
        PSQTopening[WhiteKnight][i+6] = KnightValue + KnightOpeningMap32[j+1];
        PSQTopening[WhiteKnight][i+2] = KnightValue + KnightOpeningMap32[j+2];
        PSQTopening[WhiteKnight][i+5] = KnightValue + KnightOpeningMap32[j+2];
        PSQTopening[WhiteKnight][i+3] = KnightValue + KnightOpeningMap32[j+3];
        PSQTopening[WhiteKnight][i+4] = KnightValue + KnightOpeningMap32[j+3];

        // Ending Knight
        PSQTendgame[WhiteKnight][i+0] = KnightValue + KnightEndgameMap32[j+0];
        PSQTendgame[WhiteKnight][i+7] = KnightValue + KnightEndgameMap32[j+0];
        PSQTendgame[WhiteKnight][i+1] = KnightValue + KnightEndgameMap32[j+1];
        PSQTendgame[WhiteKnight][i+6] = KnightValue + KnightEndgameMap32[j+1];
        PSQTendgame[WhiteKnight][i+2] = KnightValue + KnightEndgameMap32[j+2];
        PSQTendgame[WhiteKnight][i+5] = KnightValue + KnightEndgameMap32[j+2];
        PSQTendgame[WhiteKnight][i+3] = KnightValue + KnightEndgameMap32[j+3];
        PSQTendgame[WhiteKnight][i+4] = KnightValue + KnightEndgameMap32[j+3];
        
        // Opening Bishop
        PSQTopening[WhiteBishop][i+0] = BishopValue + BishopOpeningMap32[j+0];
        PSQTopening[WhiteBishop][i+7] = BishopValue + BishopOpeningMap32[j+0];
        PSQTopening[WhiteBishop][i+1] = BishopValue + BishopOpeningMap32[j+1];
        PSQTopening[WhiteBishop][i+6] = BishopValue + BishopOpeningMap32[j+1];
        PSQTopening[WhiteBishop][i+2] = BishopValue + BishopOpeningMap32[j+2];
        PSQTopening[WhiteBishop][i+5] = BishopValue + BishopOpeningMap32[j+2];
        PSQTopening[WhiteBishop][i+3] = BishopValue + BishopOpeningMap32[j+3];
        PSQTopening[WhiteBishop][i+4] = BishopValue + BishopOpeningMap32[j+3];

        // Ending Bishop                
        PSQTendgame[WhiteBishop][i+0] = BishopValue + BishopEndgameMap32[j+0];
        PSQTendgame[WhiteBishop][i+7] = BishopValue + BishopEndgameMap32[j+0];
        PSQTendgame[WhiteBishop][i+1] = BishopValue + BishopEndgameMap32[j+1];
        PSQTendgame[WhiteBishop][i+6] = BishopValue + BishopEndgameMap32[j+1];
        PSQTendgame[WhiteBishop][i+2] = BishopValue + BishopEndgameMap32[j+2];
        PSQTendgame[WhiteBishop][i+5] = BishopValue + BishopEndgameMap32[j+2];
        PSQTendgame[WhiteBishop][i+3] = BishopValue + BishopEndgameMap32[j+3];
        PSQTendgame[WhiteBishop][i+4] = BishopValue + BishopEndgameMap32[j+3];
        
        // Opening Rook
        PSQTopening[WhiteRook][i+0] = RookValue + RookOpeningMap32[j+0];
        PSQTopening[WhiteRook][i+7] = RookValue + RookOpeningMap32[j+0];
        PSQTopening[WhiteRook][i+1] = RookValue + RookOpeningMap32[j+1];
        PSQTopening[WhiteRook][i+6] = RookValue + RookOpeningMap32[j+1];
        PSQTopening[WhiteRook][i+2] = RookValue + RookOpeningMap32[j+2];
        PSQTopening[WhiteRook][i+5] = RookValue + RookOpeningMap32[j+2];
        PSQTopening[WhiteRook][i+3] = RookValue + RookOpeningMap32[j+3];
        PSQTopening[WhiteRook][i+4] = RookValue + RookOpeningMap32[j+3];

        // Ending Rook                
        PSQTendgame[WhiteRook][i+0] = RookValue + RookEndgameMap32[j+0];
        PSQTendgame[WhiteRook][i+7] = RookValue + RookEndgameMap32[j+0];
        PSQTendgame[WhiteRook][i+1] = RookValue + RookEndgameMap32[j+1];
        PSQTendgame[WhiteRook][i+6] = RookValue + RookEndgameMap32[j+1];
        PSQTendgame[WhiteRook][i+2] = RookValue + RookEndgameMap32[j+2];
        PSQTendgame[WhiteRook][i+5] = RookValue + RookEndgameMap32[j+2];
        PSQTendgame[WhiteRook][i+3] = RookValue + RookEndgameMap32[j+3];
        PSQTendgame[WhiteRook][i+4] = RookValue + RookEndgameMap32[j+3];
        
        // Opening Queen
        PSQTopening[WhiteQueen][i+0] = QueenValue + QueenOpeningMap32[j+0];
        PSQTopening[WhiteQueen][i+7] = QueenValue + QueenOpeningMap32[j+0];
        PSQTopening[WhiteQueen][i+1] = QueenValue + QueenOpeningMap32[j+1];
        PSQTopening[WhiteQueen][i+6] = QueenValue + QueenOpeningMap32[j+1];
        PSQTopening[WhiteQueen][i+2] = QueenValue + QueenOpeningMap32[j+2];
        PSQTopening[WhiteQueen][i+5] = QueenValue + QueenOpeningMap32[j+2];
        PSQTopening[WhiteQueen][i+3] = QueenValue + QueenOpeningMap32[j+3];
        PSQTopening[WhiteQueen][i+4] = QueenValue + QueenOpeningMap32[j+3];
        
        // Ending Queen
        PSQTendgame[WhiteQueen][i+0] = QueenValue + QueenEndgameMap32[j+0];
        PSQTendgame[WhiteQueen][i+7] = QueenValue + QueenEndgameMap32[j+0];
        PSQTendgame[WhiteQueen][i+1] = QueenValue + QueenEndgameMap32[j+1];
        PSQTendgame[WhiteQueen][i+6] = QueenValue + QueenEndgameMap32[j+1];
        PSQTendgame[WhiteQueen][i+2] = QueenValue + QueenEndgameMap32[j+2];
        PSQTendgame[WhiteQueen][i+5] = QueenValue + QueenEndgameMap32[j+2];
        PSQTendgame[WhiteQueen][i+3] = QueenValue + QueenEndgameMap32[j+3];
        PSQTendgame[WhiteQueen][i+4] = QueenValue + QueenEndgameMap32[j+3];
        
        // Opening King
        PSQTopening[WhiteKing][i+0] = KingValue + KingOpeningMap32[j+0];
        PSQTopening[WhiteKing][i+7] = KingValue + KingOpeningMap32[j+0];
        PSQTopening[WhiteKing][i+1] = KingValue + KingOpeningMap32[j+1];
        PSQTopening[WhiteKing][i+6] = KingValue + KingOpeningMap32[j+1];
        PSQTopening[WhiteKing][i+2] = KingValue + KingOpeningMap32[j+2];
        PSQTopening[WhiteKing][i+5] = KingValue + KingOpeningMap32[j+2];
        PSQTopening[WhiteKing][i+3] = KingValue + KingOpeningMap32[j+3];
        PSQTopening[WhiteKing][i+4] = KingValue + KingOpeningMap32[j+3];
        
        // Ending King
        PSQTendgame[WhiteKing][i+0] = KingValue + KingEndgameMap32[j+0];
        PSQTendgame[WhiteKing][i+7] = KingValue + KingEndgameMap32[j+0];
        PSQTendgame[WhiteKing][i+1] = KingValue + KingEndgameMap32[j+1];
        PSQTendgame[WhiteKing][i+6] = KingValue + KingEndgameMap32[j+1];
        PSQTendgame[WhiteKing][i+2] = KingValue + KingEndgameMap32[j+2];
        PSQTendgame[WhiteKing][i+5] = KingValue + KingEndgameMap32[j+2];
        PSQTendgame[WhiteKing][i+3] = KingValue + KingEndgameMap32[j+3];
        PSQTendgame[WhiteKing][i+4] = KingValue + KingEndgameMap32[j+3];    
    }
    
    for(i = BlackPawn; i <= BlackKing; i+= 4){
        for(j = 0; j < 64; j++){
            PSQTopening[i][j] = -PSQTopening[i-1][InversionTable[j]];
            PSQTendgame[i][j] = -PSQTendgame[i-1][InversionTable[j]];
        }
    }
}