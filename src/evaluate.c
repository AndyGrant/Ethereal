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

#include <stdint.h>
#include <stdlib.h>

#include "attacks.h"
#include "bitboards.h"
#include "board.h"
#include "evaluate.h"
#include "masks.h"
#include "transposition.h"
#include "types.h"

EvalTrace T, EmptyTrace;
int PSQT[32][SQUARE_NB];

#define S(mg, eg) (MakeScore((mg), (eg)))

/* Material Value Evaluation Terms */

const int PawnValue   = S( 105, 118);
const int KnightValue = S( 449, 410);
const int BishopValue = S( 473, 423);
const int RookValue   = S( 654, 684);
const int QueenValue  = S(1295,1380);
const int KingValue   = S(   0,   0);

/* Piece Square Evaluation Terms */

const int PawnPSQT32[32] = {
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
    S( -19,   9), S(   6,   4), S( -11,   7), S(  -6,  -1),
    S( -21,   4), S( -11,   3), S(  -8,  -5), S(  -2, -13),
    S( -16,  12), S( -10,  11), S(  14, -13), S(  12, -24),
    S(  -4,  16), S(   4,  11), S(   0,  -2), S(  14, -21),
    S(  -4,  32), S(   1,  30), S(  10,  19), S(  38,  -8),
    S( -17, -40), S( -65,  -9), S(   3, -23), S(  40, -37),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

const int KnightPSQT32[32] = {
    S( -50, -26), S(  -8, -40), S( -16, -29), S(  -1, -19),
    S(  -6, -22), S(   3, -13), S(   1, -31), S(  11, -20),
    S(   3, -25), S(  20, -25), S(  14, -18), S(  25,  -5),
    S(  15,   4), S(  23,   6), S(  29,  17), S(  30,  22),
    S(  24,  17), S(  26,  11), S(  39,  26), S(  30,  39),
    S( -14,  15), S(   6,  13), S(  32,  27), S(  33,  29),
    S(   7, -11), S(  -5,   2), S(  36, -20), S(  43,   0),
    S(-168, -17), S( -81,  -2), S(-110,  19), S( -30,   1),
};

const int BishopPSQT32[32] = {
    S(   8, -24), S(  13, -18), S( -10,  -6), S(  10, -10), 
    S(  30, -33), S(  12, -36), S(  20, -19), S(  12, -11), 
    S(  16, -12), S(  29, -15), S(   5, -17), S(  21,  -5), 
    S(  15,  -8), S(  19,  -3), S(  17,   3), S(  23,   6), 
    S( -11,  12), S(  19,   4), S(   6,  12), S(  13,  17), 
    S(   1,   7), S(  -1,  15), S(   0,   4), S(  18,  11), 
    S( -45,  16), S( -53,   4), S(  -7,   6), S( -21,  10), 
    S( -50,  -5), S( -47,   9), S( -87,  16), S( -90,  26), 
};

const int RookPSQT32[32] = {
    S( -10, -28), S( -14, -19), S(  -2, -23), S(   7, -29),
    S( -53, -13), S( -13, -30), S(  -9, -30), S(   1, -33),
    S( -28, -13), S(  -7, -11), S( -16, -15), S(  -1, -23),
    S( -16,  -1), S(  -7,   6), S(  -6,   2), S(   6,  -4),
    S(  -1,   8), S(  14,   5), S(  24,   1), S(  38,  -3),
    S( -11,  18), S(  27,   5), S(   3,  17), S(  35,   1),
    S(   2,   7), S( -17,  16), S(   8,   6), S(  22,   7),
    S(  35,  22), S(  25,  24), S(   6,  29), S(  16,  25),
};

const int QueenPSQT32[32] = {
    S(  23, -52), S(   4, -39), S(  11, -49), S(  20, -41),
    S(  15, -38), S(  28, -56), S(  30, -72), S(  20, -25),
    S(  12, -22), S(  27, -19), S(   9,   5), S(  10,   0),
    S(  13,  -3), S(  15,  17), S(   1,  21), S( -13,  63),
    S(  -6,  18), S(  -5,  42), S( -14,  22), S( -29,  72),
    S( -23,  28), S( -14,  19), S( -23,  23), S( -12,  26),
    S(  -5,  27), S( -60,  64), S(  -9,  16), S( -41,  53),
    S(  -7,  17), S(  18,   9), S(   9,   9), S(  -6,  20),
};

const int KingPSQT32[32] = {
    S(  41, -80), S(  41, -51), S( -11, -13), S( -26, -22),
    S(  30, -30), S(  -3, -20), S( -36,   6), S( -50,   8),
    S(   8, -31), S(  17, -27), S(  17,  -3), S(  -9,  12),
    S(   2, -36), S(  83, -37), S(  41,   2), S(  -6,  22),
    S(   4, -20), S(  95, -31), S(  46,  10), S(   2,  19),
    S(  46, -22), S( 120, -19), S(  94,   6), S(  38,   5),
    S(   6, -42), S(  47,  -5), S(  32,  10), S(   9,   2),
    S(   9, -98), S(  75, -51), S( -19, -11), S( -18, -10),
};

/* Pawn Evaluation Terms */

const int PawnCandidatePasser[2][RANK_NB] = {
   {S(   0,   0), S( -27, -10), S( -11,   9), S( -15,  28),
    S(  -1,  60), S(  44,  66), S(   0,   0), S(   0,   0)},
   {S(   0,   0), S( -14,  17), S(  -5,  20), S(   4,  43),
    S(  17,  82), S(  32,  54), S(   0,   0), S(   0,   0)},
};

const int PawnIsolated = S(  -7, -11);

const int PawnStacked[2] = { S(  -9, -14), S(  -9,  -9) };

const int PawnBackwards[2] = { S(   7,   0), S(  -7, -19) };

const int PawnConnected32[32] = {
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
    S(  -2,  -8), S(  11,   1), S(   3,   1), S(   5,  16),
    S(  14,  -1), S(  29,  -2), S(  21,   7), S(  24,  15),
    S(   9,   0), S(  23,   4), S(  10,  11), S(  15,  20),
    S(  13,   8), S(  22,  14), S(  27,  21), S(  32,  20),
    S(  57,  25), S(  51,  48), S(  68,  54), S(  84,  58),
    S( 112,   1), S( 204,  11), S( 228,  31), S( 240,  51),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

/* Knight Evaluation Terms */

const int KnightOutpost[2][2] = {
    { S(   7, -26), S(  31,  -4) },
    { S(   4, -26), S(  15,  -4) },
};

const int KnightBehindPawn = S(   4,  19);

const int KnightMobility[9] = {
    S( -74,-104), S( -31, -96), S( -16, -41), S(  -5, -16),
    S(   6,  -8), S(  11,   8), S(  19,  11), S(  28,  11),
    S(  40,  -3),
};

/* Bishop Evaluation Terms */

const int BishopPair = S(  22,  69);

const int BishopRammedPawns = S( -10, -15);

const int BishopOutpost[2][2] = {
    { S(  10, -12), S(  40,   0) },
    { S(   5, -12), S(  20,   0) },
};

const int BishopBehindPawn = S(   3,  18);

const int BishopLongDiagonal = S(  20,  11);

const int BishopMobility[14] = {
    S( -65,-147), S( -30, -95), S( -11, -56), S(  -1, -30),
    S(   9, -18), S(  17,  -4), S(  20,   6), S(  21,  11),
    S(  20,  17), S(  24,  18), S(  23,  19), S(  42,   8),
    S(  42,  18), S(  71, -14),
};

/* Rook Evaluation Terms */

const int RookFile[2] = { S(  15,   4), S(  35,   3) };

const int RookOnSeventh = S(  -2,  26);

const int RookMobility[15] = {
    S(-148,-113), S( -52,-113), S( -15, -61), S(  -7, -21),
    S(  -7,  -1), S(  -8,  14), S(  -7,  24), S(  -1,  27),
    S(   6,  30), S(  10,  34), S(  13,  40), S(  18,  43),
    S(  19,  47), S(  30,  39), S(  80,   4),
};

/* Queen Evaluation Terms */

const int QueenMobility[28] = {
    S( -61,-263), S(-210,-387), S( -58,-201), S( -16,-192),
    S(  -4,-139), S(   0, -84), S(   5, -52), S(   5, -23),
    S(   9, -16), S(  10,   6), S(  13,  13), S(  14,  28),
    S(  16,  24), S(  17,  33), S(  15,  37), S(  12,  41),
    S(  11,  44), S(   3,  45), S(   4,  42), S(  -1,  37),
    S(   7,  16), S(  22,  -6), S(  27, -34), S(  30, -51),
    S(  12, -68), S(  24, -95), S( -56, -39), S( -31, -61),
};

/* King Evaluation Terms */

const int KingDefenders[12] = {
    S( -26,   0), S(  -7,  -3), S(   1,   2), S(   8,   5),
    S(  17,   6), S(  27,   4), S(  31,  -2), S(  13,   0),
    S(  12,   6), S(  12,   6), S(  12,   6), S(  12,   6),
};

const int KingPawnFileProximity[FILE_NB]  = {
    S(  27,  19), S(  15,  15), S(   3,  10), S( -13, -12),
    S( -15, -40), S( -14, -56), S( -14, -65), S( -11, -70),
};

const int KingShelter[2][FILE_NB][RANK_NB] = {
  {{S( -11,   3), S(  15, -26), S(  20,  -9), S(  12,   4),
    S(   6,   4), S(   1,   2), S(  -3, -33), S( -49,  18)},
   {S(  17,  -8), S(  20, -18), S(   1,  -5), S( -14,   4),
    S( -30,  15), S( -70,  68), S(  92,  82), S( -25,   1)},
   {S(  35,  -4), S(  14,  -9), S( -28,   7), S( -11,  -8),
    S( -20,  -4), S( -11,   1), S(   0,  66), S( -12,  -2)},
   {S(   4,  11), S(  21, -11), S(   4, -11), S(  15, -22),
    S(  25, -36), S( -58,   5), S(-136,  52), S(   5,  -7)},
   {S( -15,   7), S(   4,  -4), S( -26,   1), S( -18,   5),
    S( -20,  -6), S( -41,  -1), S(  33, -17), S(  -6,  -2)},
   {S(  46, -18), S(  22, -17), S( -21,   2), S( -12, -18),
    S(   5, -24), S(  17, -21), S(  41, -30), S( -24,   1)},
   {S(  25, -13), S(  -1, -16), S( -23,  -2), S( -19,  -8),
    S( -30,  -7), S( -36,  32), S(   0,  44), S( -10,   1)},
   {S(  -9, -12), S(   6, -19), S(   7,   0), S(  -2,  11),
    S( -11,  15), S(  -9,  37), S(-190,  87), S( -17,  14)}},
  {{S(   0,   0), S( -11, -23), S(   4, -18), S( -40,  16),
    S( -22,   2), S(   3,  42), S(-167,  -8), S( -46,   7)},
   {S(   0,   0), S(  24, -21), S(   7,  -7), S( -18,  -1),
    S(  -1, -12), S(  26,  66), S(-184,  -3), S( -39,   3)},
   {S(   0,   0), S(  30, -11), S(  -2,  -7), S(   7, -17),
    S(  15,  -7), S( -87,  47), S( -84, -73), S( -20,  -5)},
   {S(   0,   0), S(  -3,   9), S(  -2,   0), S( -17,   2),
    S( -27,   1), S( -99,  31), S(   7, -41), S( -22,  -7)},
   {S(   0,   0), S(  12,   2), S(  11,  -5), S(  14, -11),
    S(  14, -26), S( -57,  15), S(-104, -61), S(  -1,  -7)},
   {S(   0,   0), S(   6,  -8), S( -20,   0), S( -27,  -6),
    S(  17, -24), S( -38,   3), S(  55,  38), S( -18,  -6)},
   {S(   0,   0), S(  22, -15), S(  11, -13), S(  -9,  -7),
    S( -27,   9), S(  -9,  15), S( -56, -49), S( -31,  11)},
   {S(   0,   0), S(  12, -38), S(  19, -27), S( -18,  -4),
    S( -17,  18), S(  -5,  20), S(-228, -55), S( -22,   1)}},
};

const int KingStorm[2][FILE_NB/2][RANK_NB] = {
  {{S(  -4,  28), S( 117,  -8), S( -25,  26), S( -19,   8),
    S( -14,   2), S(  -8,  -4), S( -17,   5), S( -22,  -2)},
   {S(  -3,  49), S(  57,  12), S( -19,  24), S(  -5,  11),
    S(  -4,   5), S(   5,  -4), S(  -1,   0), S( -11,   0)},
   {S(   8,  38), S(  17,  23), S( -23,  21), S( -11,   9),
    S(   3,   2), S(   7,   0), S(   9,  -7), S(   3,  -1)},
   {S(  -2,  25), S(  16,  21), S( -17,   9), S( -14,   2),
    S( -13,   2), S(   7, -11), S(   1,  -8), S( -13,   2)}},
  {{S(   0,   0), S( -15, -16), S( -17,  -1), S(  18, -17),
    S(   9,  -7), S(   3, -18), S(  -3,  -1), S(  17,  29)},
   {S(   0,   0), S( -16, -34), S(  -3,  -8), S(  35, -12),
    S(  -1,  -2), S(  13, -23), S(  -7, -10), S( -17,   2)},
   {S(   0,   0), S( -28, -49), S( -27,  -6), S(  11, -11),
    S(   3,  -2), S(  -8, -14), S( -13, -15), S( -11,   6)},
   {S(   0,   0), S(  -2, -18), S( -16, -18), S( -12,  -4),
    S(  -4,  -7), S(   4, -26), S(  72, -11), S(  13,  20)}},
};

/* King Safety Evaluation Terms */

const int KSAttackWeight[]  = { 0, 16, 6, 10, 8, 0 };
const int KSAttackValue     =   44;
const int KSWeakSquares     =   38;
const int KSFriendlyPawns   =  -22;
const int KSNoEnemyQueens   = -276;
const int KSSafeQueenCheck  =   95;
const int KSSafeRookCheck   =   94;
const int KSSafeBishopCheck =   51;
const int KSSafeKnightCheck =  123;
const int KSAdjustment      =  -18;

/* Passed Pawn Evaluation Terms */

const int PassedPawn[2][2][RANK_NB] = {
  {{S(   0,   0), S( -38,   3), S( -55,  21), S( -82,  27),
    S(  -6,  12), S(  70,  -5), S( 157,  56), S(   0,   0)},
   {S(   0,   0), S( -29,   7), S( -51,  24), S( -73,  30),
    S( -13,  31), S(  89,  32), S( 182, 101), S(   0,   0)}},
  {{S(   0,   0), S( -24,  16), S( -50,  19), S( -73,  33),
    S(  -3,  36), S(  89,  40), S( 263, 114), S(   0,   0)},
   {S(   0,   0), S( -29,  12), S( -45,  17), S( -67,  38),
    S(  -1,  52), S(  92, 117), S( 161, 275), S(   0,   0)}},
};

const int PassedFriendlyDistance[8] = {
    S(   0,   0), S(   0,   0), S(   3,  -4), S(   7, -10),
    S(   6, -14), S(  -8, -13), S( -15,  -9), S(   0,   0),
};

const int PassedEnemyDistance[8] = {
    S(   0,   0), S(   3,   0), S(   5,   2), S(   9,   9),
    S(   1,  21), S(   7,  30), S(  24,  28), S(   0,   0),
};

const int PassedSafePromotionPath = S( -29,  37);

const int PassedStacked[RANK_NB] = {
    S(   0,   0), S(   0,   0), S(   0,  -6), S(   0, -10),
    S(  -4, -12), S(  -8, -16), S(   0,   0), S(   0,   0),
};

/* Threat Evaluation Terms */

const int ThreatRestrictPiece        = S(  -3,  -1);
const int ThreatRestrictEmpty        = S(  -4,  -2);
const int ThreatWeakPawn             = S( -13, -26);
const int ThreatMinorAttackedByPawn  = S( -51, -53);
const int ThreatMinorAttackedByMinor = S( -26, -36);
const int ThreatMinorAttackedByMajor = S( -23, -44);
const int ThreatRookAttackedByLesser = S( -49, -19);
const int ThreatMinorAttackedByKing  = S( -16, -15);
const int ThreatRookAttackedByKing   = S( -13, -18);
const int ThreatQueenAttackedByOne   = S( -39, -29);
const int ThreatOverloadedPieces     = S(  -8, -13);
const int ThreatByPawnPush           = S(  15,  21);

/* Closedness Evaluation Terms */

const int ClosednessKnightAdjustment[9] = {
    S( -11, -11), S(  -9,   3), S(  -8,  11), S(  -3,  13),
    S(  -1,  18), S(   2,  16), S(   5,  13), S(  -6,  28),
    S(  -7,  16),
};

const int ClosednessRookAdjustment[9] = {
    S(  47,  -9), S(   7,  23), S(   3,  13), S(  -3,   4),
    S(  -7,   3), S(  -9,  -8), S( -13, -11), S( -21, -15),
    S( -26, -16),
};

/* Complexity Evaluation Terms */

const int ComplexityTotalPawns  = S(   0,   7);
const int ComplexityPawnFlanks  = S(   0,  49);
const int ComplexityPawnEndgame = S(   0,  34);
const int ComplexityAdjustment  = S(   0,-110);

/* General Evaluation Terms */

const int Tempo = 20;

#undef S

int evaluateBoard(Board *board, PKTable *pktable) {

    EvalInfo ei;
    int phase, factor, eval, pkeval;

    // Setup and perform all evaluations
    initEvalInfo(&ei, board, pktable);
    eval   = evaluatePieces(&ei, board);
    pkeval = ei.pkeval[WHITE] - ei.pkeval[BLACK];
    eval  += pkeval + board->psqtmat;
    eval  += evaluateClosedness(&ei, board);
    eval  += evaluateComplexity(&ei, board, eval);

    // Calculate the game phase based on remaining material (Fruit Method)
    phase = 24 - 4 * popcount(board->pieces[QUEEN ])
               - 2 * popcount(board->pieces[ROOK  ])
               - 1 * popcount(board->pieces[KNIGHT]
                             |board->pieces[BISHOP]);
    phase = (phase * 256 + 12) / 24;

    // Scale evaluation based on remaining material
    factor = evaluateScaleFactor(board, eval);

    // Compute the interpolated and scaled evaluation
    eval = (ScoreMG(eval) * (256 - phase)
         +  ScoreEG(eval) * phase * factor / SCALE_NORMAL) / 256;

    // Factor in the Tempo after interpolation and scaling, so that
    // in the search we can assume that if a null move is made, then
    // then `eval = last_eval + 2 * Tempo`
    eval += board->turn == WHITE ? Tempo : -Tempo;

    // Store a new Pawn King Entry if we did not have one
    if (ei.pkentry == NULL && pktable != NULL)
        storePKEntry(pktable, board->pkhash, ei.passedPawns, pkeval);

    // Return the evaluation relative to the side to move
    return board->turn == WHITE ? eval : -eval;
}

int evaluatePieces(EvalInfo *ei, Board *board) {

    int eval;

    eval  =   evaluatePawns(ei, board, WHITE)   - evaluatePawns(ei, board, BLACK);
    eval += evaluateKnights(ei, board, WHITE) - evaluateKnights(ei, board, BLACK);
    eval += evaluateBishops(ei, board, WHITE) - evaluateBishops(ei, board, BLACK);
    eval +=   evaluateRooks(ei, board, WHITE)   - evaluateRooks(ei, board, BLACK);
    eval +=  evaluateQueens(ei, board, WHITE)  - evaluateQueens(ei, board, BLACK);
    eval +=   evaluateKings(ei, board, WHITE)   - evaluateKings(ei, board, BLACK);
    eval +=  evaluatePassed(ei, board, WHITE)  - evaluatePassed(ei, board, BLACK);
    eval += evaluateThreats(ei, board, WHITE) - evaluateThreats(ei, board, BLACK);

    return eval;
}

int evaluatePawns(EvalInfo *ei, Board *board, int colour) {

    const int US = colour, THEM = !colour;
    const int Forward = (colour == WHITE) ? 8 : -8;

    int sq, flag, eval = 0, pkeval = 0;
    uint64_t pawns, myPawns, tempPawns, enemyPawns, attacks;

    // Store off pawn attacks for king safety and threat computations
    ei->attackedBy2[US]      = ei->pawnAttacks[US] & ei->attacked[US];
    ei->attacked[US]        |= ei->pawnAttacks[US];
    ei->attackedBy[US][PAWN] = ei->pawnAttacks[US];

    // Update King Safety calculations
    attacks = ei->pawnAttacks[US] & ei->kingAreas[THEM];
    ei->kingAttacksCount[US] += popcount(attacks);

    // Pawn hash holds the rest of the pawn evaluation
    if (ei->pkentry != NULL) return eval;

    pawns = board->pieces[PAWN];
    myPawns = tempPawns = pawns & board->colours[US];
    enemyPawns = pawns & board->colours[THEM];

    // Evaluate each pawn (but not for being passed)
    while (tempPawns) {

        // Pop off the next pawn
        sq = poplsb(&tempPawns);
        if (TRACE) T.PawnValue[US]++;
        if (TRACE) T.PawnPSQT32[relativeSquare32(US, sq)][US]++;

        uint64_t neighbors   = myPawns    & adjacentFilesMasks(fileOf(sq));
        uint64_t backup      = myPawns    & passedPawnMasks(THEM, sq);
        uint64_t stoppers    = enemyPawns & passedPawnMasks(US, sq);
        uint64_t threats     = enemyPawns & pawnAttacks(US, sq);
        uint64_t support     = myPawns    & pawnAttacks(THEM, sq);
        uint64_t pushThreats = enemyPawns & pawnAttacks(US, sq + Forward);
        uint64_t pushSupport = myPawns    & pawnAttacks(THEM, sq + Forward);
        uint64_t leftovers   = stoppers ^ threats ^ pushThreats;

        // Save passed pawn information for later evaluation
        if (!stoppers) setBit(&ei->passedPawns, sq);

        // Apply a bonus for pawns which will become passers by advancing a
        // square then exchanging our supporters with the remaining stoppers
        else if (!leftovers && popcount(pushSupport) >= popcount(pushThreats)) {
            flag = popcount(support) >= popcount(threats);
            pkeval += PawnCandidatePasser[flag][relativeRankOf(US, sq)];
            if (TRACE) T.PawnCandidatePasser[flag][relativeRankOf(US, sq)][US]++;
        }

        // Apply a penalty if the pawn is isolated. We consider pawns that
        // are able to capture another pawn to not be isolated, as they may
        // have the potential to deisolate by capturing, or be traded away
        if (!threats && !neighbors) {
            pkeval += PawnIsolated;
            if (TRACE) T.PawnIsolated[US]++;
        }

        // Apply a penalty if the pawn is stacked. We adjust the bonus for when
        // the pawn appears to be a candidate to unstack. This occurs when the
        // pawn is not passed but may capture or be recaptured by our own pawns,
        // and when the pawn may freely advance on a file and then be traded away
        if (several(Files[fileOf(sq)] & myPawns)) {
            flag = (stoppers && (threats || neighbors))
                || (stoppers & ~forwardFileMasks(US, sq));
            pkeval += PawnStacked[flag];
            if (TRACE) T.PawnStacked[flag][US]++;
        }

        // Apply a penalty if the pawn is backward. We follow the usual definition
        // of backwards, but also specify that the pawn is not both isolated and
        // backwards at the same time. We don't give backward pawns a connected bonus
        if (neighbors && pushThreats && !backup) {
            flag = !(Files[fileOf(sq)] & enemyPawns);
            pkeval += PawnBackwards[flag];
            if (TRACE) T.PawnBackwards[flag][US]++;
        }

        // Apply a bonus if the pawn is connected and not backwards. We consider a
        // pawn to be connected when there is a pawn lever or the pawn is supported
        else if (pawnConnectedMasks(US, sq) & myPawns) {
            pkeval += PawnConnected32[relativeSquare32(US, sq)];
            if (TRACE) T.PawnConnected32[relativeSquare32(US, sq)][US]++;
        }
    }

    ei->pkeval[US] = pkeval; // Save eval for the Pawn Hash

    return eval;
}

int evaluateKnights(EvalInfo *ei, Board *board, int colour) {

    const int US = colour, THEM = !colour;

    int sq, outside, defended, count, eval = 0;
    uint64_t attacks;

    uint64_t enemyPawns  = board->pieces[PAWN  ] & board->colours[THEM];
    uint64_t tempKnights = board->pieces[KNIGHT] & board->colours[US  ];

    ei->attackedBy[US][KNIGHT] = 0ull;

    // Evaluate each knight
    while (tempKnights) {

        // Pop off the next knight
        sq = poplsb(&tempKnights);
        if (TRACE) T.KnightValue[US]++;
        if (TRACE) T.KnightPSQT32[relativeSquare32(US, sq)][US]++;

        // Compute possible attacks and store off information for king safety
        attacks = knightAttacks(sq);
        ei->attackedBy2[US]        |= attacks & ei->attacked[US];
        ei->attacked[US]           |= attacks;
        ei->attackedBy[US][KNIGHT] |= attacks;

        // Apply a bonus if the knight is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the knight
        if (     testBit(outpostRanksMasks(US), sq)
            && !(outpostSquareMasks(US, sq) & enemyPawns)) {
            outside  = testBit(FILE_A | FILE_H, sq);
            defended = testBit(ei->pawnAttacks[US], sq);
            eval += KnightOutpost[outside][defended];
            if (TRACE) T.KnightOutpost[outside][defended][US]++;
        }

        // Apply a bonus if the knight is behind a pawn
        if (testBit(pawnAdvance(board->pieces[PAWN], 0ull, THEM), sq)) {
            eval += KnightBehindPawn;
            if (TRACE) T.KnightBehindPawn[US]++;
        }

        // Apply a bonus (or penalty) based on the mobility of the knight
        count = popcount(ei->mobilityAreas[US] & attacks);
        eval += KnightMobility[count];
        if (TRACE) T.KnightMobility[count][US]++;

        // Update King Safety calculations
        if ((attacks &= ei->kingAreas[THEM])) {
            ei->kingAttacksCount[US] += popcount(attacks);
            ei->kingAttackersCount[US] += 1;
            ei->kingAttackersWeight[US] += KSAttackWeight[KNIGHT];
        }
    }

    return eval;
}

int evaluateBishops(EvalInfo *ei, Board *board, int colour) {

    const int US = colour, THEM = !colour;

    int sq, outside, defended, count, eval = 0;
    uint64_t attacks;

    uint64_t enemyPawns  = board->pieces[PAWN  ] & board->colours[THEM];
    uint64_t tempBishops = board->pieces[BISHOP] & board->colours[US  ];

    ei->attackedBy[US][BISHOP] = 0ull;

    // Apply a bonus for having a pair of bishops
    if ((tempBishops & WHITE_SQUARES) && (tempBishops & BLACK_SQUARES)) {
        eval += BishopPair;
        if (TRACE) T.BishopPair[US]++;
    }

    // Evaluate each bishop
    while (tempBishops) {

        // Pop off the next Bishop
        sq = poplsb(&tempBishops);
        if (TRACE) T.BishopValue[US]++;
        if (TRACE) T.BishopPSQT32[relativeSquare32(US, sq)][US]++;

        // Compute possible attacks and store off information for king safety
        attacks = bishopAttacks(sq, ei->occupiedMinusBishops[US]);
        ei->attackedBy2[US]        |= attacks & ei->attacked[US];
        ei->attacked[US]           |= attacks;
        ei->attackedBy[US][BISHOP] |= attacks;

        // Apply a penalty for the bishop based on number of rammed pawns
        // of our own colour, which reside on the same shade of square as the bishop
        count = popcount(ei->rammedPawns[US] & squaresOfMatchingColour(sq));
        eval += count * BishopRammedPawns;
        if (TRACE) T.BishopRammedPawns[US] += count;

        // Apply a bonus if the bishop is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the bishop.
        if (     testBit(outpostRanksMasks(US), sq)
            && !(outpostSquareMasks(US, sq) & enemyPawns)) {
            outside  = testBit(FILE_A | FILE_H, sq);
            defended = testBit(ei->pawnAttacks[US], sq);
            eval += BishopOutpost[outside][defended];
            if (TRACE) T.BishopOutpost[outside][defended][US]++;
        }

        // Apply a bonus if the bishop is behind a pawn
        if (testBit(pawnAdvance(board->pieces[PAWN], 0ull, THEM), sq)) {
            eval += BishopBehindPawn;
            if (TRACE) T.BishopBehindPawn[US]++;
        }

        // Apply a bonus if the bishop is on a long diagonal
        // and if it controls both central squares
        if (   testBit(LONG_DIAGONALS & ~CENTER_SQUARES, sq)
            && popcount(bishopAttacks(sq, board->pieces[PAWN]) & CENTER_SQUARES) == 2) {
            eval += BishopLongDiagonal;
            if (TRACE) T.BishopLongDiagonal[US]++;
        }

        // Apply a bonus (or penalty) based on the mobility of the bishop
        count = popcount(ei->mobilityAreas[US] & attacks);
        eval += BishopMobility[count];
        if (TRACE) T.BishopMobility[count][US]++;

        // Update King Safety calculations
        if ((attacks &= ei->kingAreas[THEM])) {
            ei->kingAttacksCount[US] += popcount(attacks);
            ei->kingAttackersCount[US] += 1;
            ei->kingAttackersWeight[US] += KSAttackWeight[BISHOP];
        }
    }

    return eval;
}

int evaluateRooks(EvalInfo *ei, Board *board, int colour) {

    const int US = colour, THEM = !colour;

    int sq, open, count, eval = 0;
    uint64_t attacks;

    uint64_t myPawns    = board->pieces[PAWN] & board->colours[  US];
    uint64_t enemyPawns = board->pieces[PAWN] & board->colours[THEM];
    uint64_t tempRooks  = board->pieces[ROOK] & board->colours[  US];

    ei->attackedBy[US][ROOK] = 0ull;

    // Evaluate each rook
    while (tempRooks) {

        // Pop off the next rook
        sq = poplsb(&tempRooks);
        if (TRACE) T.RookValue[US]++;
        if (TRACE) T.RookPSQT32[relativeSquare32(US, sq)][US]++;

        // Compute possible attacks and store off information for king safety
        attacks = rookAttacks(sq, ei->occupiedMinusRooks[US]);
        ei->attackedBy2[US]      |= attacks & ei->attacked[US];
        ei->attacked[US]         |= attacks;
        ei->attackedBy[US][ROOK] |= attacks;

        // Rook is on a semi-open file if there are no pawns of the rook's
        // colour on the file. If there are no pawns at all, it is an open file
        if (!(myPawns & Files[fileOf(sq)])) {
            open = !(enemyPawns & Files[fileOf(sq)]);
            eval += RookFile[open];
            if (TRACE) T.RookFile[open][US]++;
        }

        // Rook gains a bonus for being located on seventh rank relative to its
        // colour so long as the enemy king is on the last two ranks of the board
        if (   relativeRankOf(US, sq) == 6
            && relativeRankOf(US, ei->kingSquare[THEM]) >= 6) {
            eval += RookOnSeventh;
            if (TRACE) T.RookOnSeventh[US]++;
        }

        // Apply a bonus (or penalty) based on the mobility of the rook
        count = popcount(ei->mobilityAreas[US] & attacks);
        eval += RookMobility[count];
        if (TRACE) T.RookMobility[count][US]++;

        // Update King Safety calculations
        if ((attacks &= ei->kingAreas[THEM])) {
            ei->kingAttacksCount[US] += popcount(attacks);
            ei->kingAttackersCount[US] += 1;
            ei->kingAttackersWeight[US] += KSAttackWeight[ROOK];
        }
    }

    return eval;
}

int evaluateQueens(EvalInfo *ei, Board *board, int colour) {

    const int US = colour, THEM = !colour;

    int sq, count, eval = 0;
    uint64_t tempQueens, attacks;

    tempQueens = board->pieces[QUEEN] & board->colours[US];

    ei->attackedBy[US][QUEEN] = 0ull;

    // Evaluate each queen
    while (tempQueens) {

        // Pop off the next queen
        sq = poplsb(&tempQueens);
        if (TRACE) T.QueenValue[US]++;
        if (TRACE) T.QueenPSQT32[relativeSquare32(US, sq)][US]++;

        // Compute possible attacks and store off information for king safety
        attacks = queenAttacks(sq, board->colours[WHITE] | board->colours[BLACK]);
        ei->attackedBy2[US]       |= attacks & ei->attacked[US];
        ei->attacked[US]          |= attacks;
        ei->attackedBy[US][QUEEN] |= attacks;

        // Apply a bonus (or penalty) based on the mobility of the queen
        count = popcount(ei->mobilityAreas[US] & attacks);
        eval += QueenMobility[count];
        if (TRACE) T.QueenMobility[count][US]++;

        // Update King Safety calculations
        if ((attacks &= ei->kingAreas[THEM])) {
            ei->kingAttacksCount[US] += popcount(attacks);
            ei->kingAttackersCount[US] += 1;
            ei->kingAttackersWeight[US] += KSAttackWeight[QUEEN];
        }
    }

    return eval;
}

int evaluateKings(EvalInfo *ei, Board *board, int colour) {

    const int US = colour, THEM = !colour;

    int count, dist, blocked, eval = 0;

    uint64_t myPawns     = board->pieces[PAWN ] & board->colours[  US];
    uint64_t enemyPawns  = board->pieces[PAWN ] & board->colours[THEM];
    uint64_t enemyQueens = board->pieces[QUEEN] & board->colours[THEM];

    uint64_t defenders  = (board->pieces[PAWN  ] & board->colours[US])
                        | (board->pieces[KNIGHT] & board->colours[US])
                        | (board->pieces[BISHOP] & board->colours[US]);

    int kingSq = ei->kingSquare[US];
    if (TRACE) T.KingValue[US]++;
    if (TRACE) T.KingPSQT32[relativeSquare32(US, kingSq)][US]++;

    // Bonus for our pawns and minors sitting within our king area
    count = popcount(defenders & ei->kingAreas[US]);
    eval += KingDefenders[count];
    if (TRACE) T.KingDefenders[count][US]++;

    // Perform King Safety when we have two attackers, or
    // one attacker with a potential for a Queen attacker
    if (ei->kingAttackersCount[THEM] > 1 - popcount(enemyQueens)) {

        // Weak squares are attacked by the enemy, defended no more
        // than once and only defended by our Queens or our King
        uint64_t weak =   ei->attacked[THEM]
                      &  ~ei->attackedBy2[US]
                      & (~ei->attacked[US] | ei->attackedBy[US][QUEEN] | ei->attackedBy[US][KING]);

        // Usually the King Area is 9 squares. Scale are attack counts to account for
        // when the king is in an open area and expects more attacks, or the opposite
        float scaledAttackCounts = 9.0 * ei->kingAttacksCount[THEM] / popcount(ei->kingAreas[US]);

        // Safe target squares are defended or are weak and attacked by two.
        // We exclude squares containing pieces which we cannot capture.
        uint64_t safe =  ~board->colours[THEM]
                      & (~ei->attacked[US] | (weak & ei->attackedBy2[THEM]));

        // Find square and piece combinations which would check our King
        uint64_t occupied      = board->colours[WHITE] | board->colours[BLACK];
        uint64_t knightThreats = knightAttacks(kingSq);
        uint64_t bishopThreats = bishopAttacks(kingSq, occupied);
        uint64_t rookThreats   = rookAttacks(kingSq, occupied);
        uint64_t queenThreats  = bishopThreats | rookThreats;

        // Identify if there are pieces which can move to the checking squares safely.
        // We consider forking a Queen to be a safe check, even with our own Queen.
        uint64_t knightChecks = knightThreats & safe & ei->attackedBy[THEM][KNIGHT];
        uint64_t bishopChecks = bishopThreats & safe & ei->attackedBy[THEM][BISHOP];
        uint64_t rookChecks   = rookThreats   & safe & ei->attackedBy[THEM][ROOK  ];
        uint64_t queenChecks  = queenThreats  & safe & ei->attackedBy[THEM][QUEEN ];

        count  = ei->kingAttackersCount[THEM] * ei->kingAttackersWeight[THEM];

        count += KSAttackValue     * scaledAttackCounts
               + KSWeakSquares     * popcount(weak & ei->kingAreas[US])
               + KSFriendlyPawns   * popcount(myPawns & ei->kingAreas[US] & ~weak)
               + KSNoEnemyQueens   * !enemyQueens
               + KSSafeQueenCheck  * popcount(queenChecks)
               + KSSafeRookCheck   * popcount(rookChecks)
               + KSSafeBishopCheck * popcount(bishopChecks)
               + KSSafeKnightCheck * popcount(knightChecks)
               + KSAdjustment;

        // Convert safety to an MG and EG score, if we are unsafe
        if (count > 0) eval -= MakeScore(count * count / 720, count / 20);
    }

    // Everything else is stored in the Pawn King Table
    if (ei->pkentry != NULL) return eval;

    // Evaluate based on the number of files between our King and the nearest
    // file-wise pawn. If there is no pawn, kingPawnFileDistance() returns the
    // same distance for both sides causing this evaluation term to be neutral
    dist = kingPawnFileDistance(board->pieces[PAWN], kingSq);
    ei->pkeval[US] += KingPawnFileProximity[dist];
    if (TRACE) T.KingPawnFileProximity[dist][US]++;

    // Evaluate King Shelter & King Storm threat by looking at the file of our King,
    // as well as the adjacent files. When looking at pawn distances, we will use a
    // distance of 7 to denote a missing pawn, since distance 7 is not possible otherwise.
    for (int file = MAX(0, fileOf(kingSq) - 1); file <= MIN(FILE_NB - 1, fileOf(kingSq) + 1); file++) {

        // Find closest friendly pawn at or above our King on a given file
        uint64_t ours = myPawns & Files[file] & forwardRanksMasks(US, rankOf(kingSq));
        int ourDist = !ours ? 7 : abs(rankOf(kingSq) - rankOf(backmost(US, ours)));

        // Find closest enemy pawn at or above our King on a given file
        uint64_t theirs = enemyPawns & Files[file] & forwardRanksMasks(US, rankOf(kingSq));
        int theirDist = !theirs ? 7 : abs(rankOf(kingSq) - rankOf(backmost(US, theirs)));

        // Evaluate King Shelter using pawn distance. Use separate evaluation
        // depending on the file, and if we are looking at the King's file
        ei->pkeval[US] += KingShelter[file == fileOf(kingSq)][file][ourDist];
        if (TRACE) T.KingShelter[file == fileOf(kingSq)][file][ourDist][US]++;

        // Evaluate King Storm using enemy pawn distance. Use a separate evaluation
        // depending on the file, and if the opponent's pawn is blocked by our own
        blocked = (ourDist != 7 && (ourDist == theirDist - 1));
        ei->pkeval[US] += KingStorm[blocked][mirrorFile(file)][theirDist];
        if (TRACE) T.KingStorm[blocked][mirrorFile(file)][theirDist][US]++;
    }

    return eval;
}

int evaluatePassed(EvalInfo *ei, Board *board, int colour) {

    const int US = colour, THEM = !colour;

    int sq, rank, dist, flag, canAdvance, safeAdvance, eval = 0;

    uint64_t bitboard;
    uint64_t myPassers = board->colours[US] & ei->passedPawns;
    uint64_t occupied  = board->colours[WHITE] | board->colours[BLACK];
    uint64_t tempPawns = myPassers;

    // Evaluate each passed pawn
    while (tempPawns) {

        // Pop off the next passed Pawn
        sq = poplsb(&tempPawns);
        rank = relativeRankOf(US, sq);
        bitboard = pawnAdvance(1ull << sq, 0ull, US);

        // Evaluate based on rank, ability to advance, and safety
        canAdvance = !(bitboard & occupied);
        safeAdvance = !(bitboard & ei->attacked[THEM]);
        eval += PassedPawn[canAdvance][safeAdvance][rank];
        if (TRACE) T.PassedPawn[canAdvance][safeAdvance][rank][US]++;

        // Evaluate based on distance from our king
        dist = distanceBetween(sq, ei->kingSquare[US]);
        eval += dist * PassedFriendlyDistance[rank];
        if (TRACE) T.PassedFriendlyDistance[rank][US] += dist;

        // Evaluate based on distance from their king
        dist = distanceBetween(sq, ei->kingSquare[THEM]);
        eval += dist * PassedEnemyDistance[rank];
        if (TRACE) T.PassedEnemyDistance[rank][US] += dist;

        // Apply a bonus when the path to promoting is uncontested
        bitboard = forwardRanksMasks(US, rankOf(sq)) & Files[fileOf(sq)];
        flag = !(bitboard & (board->colours[THEM] | ei->attacked[THEM]));
        eval += flag * PassedSafePromotionPath;
        if (TRACE) T.PassedSafePromotionPath[US] += flag;

        // Apply an extra penalty to the foremost stacked passer
        if(several(forwardFileMasks(THEM, sq) & myPassers)) {
            eval += PassedStacked[rank];
            if (TRACE) T.PassedStacked[rank][US]++;
        }
    }

    return eval;
}

int evaluateThreats(EvalInfo *ei, Board *board, int colour) {

    const int US = colour, THEM = !colour;
    const uint64_t Rank3Rel = US == WHITE ? RANK_3 : RANK_6;

    int count, eval = 0;

    uint64_t friendly = board->colours[  US];
    uint64_t enemy    = board->colours[THEM];
    uint64_t occupied = friendly | enemy;

    uint64_t pawns   = friendly & board->pieces[PAWN  ];
    uint64_t knights = friendly & board->pieces[KNIGHT];
    uint64_t bishops = friendly & board->pieces[BISHOP];
    uint64_t rooks   = friendly & board->pieces[ROOK  ];
    uint64_t queens  = friendly & board->pieces[QUEEN ];

    uint64_t attacksByPawns  = ei->attackedBy[THEM][PAWN  ];
    uint64_t attacksByMinors = ei->attackedBy[THEM][KNIGHT] | ei->attackedBy[THEM][BISHOP];
    uint64_t attacksByMajors = ei->attackedBy[THEM][ROOK  ] | ei->attackedBy[THEM][QUEEN ];

    // Squares with more attackers, few defenders, and no pawn support
    uint64_t poorlyDefended = (ei->attacked[THEM] & ~ei->attacked[US])
                            | (ei->attackedBy2[THEM] & ~ei->attackedBy2[US] & ~ei->attackedBy[US][PAWN]);

    uint64_t uncontrolled =  ei->attacked[THEM] & ei->attacked[US] & poorlyDefended;

    uint64_t weakMinors = (knights | bishops) & poorlyDefended;

    // A friendly minor or major is overloaded if attacked and defended by exactly one
    uint64_t overloaded = (knights | bishops | rooks | queens)
                        & ei->attacked[  US] & ~ei->attackedBy2[  US]
                        & ei->attacked[THEM] & ~ei->attackedBy2[THEM];

    // Look for enemy non-pawn pieces which we may threaten with a pawn advance.
    // Don't consider pieces we already threaten, pawn moves which would be countered
    // by a pawn capture, and squares which are completely unprotected by our pieces.
    uint64_t pushThreat  = pawnAdvance(pawns, occupied, US);
    pushThreat |= pawnAdvance(pushThreat & ~attacksByPawns & Rank3Rel, occupied, US);
    pushThreat &= ~attacksByPawns & (ei->attacked[US] | ~ei->attacked[THEM]);
    pushThreat  = pawnAttackSpan(pushThreat, enemy & ~ei->attackedBy[US][PAWN], US);


    // Penalty for restricted piece moves
    count = popcount(uncontrolled & (friendly | enemy));
    eval += count * ThreatRestrictPiece;
    if (TRACE) T.ThreatRestrictPiece[US] += count;

    count = popcount(uncontrolled & ~friendly & ~enemy);
    eval += count * ThreatRestrictEmpty;
    if (TRACE) T.ThreatRestrictEmpty[US] += count;

    // Penalty for each of our poorly supported pawns
    count = popcount(pawns & ~attacksByPawns & poorlyDefended);
    eval += count * ThreatWeakPawn;
    if (TRACE) T.ThreatWeakPawn[US] += count;

    // Penalty for pawn threats against our minors
    count = popcount((knights | bishops) & attacksByPawns);
    eval += count * ThreatMinorAttackedByPawn;
    if (TRACE) T.ThreatMinorAttackedByPawn[US] += count;

    // Penalty for any minor threat against minor pieces
    count = popcount((knights | bishops) & attacksByMinors);
    eval += count * ThreatMinorAttackedByMinor;
    if (TRACE) T.ThreatMinorAttackedByMinor[US] += count;

    // Penalty for all major threats against poorly supported minors
    count = popcount(weakMinors & attacksByMajors);
    eval += count * ThreatMinorAttackedByMajor;
    if (TRACE) T.ThreatMinorAttackedByMajor[US] += count;

    // Penalty for pawn and minor threats against our rooks
    count = popcount(rooks & (attacksByPawns | attacksByMinors));
    eval += count * ThreatRookAttackedByLesser;
    if (TRACE) T.ThreatRookAttackedByLesser[US] += count;

    // Penalty for king threats against our poorly defended minors
    count = popcount(weakMinors & ei->attackedBy[THEM][KING]);
    eval += count * ThreatMinorAttackedByKing;
    if (TRACE) T.ThreatMinorAttackedByKing[US] += count;

    // Penalty for king threats against our poorly defended rooks
    count = popcount(rooks & poorlyDefended & ei->attackedBy[THEM][KING]);
    eval += count * ThreatRookAttackedByKing;
    if (TRACE) T.ThreatRookAttackedByKing[US] += count;

    // Penalty for any threat against our queens
    count = popcount(queens & ei->attacked[THEM]);
    eval += count * ThreatQueenAttackedByOne;
    if (TRACE) T.ThreatQueenAttackedByOne[US] += count;

    // Penalty for any overloaded minors or majors
    count = popcount(overloaded);
    eval += count * ThreatOverloadedPieces;
    if (TRACE) T.ThreatOverloadedPieces[US] += count;

    // Bonus for giving threats by safe pawn pushes
    count = popcount(pushThreat);
    eval += count * ThreatByPawnPush;
    if (TRACE) T.ThreatByPawnPush[colour] += count;

    return eval;
}

int evaluateClosedness(EvalInfo *ei, Board *board) {

    int closedness, count, eval = 0;

    uint64_t white = board->colours[WHITE];
    uint64_t black = board->colours[BLACK];

    uint64_t knights = board->pieces[KNIGHT];
    uint64_t rooks   = board->pieces[ROOK  ];

    // Compute Closedness factor for this position
    closedness = 1 * popcount(board->pieces[PAWN])
               + 3 * popcount(ei->rammedPawns[WHITE])
               - 4 * openFileCount(board->pieces[PAWN]);
    closedness = MAX(0, MIN(8, closedness / 3));

    // Evaluate Knights based on how Closed the position is
    count = popcount(white & knights) - popcount(black & knights);
    eval += count * ClosednessKnightAdjustment[closedness];
    if (TRACE) T.ClosednessKnightAdjustment[closedness][WHITE] += count;

    // Evaluate Rooks based on how Closed the position is
    count = popcount(white & rooks) - popcount(black & rooks);
    eval += count * ClosednessRookAdjustment[closedness];
    if (TRACE) T.ClosednessRookAdjustment[closedness][WHITE] += count;

    return eval;
}

int evaluateComplexity(EvalInfo *ei, Board *board, int eval) {

    // Adjust endgame evaluation based on features related to how
    // likely the stronger side is to convert the position.
    // More often than not, this is a penalty for drawish positions.

    (void) ei; // Silence compiler warning

    int complexity;
    int eg = ScoreEG(eval);
    int sign = (eg > 0) - (eg < 0);

    int pawnsOnBothFlanks = (board->pieces[PAWN] & LEFT_FLANK )
                         && (board->pieces[PAWN] & RIGHT_FLANK);

    uint64_t knights = board->pieces[KNIGHT];
    uint64_t bishops = board->pieces[BISHOP];
    uint64_t rooks   = board->pieces[ROOK  ];
    uint64_t queens  = board->pieces[QUEEN ];

    // Compute the initiative bonus or malus for the attacking side
    complexity =  ComplexityTotalPawns  * popcount(board->pieces[PAWN])
               +  ComplexityPawnFlanks  * pawnsOnBothFlanks
               +  ComplexityPawnEndgame * !(knights | bishops | rooks | queens)
               +  ComplexityAdjustment;

    if (TRACE) T.ComplexityTotalPawns[WHITE]  += sign * popcount(board->pieces[PAWN]);
    if (TRACE) T.ComplexityPawnFlanks[WHITE]  += sign * pawnsOnBothFlanks;
    if (TRACE) T.ComplexityPawnEndgame[WHITE] += sign * !(knights | bishops | rooks | queens);
    if (TRACE) T.ComplexityAdjustment[WHITE]  += sign;

    // Avoid changing which side has the advantage
    int v = sign * MAX(ScoreEG(complexity), -abs(eg));

    return MakeScore(0, v);
}

int evaluateScaleFactor(Board *board, int eval) {

    // Scale endgames based on remaining material. Currently, we only
    // look for OCB endgames that include only one Knight or one Rook

    uint64_t white   = board->colours[WHITE];
    uint64_t black   = board->colours[BLACK];
    uint64_t knights = board->pieces[KNIGHT];
    uint64_t bishops = board->pieces[BISHOP];
    uint64_t rooks   = board->pieces[ROOK  ];
    uint64_t queens  = board->pieces[QUEEN ];

    if (   onlyOne(white & bishops)
        && onlyOne(black & bishops)
        && onlyOne(bishops & WHITE_SQUARES)) {

        if (!(knights | rooks | queens))
            return SCALE_OCB_BISHOPS_ONLY;

        if (   !(rooks | queens)
            &&  onlyOne(white & knights)
            &&  onlyOne(black & knights))
            return SCALE_OCB_ONE_KNIGHT;

        if (   !(knights | queens)
            && onlyOne(white & rooks)
            && onlyOne(black & rooks))
            return SCALE_OCB_ONE_ROOK;
    }

    int eg = ScoreEG(eval);

    // Lone minor vs king and pawns, never give the advantage to the side with the minor
    if ( (eg > 0) && popcount(white) == 2 && (white & (knights | bishops)))
        return SCALE_DRAW;
    else if ( (eg < 0) && popcount(black) == 2 && (black & (knights | bishops)))
        return SCALE_DRAW;

    return SCALE_NORMAL;
}

void initEvalInfo(EvalInfo *ei, Board *board, PKTable *pktable) {

    uint64_t white   = board->colours[WHITE];
    uint64_t black   = board->colours[BLACK];

    uint64_t pawns   = board->pieces[PAWN  ];
    uint64_t bishops = board->pieces[BISHOP] | board->pieces[QUEEN];
    uint64_t rooks   = board->pieces[ROOK  ] | board->pieces[QUEEN];
    uint64_t kings   = board->pieces[KING  ];

    // Save some general information about the pawn structure for later
    ei->pawnAttacks[WHITE]  = pawnAttackSpan(white & pawns, ~0ull, WHITE);
    ei->pawnAttacks[BLACK]  = pawnAttackSpan(black & pawns, ~0ull, BLACK);
    ei->rammedPawns[WHITE]  = pawnAdvance(black & pawns, ~(white & pawns), BLACK);
    ei->rammedPawns[BLACK]  = pawnAdvance(white & pawns, ~(black & pawns), WHITE);
    ei->blockedPawns[WHITE] = pawnAdvance(white | black, ~(white & pawns), BLACK);
    ei->blockedPawns[BLACK] = pawnAdvance(white | black, ~(black & pawns), WHITE);

    // Compute an area for evaluating our King's safety.
    // The definition of the King Area can be found in masks.c
    ei->kingSquare[WHITE] = getlsb(white & kings);
    ei->kingSquare[BLACK] = getlsb(black & kings);
    ei->kingAreas[WHITE] = kingAreaMasks(WHITE, ei->kingSquare[WHITE]);
    ei->kingAreas[BLACK] = kingAreaMasks(BLACK, ei->kingSquare[BLACK]);

    // Exclude squares attacked by our opponents, our blocked pawns, and our own King
    ei->mobilityAreas[WHITE] = ~(ei->pawnAttacks[BLACK] | (white & kings) | ei->blockedPawns[WHITE]);
    ei->mobilityAreas[BLACK] = ~(ei->pawnAttacks[WHITE] | (black & kings) | ei->blockedPawns[BLACK]);

    // Init part of the attack tables. By doing this step here, evaluatePawns()
    // can start by setting up the attackedBy2 table, since King attacks are resolved
    ei->attacked[WHITE] = ei->attackedBy[WHITE][KING] = kingAttacks(ei->kingSquare[WHITE]);
    ei->attacked[BLACK] = ei->attackedBy[BLACK][KING] = kingAttacks(ei->kingSquare[BLACK]);

    // For mobility, we allow bishops to attack through each other
    ei->occupiedMinusBishops[WHITE] = (white | black) ^ (white & bishops);
    ei->occupiedMinusBishops[BLACK] = (white | black) ^ (black & bishops);

    // For mobility, we allow rooks to attack through each other
    ei->occupiedMinusRooks[WHITE] = (white | black) ^ (white & rooks);
    ei->occupiedMinusRooks[BLACK] = (white | black) ^ (black & rooks);

    // Init all of the King Safety information
    ei->kingAttacksCount[WHITE]    = ei->kingAttacksCount[BLACK]    = 0;
    ei->kingAttackersCount[WHITE]  = ei->kingAttackersCount[BLACK]  = 0;
    ei->kingAttackersWeight[WHITE] = ei->kingAttackersWeight[BLACK] = 0;

    // Try to read a hashed Pawn King Eval. Otherwise, start from scratch
    ei->pkentry       =     pktable == NULL ? NULL : getPKEntry(pktable, board->pkhash);
    ei->passedPawns   = ei->pkentry == NULL ? 0ull : ei->pkentry->passed;
    ei->pkeval[WHITE] = ei->pkentry == NULL ? 0    : ei->pkentry->eval;
    ei->pkeval[BLACK] = ei->pkentry == NULL ? 0    : 0;
}

void initEval() {

    // Init a normalized 64-length PSQT for the evaluation which
    // combines the Piece Values with the original PSQT Values

    for (int sq = 0; sq < SQUARE_NB; sq++) {

        const int w32 = relativeSquare32(WHITE, sq);
        const int b32 = relativeSquare32(BLACK, sq);

        PSQT[WHITE_PAWN  ][sq] = + PawnValue   +   PawnPSQT32[w32];
        PSQT[WHITE_KNIGHT][sq] = + KnightValue + KnightPSQT32[w32];
        PSQT[WHITE_BISHOP][sq] = + BishopValue + BishopPSQT32[w32];
        PSQT[WHITE_ROOK  ][sq] = + RookValue   +   RookPSQT32[w32];
        PSQT[WHITE_QUEEN ][sq] = + QueenValue  +  QueenPSQT32[w32];
        PSQT[WHITE_KING  ][sq] = + KingValue   +   KingPSQT32[w32];

        PSQT[BLACK_PAWN  ][sq] = - PawnValue   -   PawnPSQT32[b32];
        PSQT[BLACK_KNIGHT][sq] = - KnightValue - KnightPSQT32[b32];
        PSQT[BLACK_BISHOP][sq] = - BishopValue - BishopPSQT32[b32];
        PSQT[BLACK_ROOK  ][sq] = - RookValue   -   RookPSQT32[b32];
        PSQT[BLACK_QUEEN ][sq] = - QueenValue  -  QueenPSQT32[b32];
        PSQT[BLACK_KING  ][sq] = - KingValue   -   KingPSQT32[b32];
    }
}
