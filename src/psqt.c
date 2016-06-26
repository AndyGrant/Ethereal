#include "evaluate.h"
#include "piece.h"
#include "psqt.h"

int INITALIZED_PSQT = 0;

int PSQTopening[32][64];
int PSQTendgame[32][64];

int InversionTable[64] = {
    56,  57,  58,  59,  60,  61,  62,  63,
    48,  49,  50,  51,  52,  53,  54,  55,
    40,  41,  42,  43,  44,  45,  46,  47,
    32,  33,  34,  35,  36,  37,  38,  39,
    24,  25,  26,  27,  28,  29,  30,  31,
    16,  17,  18,  19,  20,  21,  22,  23,
     8,   9,  10,  11,  12,  13,  14,  15,
     0,   1,   2,   3,   4,   5,   6,   7
};

int PawnOpeningMap32[32] = {
     -15, -10,   0,   5,
     -15, -10,   0,   5,
     -15, -10,   0,  15,
     -15, -10,   0,  25,
     -15, -10,   0,  15,
     -15, -10,   0,   5,
     -15, -10,   0,   5,
     -15, -10,   0,   5,
};

int PawnEndgameMap32[32] = {
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,

};

int KnightOpeningMap32[32] = {
     -50, -40, -30, -25,
     -35, -25, -15, -10,
     -20, -10,   0,   5,
     -10,   0,  10,  15,
      -5,   5,  15,  20,
      -5,   5,  15,  20,
     -20, -10,   0,   5,
    -135, -25, -15, -10,
};

int KnightEndgameMap32[32] = {
     -40, -30, -20, -15,
     -30, -20, -10,  -5,
     -20, -10,   0,   5,
     -15,  -5,   5,  10,
     -15,  -5,   5,  10,
     -20, -10,   0,   5,
     -30, -20, -10,  -5,
     -40, -30, -20, -15,
};

int BishopOpeningMap32[32] = {
     -18, -18, -16, -14,
      -8,   0,  -2,   0,
      -6,  -2,   4,   2,
      -4,   0,   2,   8,
      -4,   0,   2,   8,
      -6,  -2,   4,   2,
      -8,   0,  -2,   0,
      -8,  -8,  -6,  -4,
};

int BishopEndgameMap32[32] = {
     -18, -12,  -9,  -6,
     -12,  -6,  -3,   0,
      -9,  -3,   0,   3,
      -6,   0,   3,   6,
      -6,   0,   3,   6,
      -9,  -3,   0,   3,
     -12,  -6,  -3,   0,
     -18, -12,  -9,  -6,
};

int RookOpeningMap32[32] = {
      -2,  -1,   0,   0,
      -2,  -1,   0,   0,
      -2,  -1,   0,   0,
      -2,  -1,   0,   0,
      -2,  -1,   0,   0,
      -2,  -1,   0,   0,
      -2,  -1,   0,   0,
      -2,  -1,   0,   0,
};

int RookEndgameMap32[32] = {
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0
};

int QueenOpeningMap32[32] = {
      -5,  -5,  -5,  -5,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0,
       0,   0,   0,   0
};

int QueenEndgameMap32[32] = {
     -24, -16, -12,  -8,
     -16,  -8,  -4,   0,
     -12,  -4,   0,   4,
      -8,   0,   4,   8,
      -8,   0,   4,   8,
     -12,  -4,   0,   4,
     -16,  -8,  -4,   0,
     -24, -16, -12,  -8,
};

int KingOpeningMap32[32] = {
      40,  50,  30,  10,
      30,  40,  20,   0,
      10,  20,   0, -20,
       0,  10, -10, -30,
     -10,   0, -20, -40,
     -20, -10, -30, -50,
     -30, -20, -40, -60,
     -40, -30, -50, -70,
};

int KingEndgameMap32[32] = {
     -72, -48, -36, -24,
     -48, -24, -12,   0,
     -36, -12,   0,  12,
     -24,   0,  12,  24,
     -24,   0,  12,  24,
     -36, -12,   0,  12,
     -48, -24, -12,   0,
     -72, -48, -36, -24,
};

/**
 * Fill the opening and endgame piece-square tables using
 * the arrays located in psqt.h. Mirror these values for
 * and negate them for black. The tables are always scoring
 * assuming that white is positive and black is negative. Set
 * INITALIZED_PSQT so that this is only done once.
 */
void initalizePSQT(){
    int i, j;
    
    for(i = 0; i < 32; i++){
        for(j = 0; j < 64; j++){
            PSQTopening[i][j] = 0;
            PSQTendgame[i][j] = 0;
        }
    }
    
    for(i = 0, j = 0; i < 64; i += 8, j += 4){
        
        // Opening Pawn
        PSQTopening[WhitePawn][i+0] = PawnValue + PSQT_MULTIPLIER * PawnOpeningMap32[j+0];
        PSQTopening[WhitePawn][i+7] = PawnValue + PSQT_MULTIPLIER * PawnOpeningMap32[j+0];
        PSQTopening[WhitePawn][i+1] = PawnValue + PSQT_MULTIPLIER * PawnOpeningMap32[j+1];
        PSQTopening[WhitePawn][i+6] = PawnValue + PSQT_MULTIPLIER * PawnOpeningMap32[j+1];
        PSQTopening[WhitePawn][i+2] = PawnValue + PSQT_MULTIPLIER * PawnOpeningMap32[j+2];
        PSQTopening[WhitePawn][i+5] = PawnValue + PSQT_MULTIPLIER * PawnOpeningMap32[j+2];
        PSQTopening[WhitePawn][i+3] = PawnValue + PSQT_MULTIPLIER * PawnOpeningMap32[j+3];
        PSQTopening[WhitePawn][i+4] = PawnValue + PSQT_MULTIPLIER * PawnOpeningMap32[j+3];

        // Endgame Pawn
        PSQTendgame[WhitePawn][i+0] = PawnValue + PSQT_MULTIPLIER * PawnEndgameMap32[j+0];
        PSQTendgame[WhitePawn][i+7] = PawnValue + PSQT_MULTIPLIER * PawnEndgameMap32[j+0];
        PSQTendgame[WhitePawn][i+1] = PawnValue + PSQT_MULTIPLIER * PawnEndgameMap32[j+1];
        PSQTendgame[WhitePawn][i+6] = PawnValue + PSQT_MULTIPLIER * PawnEndgameMap32[j+1];
        PSQTendgame[WhitePawn][i+2] = PawnValue + PSQT_MULTIPLIER * PawnEndgameMap32[j+2];
        PSQTendgame[WhitePawn][i+5] = PawnValue + PSQT_MULTIPLIER * PawnEndgameMap32[j+2];
        PSQTendgame[WhitePawn][i+3] = PawnValue + PSQT_MULTIPLIER * PawnEndgameMap32[j+3];
        PSQTendgame[WhitePawn][i+4] = PawnValue + PSQT_MULTIPLIER * PawnEndgameMap32[j+3];
        
        // Opening Knight
        PSQTopening[WhiteKnight][i+0] = KnightValue + PSQT_MULTIPLIER * KnightOpeningMap32[j+0];
        PSQTopening[WhiteKnight][i+7] = KnightValue + PSQT_MULTIPLIER * KnightOpeningMap32[j+0];
        PSQTopening[WhiteKnight][i+1] = KnightValue + PSQT_MULTIPLIER * KnightOpeningMap32[j+1];
        PSQTopening[WhiteKnight][i+6] = KnightValue + PSQT_MULTIPLIER * KnightOpeningMap32[j+1];
        PSQTopening[WhiteKnight][i+2] = KnightValue + PSQT_MULTIPLIER * KnightOpeningMap32[j+2];
        PSQTopening[WhiteKnight][i+5] = KnightValue + PSQT_MULTIPLIER * KnightOpeningMap32[j+2];
        PSQTopening[WhiteKnight][i+3] = KnightValue + PSQT_MULTIPLIER * KnightOpeningMap32[j+3];
        PSQTopening[WhiteKnight][i+4] = KnightValue + PSQT_MULTIPLIER * KnightOpeningMap32[j+3];

        // Ending Knight
        PSQTendgame[WhiteKnight][i+0] = KnightValue + PSQT_MULTIPLIER * KnightEndgameMap32[j+0];
        PSQTendgame[WhiteKnight][i+7] = KnightValue + PSQT_MULTIPLIER * KnightEndgameMap32[j+0];
        PSQTendgame[WhiteKnight][i+1] = KnightValue + PSQT_MULTIPLIER * KnightEndgameMap32[j+1];
        PSQTendgame[WhiteKnight][i+6] = KnightValue + PSQT_MULTIPLIER * KnightEndgameMap32[j+1];
        PSQTendgame[WhiteKnight][i+2] = KnightValue + PSQT_MULTIPLIER * KnightEndgameMap32[j+2];
        PSQTendgame[WhiteKnight][i+5] = KnightValue + PSQT_MULTIPLIER * KnightEndgameMap32[j+2];
        PSQTendgame[WhiteKnight][i+3] = KnightValue + PSQT_MULTIPLIER * KnightEndgameMap32[j+3];
        PSQTendgame[WhiteKnight][i+4] = KnightValue + PSQT_MULTIPLIER * KnightEndgameMap32[j+3];
        
        // Opening Bishop
        PSQTopening[WhiteBishop][i+0] = BishopValue + PSQT_MULTIPLIER * BishopOpeningMap32[j+0];
        PSQTopening[WhiteBishop][i+7] = BishopValue + PSQT_MULTIPLIER * BishopOpeningMap32[j+0];
        PSQTopening[WhiteBishop][i+1] = BishopValue + PSQT_MULTIPLIER * BishopOpeningMap32[j+1];
        PSQTopening[WhiteBishop][i+6] = BishopValue + PSQT_MULTIPLIER * BishopOpeningMap32[j+1];
        PSQTopening[WhiteBishop][i+2] = BishopValue + PSQT_MULTIPLIER * BishopOpeningMap32[j+2];
        PSQTopening[WhiteBishop][i+5] = BishopValue + PSQT_MULTIPLIER * BishopOpeningMap32[j+2];
        PSQTopening[WhiteBishop][i+3] = BishopValue + PSQT_MULTIPLIER * BishopOpeningMap32[j+3];
        PSQTopening[WhiteBishop][i+4] = BishopValue + PSQT_MULTIPLIER * BishopOpeningMap32[j+3];

        // Ending Bishop
        PSQTendgame[WhiteBishop][i+0] = BishopValue + PSQT_MULTIPLIER * BishopEndgameMap32[j+0];
        PSQTendgame[WhiteBishop][i+7] = BishopValue + PSQT_MULTIPLIER * BishopEndgameMap32[j+0];
        PSQTendgame[WhiteBishop][i+1] = BishopValue + PSQT_MULTIPLIER * BishopEndgameMap32[j+1];
        PSQTendgame[WhiteBishop][i+6] = BishopValue + PSQT_MULTIPLIER * BishopEndgameMap32[j+1];
        PSQTendgame[WhiteBishop][i+2] = BishopValue + PSQT_MULTIPLIER * BishopEndgameMap32[j+2];
        PSQTendgame[WhiteBishop][i+5] = BishopValue + PSQT_MULTIPLIER * BishopEndgameMap32[j+2];
        PSQTendgame[WhiteBishop][i+3] = BishopValue + PSQT_MULTIPLIER * BishopEndgameMap32[j+3];
        PSQTendgame[WhiteBishop][i+4] = BishopValue + PSQT_MULTIPLIER * BishopEndgameMap32[j+3];
        
        // Opening Rook
        PSQTopening[WhiteRook][i+0] = RookValue + PSQT_MULTIPLIER * RookOpeningMap32[j+0];
        PSQTopening[WhiteRook][i+7] = RookValue + PSQT_MULTIPLIER * RookOpeningMap32[j+0];
        PSQTopening[WhiteRook][i+1] = RookValue + PSQT_MULTIPLIER * RookOpeningMap32[j+1];
        PSQTopening[WhiteRook][i+6] = RookValue + PSQT_MULTIPLIER * RookOpeningMap32[j+1];
        PSQTopening[WhiteRook][i+2] = RookValue + PSQT_MULTIPLIER * RookOpeningMap32[j+2];
        PSQTopening[WhiteRook][i+5] = RookValue + PSQT_MULTIPLIER * RookOpeningMap32[j+2];
        PSQTopening[WhiteRook][i+3] = RookValue + PSQT_MULTIPLIER * RookOpeningMap32[j+3];
        PSQTopening[WhiteRook][i+4] = RookValue + PSQT_MULTIPLIER * RookOpeningMap32[j+3];

        // Ending Rook
        PSQTendgame[WhiteRook][i+0] = RookValue + PSQT_MULTIPLIER * RookEndgameMap32[j+0];
        PSQTendgame[WhiteRook][i+7] = RookValue + PSQT_MULTIPLIER * RookEndgameMap32[j+0];
        PSQTendgame[WhiteRook][i+1] = RookValue + PSQT_MULTIPLIER * RookEndgameMap32[j+1];
        PSQTendgame[WhiteRook][i+6] = RookValue + PSQT_MULTIPLIER * RookEndgameMap32[j+1];
        PSQTendgame[WhiteRook][i+2] = RookValue + PSQT_MULTIPLIER * RookEndgameMap32[j+2];
        PSQTendgame[WhiteRook][i+5] = RookValue + PSQT_MULTIPLIER * RookEndgameMap32[j+2];
        PSQTendgame[WhiteRook][i+3] = RookValue + PSQT_MULTIPLIER * RookEndgameMap32[j+3];
        PSQTendgame[WhiteRook][i+4] = RookValue + PSQT_MULTIPLIER * RookEndgameMap32[j+3];
        
        // Opening Queen
        PSQTopening[WhiteQueen][i+0] = QueenValue + PSQT_MULTIPLIER * QueenOpeningMap32[j+0];
        PSQTopening[WhiteQueen][i+7] = QueenValue + PSQT_MULTIPLIER * QueenOpeningMap32[j+0];
        PSQTopening[WhiteQueen][i+1] = QueenValue + PSQT_MULTIPLIER * QueenOpeningMap32[j+1];
        PSQTopening[WhiteQueen][i+6] = QueenValue + PSQT_MULTIPLIER * QueenOpeningMap32[j+1];
        PSQTopening[WhiteQueen][i+2] = QueenValue + PSQT_MULTIPLIER * QueenOpeningMap32[j+2];
        PSQTopening[WhiteQueen][i+5] = QueenValue + PSQT_MULTIPLIER * QueenOpeningMap32[j+2];
        PSQTopening[WhiteQueen][i+3] = QueenValue + PSQT_MULTIPLIER * QueenOpeningMap32[j+3];
        PSQTopening[WhiteQueen][i+4] = QueenValue + PSQT_MULTIPLIER * QueenOpeningMap32[j+3];
        
        // Ending Queen
        PSQTendgame[WhiteQueen][i+0] = QueenValue + PSQT_MULTIPLIER * QueenEndgameMap32[j+0];
        PSQTendgame[WhiteQueen][i+7] = QueenValue + PSQT_MULTIPLIER * QueenEndgameMap32[j+0];
        PSQTendgame[WhiteQueen][i+1] = QueenValue + PSQT_MULTIPLIER * QueenEndgameMap32[j+1];
        PSQTendgame[WhiteQueen][i+6] = QueenValue + PSQT_MULTIPLIER * QueenEndgameMap32[j+1];
        PSQTendgame[WhiteQueen][i+2] = QueenValue + PSQT_MULTIPLIER * QueenEndgameMap32[j+2];
        PSQTendgame[WhiteQueen][i+5] = QueenValue + PSQT_MULTIPLIER * QueenEndgameMap32[j+2];
        PSQTendgame[WhiteQueen][i+3] = QueenValue + PSQT_MULTIPLIER * QueenEndgameMap32[j+3];
        PSQTendgame[WhiteQueen][i+4] = QueenValue + PSQT_MULTIPLIER * QueenEndgameMap32[j+3];
        
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
    
    INITALIZED_PSQT = 1;
}