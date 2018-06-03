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

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attacks.h"
#include "board.h"
#include "bitboards.h"
#include "castle.h"
#include "evaluate.h"
#include "masks.h"
#include "movegen.h"
#include "psqt.h"
#include "transposition.h"
#include "types.h"

#ifdef TUNE
    const int TEXEL = 1;
    const int TRACE = 1;
    const EvalTrace EmptyTrace;
    EvalTrace T;
#else
    const int TEXEL = 0;
    const int TRACE = 0;
    EvalTrace T;
#endif

#define S(mg, eg) (MakeScore((mg), (eg)))

const int PawnValue   = S( 100, 123);

const int KnightValue = S( 463, 392);

const int BishopValue = S( 473, 417);

const int RookValue   = S( 639, 717);

const int QueenValue  = S(1313,1348);

const int KingValue   = S(   0,   0);

const int PieceValues[8][PHASE_NB] = {
    { 100, 123}, { 463, 392}, { 473, 417}, { 639, 717},
    {1313,1348}, {   0,   0}, {   0,   0}, {   0,   0},
};

const int PawnIsolated = S(  -3,  -4);

const int PawnStacked = S( -10, -32);

const int PawnBackwards[2] = { S(   7,  -3), S( -11, -11) };

const int PawnConnected32[32] = {
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
    S(   0, -16), S(   7,   1), S(   3,  -3), S(   5,  20),
    S(   7,   0), S(  21,   0), S(  15,   8), S(  17,  21),
    S(   6,   0), S(  20,   3), S(  14,   7), S(  16,  17),
    S(   6,  11), S(  20,  20), S(  19,  24), S(  37,  24),
    S(  23,  55), S(  24,  65), S(  66,  63), S(  50,  75),
    S( 106, -14), S( 199,  17), S( 227,  22), S( 250,  76),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

const int KnightRammedPawns = S(   0,   5);

const int KnightOutpost[2] = { S(  18, -35), S(  36,   5) };

const int KnightMobility[9] = {
    S( -87, -97), S( -37, -90), S( -19, -42),
    S(  -6, -14), S(   2, -14), S(   7,  -1),
    S(  17,  -2), S(  32,  -3), S(  48, -33),
};

const int BishopPair = S(  40,  69);

const int BishopRammedPawns = S( -11,  -7);

const int BishopOutpost[2] = { S(  18, -16), S(  50,  -9) };

const int BishopMobility[14] = {
    S( -59,-128), S( -48, -67), S( -18, -46), S(  -5, -21),
    S(   5,  -9), S(  17,   0), S(  22,   7), S(  27,   4),
    S(  28,   9), S(  34,   3), S(  36,   4), S(  46, -15),
    S(  46,  -4), S(  40, -35),
};

const int RookFile[2] = { S(  14,   0), S(  38,  -8) };

const int RookOnSeventh = S(   0,  25);

const int RookMobility[15] = {
    S(-147,-107), S( -72,-120), S( -16, -68), S(  -9, -26),
    S(  -8,  -3), S(  -7,  14), S(  -8,  25), S(  -3,  32),
    S(   1,  35), S(   5,  36), S(   9,  42), S(  17,  48),
    S(  19,  50), S(  25,  46), S(  20,  47),
};

const int QueenMobility[28] = {
    S( -61,-263), S(-217,-390), S( -48,-205), S( -36,-190),
    S( -15,-126), S( -24, -66), S( -15, -91), S( -17, -83),
    S( -13, -60), S( -10, -54), S(  -8, -28), S(  -6, -28),
    S(  -6, -15), S(  -1, -10), S(   0,  -6), S(  -2,   4),
    S(   3,  17), S(   0,  16), S(  11,  25), S(  -2,  25),
    S(   4,  26), S(  18,  28), S(  13,   8), S(  39,  13),
    S(  44,  20), S(  62,   1), S( -33,  -7), S(  14,   3),
};

const int KingDefenders[12] = {
    S( -37,  -4), S( -18,   6), S(   0,   1), S(  10,   0),
    S(  24,  -3), S(  35,   2), S(  39,  14), S(  28,-207),
    S(  12,   6), S(  12,   6), S(  12,   6), S(  12,   6),
};

const int KingShelter[2][FILE_NB][RANK_NB] = {
  {{S( -17,  15), S(   6, -11), S(  16,   1), S(  23,   2), S(   8,   7), S(  31,   4), S(  -1, -33), S( -31,   2)},
   {S(   4,   6), S(  16,  -8), S(  12, -10), S(  -2, -13), S( -27,   0), S( -66,  79), S( 101,  94), S( -30,   1)},
   {S(  13,  13), S(   8,  -2), S( -20,   0), S(  -7,  -2), S( -22,  -3), S(  14,  -9), S(   4,  65), S( -16,   2)},
   {S(   9,  31), S(  14,   0), S(  -4,  -8), S(  20, -16), S(  16, -36), S( -23, -33), S(-148,  41), S(  -1,   0)},
   {S( -19,  20), S(  -2,   3), S( -31,   2), S( -11,   5), S( -25, -17), S( -41, -19), S(  39,  -8), S( -14,   0)},
   {S(  22,   3), S(  17,  -4), S( -21,   0), S(  -3, -21), S(   7, -32), S(  17, -49), S(  50, -34), S( -12,   0)},
   {S(  19,   3), S(   2, -10), S( -30,  -8), S( -17, -14), S( -24, -21), S( -43,   9), S(   7,  47), S( -26,   8)},
   {S( -19,  -3), S(  -1,  -9), S(   4,   0), S(   3,   3), S( -11,  14), S(   4,  33), S(-201,  79), S( -18,  15)}},
  {{S(   0,   0), S(   5, -16), S(   6, -14), S( -46,  19), S(   0, -10), S(   9,  51), S(-171, -15), S( -51,  11)},
   {S(   0,   0), S(  17,  -7), S(   8,  -7), S(  -5,  -8), S(  11, -33), S(  52, 103), S(-188, -11), S( -45,   4)},
   {S(   0,   0), S(  24,   0), S(   0,  -7), S(  20, -27), S(  14,  -8), S(-114,  45), S( -90, -84), S( -22,  -1)},
   {S(   0,   0), S(  -6,   9), S( -11,  13), S( -14,   3), S( -24,   0), S(-117,  10), S(   5, -50), S( -28,   1)},
   {S(   0,   0), S(   4,   5), S(   8,  -7), S(  25,  -4), S(  10, -23), S( -45,   8), S(-109, -73), S(  -4,  -3)},
   {S(   0,   0), S(  10,   1), S( -17,  -1), S( -12, -14), S(  30, -38), S( -45,   5), S(  59,  47), S( -30,   1)},
   {S(   0,   0), S(  12,  -1), S(  -3,   0), S( -20,  -5), S( -20, -13), S(  23,  12), S( -58, -64), S( -43,  14)},
   {S(   0,   0), S(   8, -28), S(   9, -16), S( -22,   0), S( -27,  -3), S(   7, -17), S(-240, -74), S( -44,  16)}},
};

const int KSAttackWeight[]  = { 0, 16, 6, 10, 8, 0 };
const int KSAttackValue     =   44;
const int KSWeakSquares     =   38;
const int KSFriendlyPawns   =  -22;
const int KSNoEnemyQueens   = -256;
const int KSSafeQueenCheck  =   86;
const int KSSafeRookCheck   =   86;
const int KSSafeBishopCheck =   46;
const int KSSafeKnightCheck =  119;
const int KSAdjustment      =  -36;

const int PassedPawn[2][2][RANK_NB] = {
  {{S(   0,   0), S( -31, -27), S( -25,   7), S( -16,  -3), S(  20,   0), S(  59,  -4), S( 147,  33), S(   0,   0)},
   {S(   0,   0), S(  -1,   2), S( -19,  24), S( -12,  37), S(   6,  46), S(  66,  63), S( 191, 133), S(   0,   0)}},
  {{S(   0,   0), S(  -7,  15), S( -13,   9), S(  -6,  28), S(  29,  34), S(  78,  66), S( 234, 152), S(   0,   0)},
   {S(   0,   0), S(  -9,   9), S( -12,  18), S( -18,  54), S(  -5, 113), S(  41, 213), S( 126, 378), S(   0,   0)}},
};

const int ThreatPawnAttackedByOne    = S( -17, -27);
const int ThreatMinorAttackedByPawn  = S( -73, -54);
const int ThreatMinorAttackedByMajor = S( -43, -41);
const int ThreatQueenAttackedByOne   = S( -84,   3);
const int ThreatOverloadedPieces     = S(  -7, -19);

const int Tempo[COLOUR_NB] = { S(  25,  12), S( -25, -12) };

#undef S


int evaluateBoard(Board* board, PawnKingTable* pktable){

    EvalInfo ei; // Fix bogus GCC warning
    ei.pkeval[WHITE] = ei.pkeval[BLACK] = 0;

    int phase, eval, pkeval;

    // Check for obviously drawn positions
    if (evaluateDraws(board)) return 0;

    // Setup and perform the evaluation of all pieces
    initializeEvalInfo(&ei, board, pktable);
    eval = evaluatePieces(&ei, board);

    // Store a new Pawn King Entry if we did not have one
    if (ei.pkentry == NULL && !TEXEL){
        pkeval = ei.pkeval[WHITE] - ei.pkeval[BLACK];
        storePawnKingEntry(pktable, board->pkhash, ei.passedPawns, pkeval);
    }

    // Add in the PSQT and Material values, as well as the tempo
    eval += board->psqtmat + Tempo[board->turn];

    // Calcuate the game phase based on remaining material (Fruit Method)
    phase = 24 - popcount(board->pieces[QUEEN]) * 4
               - popcount(board->pieces[ROOK]) * 2
               - popcount(board->pieces[KNIGHT] | board->pieces[BISHOP]);
    phase = (phase * 256 + 12) / 24;

    // Compute the interpolated evaluation
    eval  = (ScoreMG(eval) * (256 - phase) + ScoreEG(eval) * phase) / 256;

    // Return the evaluation relative to the side to move
    return board->turn == WHITE ? eval : -eval;
}

int evaluateDraws(Board* board){

    uint64_t white   = board->colours[WHITE];
    uint64_t black   = board->colours[BLACK];
    uint64_t pawns   = board->pieces[PAWN];
    uint64_t knights = board->pieces[KNIGHT];
    uint64_t bishops = board->pieces[BISHOP];
    uint64_t rooks   = board->pieces[ROOK];
    uint64_t queens  = board->pieces[QUEEN];
    uint64_t kings   = board->pieces[KING];

    // Unlikely to have a draw if we have pawns, rooks, or queens left
    if (pawns | rooks | queens)
        return 0;

    // Check for King Vs. King
    if (kings == (white | black)) return 1;

    if ((white & kings) == white){
        // Check for King Vs King and Knight/Bishop
        if (popcount(black & (knights | bishops)) <= 1) return 1;

        // Check for King Vs King and two Knights
        if (popcount(black & knights) == 2 && (black & bishops) == 0ull) return 1;
    }

    if ((black & kings) == black){
        // Check for King Vs King and Knight/Bishop
        if (popcount(white & (knights | bishops)) <= 1) return 1;

        // Check for King Vs King and two Knights
        if (popcount(white & knights) == 2 && (white & bishops) == 0ull) return 1;
    }

    return 0;
}

int evaluatePieces(EvalInfo* ei, Board* board){

    int eval = 0;

    eval += evaluatePawns(ei, board, WHITE)
          - evaluatePawns(ei, board, BLACK);

    eval += evaluateKnights(ei, board, WHITE)
          - evaluateKnights(ei, board, BLACK);

    eval += evaluateBishops(ei, board, WHITE)
          - evaluateBishops(ei, board, BLACK);

    eval += evaluateRooks(ei, board, WHITE)
          - evaluateRooks(ei, board, BLACK);

    eval += evaluateQueens(ei, board, WHITE)
          - evaluateQueens(ei, board, BLACK);

    eval += evaluateKings(ei, board, WHITE)
          - evaluateKings(ei, board, BLACK);

    eval += evaluatePassedPawns(ei, board, WHITE)
          - evaluatePassedPawns(ei, board, BLACK);

    eval += evaluateThreats(ei, board, WHITE)
          - evaluateThreats(ei, board, BLACK);

    return eval;
}

int evaluatePawns(EvalInfo* ei, Board* board, int colour){

    const int forward = (colour == WHITE) ? 8 : -8;

    int sq, semi, eval = 0;
    uint64_t pawns, myPawns, tempPawns, enemyPawns, attacks;

    // Update the attacks array with the pawn attacks. We will use this to
    // determine whether or not passed pawns may advance safely later on.
    // It is also used to compute pawn threats against minors and majors
    ei->attackedBy2[colour]      = ei->pawnAttacks[colour] & ei->attacked[colour];
    ei->attacked[colour]        |= ei->pawnAttacks[colour];
    ei->attackedBy[colour][PAWN] = ei->pawnAttacks[colour];

    // Update pawn attacks into the enemy's king area. We do
    // not count pawns torwards attackers or threat weight.
    attacks = ei->pawnAttacks[colour] & ei->kingAreas[!colour];
    ei->kingAttacksCount[colour] += popcount(attacks);

    // The pawn table holds the rest of the eval information we will calculate.
    // We return the saved value only when we evaluate for white, since we save
    // the evaluation as a combination of white and black, from white's POV
    if (ei->pkentry != NULL) return colour == WHITE ? ei->pkentry->eval : 0;

    pawns = board->pieces[PAWN];
    myPawns = tempPawns = pawns & board->colours[colour];
    enemyPawns = pawns & board->colours[!colour];

    // Evaluate each pawn (but not for being passed)
    while (tempPawns != 0ull){

        // Pop off the next pawn
        sq = poplsb(&tempPawns);

        if (TRACE) T.PawnValue[colour]++;
        if (TRACE) T.PawnPSQT32[relativeSquare32(sq, colour)][colour]++;

        // Save the fact that this pawn is passed
        if (!(passedPawnMasks(colour, sq) & enemyPawns))
            ei->passedPawns |= (1ull << sq);

        // Apply a penalty if the pawn is isolated
        if (!(isolatedPawnMasks(sq) & tempPawns)){
            eval += PawnIsolated;
            if (TRACE) T.PawnIsolated[colour]++;
        }

        // Apply a penalty if the pawn is stacked
        if (Files[fileOf(sq)] & tempPawns){
            eval += PawnStacked;
            if (TRACE) T.PawnStacked[colour]++;
        }

        // Apply a penalty if the pawn is backward
        if (   !(passedPawnMasks(!colour, sq) & myPawns)
            &&  (ei->pawnAttacks[!colour] & (1ull << (sq + forward)))){
            semi = !(Files[fileOf(sq)] & enemyPawns);
            eval += PawnBackwards[semi];
            if (TRACE) T.PawnBackwards[semi][colour]++;
        }

        // Apply a bonus if the pawn is connected and not backward
        else if (pawnConnectedMasks(colour, sq) & myPawns){
            eval += PawnConnected32[relativeSquare32(sq, colour)];
            if (TRACE) T.PawnConnected32[relativeSquare32(sq, colour)][colour]++;
        }
    }

    ei->pkeval[colour] = eval;

    return eval;
}

int evaluateKnights(EvalInfo* ei, Board* board, int colour){

    int sq, defended, count, eval = 0;
    uint64_t tempKnights, enemyPawns, attacks;

    tempKnights = board->pieces[KNIGHT] & board->colours[colour];
    enemyPawns = board->pieces[PAWN] & board->colours[!colour];

    ei->attackedBy[colour][KNIGHT] = 0ull;

    // Evaluate each knight
    while (tempKnights){

        // Pop off the next knight
        sq = poplsb(&tempKnights);

        if (TRACE) T.KnightValue[colour]++;
        if (TRACE) T.KnightPSQT32[relativeSquare32(sq, colour)][colour]++;

        // Update the attacks array with the knight attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = knightAttacks(sq);
        ei->attackedBy2[colour]        |= attacks & ei->attacked[colour];
        ei->attacked[colour]           |= attacks;
        ei->attackedBy[colour][KNIGHT] |= attacks;

        // Apply a bonus for the knight based on number of rammed pawns
        count = popcount(ei->rammedPawns[colour]);
        eval += count * KnightRammedPawns;
        if (TRACE) T.KnightRammedPawns[colour] += count;

        // Apply a bonus if the knight is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the knight.
        if (    (outpostRanks(colour) & (1ull << sq))
            && !(outpostSquareMasks(colour, sq) & enemyPawns)){
            defended = !!(ei->pawnAttacks[colour] & (1ull << sq));
            eval += KnightOutpost[defended];
            if (TRACE) T.KnightOutpost[defended][colour]++;
        }

        // Apply a bonus (or penalty) based on the mobility of the knight
        count = popcount((ei->mobilityAreas[colour] & attacks));
        eval += KnightMobility[count];
        if (TRACE) T.KnightMobility[count][colour]++;

        // Update for King Safety calculation
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->kingAttacksCount[colour] += popcount(attacks);
            ei->kingAttackersCount[colour] += 1;
            ei->kingAttackersWeight[colour] += KSAttackWeight[KNIGHT];
        }
    }

    return eval;
}

int evaluateBishops(EvalInfo* ei, Board* board, int colour){

    int sq, defended, count, eval = 0;
    uint64_t tempBishops, enemyPawns, attacks;

    tempBishops = board->pieces[BISHOP] & board->colours[colour];
    enemyPawns = board->pieces[PAWN] & board->colours[!colour];

    ei->attackedBy[colour][BISHOP] = 0ull;

    // Apply a bonus for having a pair of bishops
    if ((tempBishops & WHITE_SQUARES) && (tempBishops & BLACK_SQUARES)){
        eval += BishopPair;
        if (TRACE) T.BishopPair[colour]++;
    }

    // Evaluate each bishop
    while (tempBishops){

        // Pop off the next Bishop
        sq = poplsb(&tempBishops);
        if (TRACE) T.BishopValue[colour]++;
        if (TRACE) T.BishopPSQT32[relativeSquare32(sq, colour)][colour]++;

        // Update the attacks array with the bishop attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = bishopAttacks(sq, ei->occupiedMinusBishops[colour]);
        ei->attackedBy2[colour]        |= attacks & ei->attacked[colour];
        ei->attacked[colour]           |= attacks;
        ei->attackedBy[colour][BISHOP] |= attacks;

        // Apply a penalty for the bishop based on number of rammed pawns
        // of our own colour, which reside on the same shade of square as the bishop
        count = popcount(ei->rammedPawns[colour] & (((1ull << sq) & WHITE_SQUARES ? WHITE_SQUARES : BLACK_SQUARES)));
        eval += count * BishopRammedPawns;
        if (TRACE) T.BishopRammedPawns[colour] += count;

        // Apply a bonus if the bishop is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the bishop.
        if (    (outpostRanks(colour) & (1ull << sq))
            && !(outpostSquareMasks(colour, sq) & enemyPawns)){
            defended = !!(ei->pawnAttacks[colour] & (1ull << sq));
            eval += BishopOutpost[defended];
            if (TRACE) T.BishopOutpost[defended][colour]++;
        }

        // Apply a bonus (or penalty) based on the mobility of the bishop
        count = popcount((ei->mobilityAreas[colour] & attacks));
        eval += BishopMobility[count];
        if (TRACE) T.BishopMobility[count][colour]++;

        // Update for King Safety calculation
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->kingAttacksCount[colour] += popcount(attacks);
            ei->kingAttackersCount[colour] += 1;
            ei->kingAttackersWeight[colour] += KSAttackWeight[BISHOP];
        }
    }

    return eval;
}

int evaluateRooks(EvalInfo* ei, Board* board, int colour){

    int sq, open, count, eval = 0;
    uint64_t attacks;

    uint64_t myPawns    = board->pieces[PAWN] & board->colours[ colour];
    uint64_t enemyPawns = board->pieces[PAWN] & board->colours[!colour];
    uint64_t tempRooks  = board->pieces[ROOK] & board->colours[ colour];
    uint64_t enemyKings = board->pieces[KING] & board->colours[!colour];

    ei->attackedBy[colour][ROOK] = 0ull;

    // Evaluate each rook
    while (tempRooks){

        // Pop off the next rook
        sq = poplsb(&tempRooks);
        if (TRACE) T.RookValue[colour]++;
        if (TRACE) T.RookPSQT32[relativeSquare32(sq, colour)][colour]++;

        // Update the attacks array with the rooks attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = rookAttacks(sq, ei->occupiedMinusRooks[colour]);
        ei->attackedBy2[colour]      |= attacks & ei->attacked[colour];
        ei->attacked[colour]         |= attacks;
        ei->attackedBy[colour][ROOK] |= attacks;

        // Rook is on a semi-open file if there are no
        // pawns of the Rook's colour on the file. If
        // there are no pawns at all, it is an open file
        if (!(myPawns & Files[fileOf(sq)])){
            open = !(enemyPawns & Files[fileOf(sq)]);
            eval += RookFile[open];
            if (TRACE) T.RookFile[open][colour]++;
        }

        // Rook gains a bonus for being located on seventh rank relative to its
        // colour so long as the enemy king is on the last two ranks of the board
        if (   rankOf(sq) == (colour == BLACK ? 1 : 6)
            && relativeRankOf(colour, getlsb(enemyKings)) >= 6) {
            eval += RookOnSeventh;
            if (TRACE) T.RookOnSeventh[colour]++;
        }

        // Apply a bonus (or penalty) based on the mobility of the rook
        count = popcount((ei->mobilityAreas[colour] & attacks));
        eval += RookMobility[count];
        if (TRACE) T.RookMobility[count][colour]++;

        // Update for King Safety calculation
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->kingAttacksCount[colour] += popcount(attacks);
            ei->kingAttackersCount[colour] += 1;
            ei->kingAttackersWeight[colour] += KSAttackWeight[ROOK];
        }
    }

    return eval;
}

int evaluateQueens(EvalInfo* ei, Board* board, int colour){

    int sq, count, eval = 0;
    uint64_t tempQueens, attacks;

    tempQueens = board->pieces[QUEEN] & board->colours[colour];

    ei->attackedBy[colour][QUEEN] = 0ull;

    // Evaluate each queen
    while (tempQueens){

        // Pop off the next queen
        sq = poplsb(&tempQueens);
        if (TRACE) T.QueenValue[colour]++;
        if (TRACE) T.QueenPSQT32[relativeSquare32(sq, colour)][colour]++;

        // Update the attacks array with the rooks attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = rookAttacks(sq, ei->occupiedMinusRooks[colour])
                | bishopAttacks(sq, ei->occupiedMinusBishops[colour]);
        ei->attackedBy2[colour]       |= attacks & ei->attacked[colour];
        ei->attacked[colour]          |= attacks;
        ei->attackedBy[colour][QUEEN] |= attacks;

        // Apply a bonus (or penalty) based on the mobility of the queen
        count = popcount((ei->mobilityAreas[colour] & attacks));
        eval += QueenMobility[count];
        if (TRACE) T.QueenMobility[count][colour]++;

        // Update for King Safety calculation
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->kingAttacksCount[colour] += popcount(attacks);
            ei->kingAttackersCount[colour] += 1;
            ei->kingAttackersWeight[colour] += KSAttackWeight[QUEEN];
        }
    }

    return eval;
}

int evaluateKings(EvalInfo* ei, Board* board, int colour){

    const int US = colour, THEM = !colour;

    int count, eval = 0, pkeval = 0;

    uint64_t myPawns     = board->pieces[PAWN ] & board->colours[  US];
    uint64_t enemyQueens = board->pieces[QUEEN] & board->colours[THEM];
    uint64_t myKings     = board->pieces[KING ] & board->colours[  US];

    uint64_t myDefenders  = (board->pieces[PAWN  ] & board->colours[US])
                          | (board->pieces[KNIGHT] & board->colours[US])
                          | (board->pieces[BISHOP] & board->colours[US]);

    int kingSq = getlsb(myKings);
    int kingFile = fileOf(kingSq);
    int kingRank = rankOf(kingSq);

    if (TRACE) T.KingValue[US]++;
    if (TRACE) T.KingPSQT32[relativeSquare32(kingSq, US)][US]++;

    // Bonus for our pawns and minors sitting within our king area
    count = popcount(myDefenders & ei->kingAreas[US]);
    eval += KingDefenders[count];
    if (TRACE) T.KingDefenders[count][US]++;

    // Perform King Safety when we have two attackers, or
    // one attacker with a potential for a Queen attacker
    if (ei->kingAttackersCount[THEM] > 1 - popcount(enemyQueens)){

        // Weak squares are attacked by the enemy, defended no more
        // than once and only defended by our Queens or our King
        uint64_t weak =   ei->attacked[THEM]
                      &  ~ei->attackedBy2[US]
                      & (~ei->attacked[US] | ei->attackedBy[US][QUEEN] | ei->attackedBy[US][KING]);

        // Usually the King Area is 9 squares. Scale are attack counts to account for
        // when the king is in an open area and expects more attacks, or the opposite
        float scaledAttackCounts = 9.0 * ei->kingAttacksCount[THEM] / popcount(ei->kingAreas[US]);

        // Safe target squares are defended or are weak and attacked by two.
        // We exclude squares containing pieces which we cannot caputre
        uint64_t safe =  ~board->colours[THEM]
                      & (~ei->attacked[US] | (weak & ei->attackedBy2[THEM]));

        // Find square and piece combinations which would check our King
        uint64_t occupied      = board->colours[WHITE] | board->colours[BLACK];
        uint64_t knightThreats = knightAttacks(kingSq);
        uint64_t bishopThreats = bishopAttacks(kingSq, occupied);
        uint64_t rookThreats   = rookAttacks(kingSq, occupied);
        uint64_t queenThreats  = bishopThreats | rookThreats;

        // Identify if pieces can move to those checking squares safely.
        // We check if our Queen can attack the square for safe Queen checks.
        // No attacks of other pieces is implicit in our definition of weak.
        uint64_t knightChecks = knightThreats & safe &  ei->attackedBy[THEM][KNIGHT];
        uint64_t bishopChecks = bishopThreats & safe &  ei->attackedBy[THEM][BISHOP];
        uint64_t rookChecks   = rookThreats   & safe &  ei->attackedBy[THEM][ROOK  ];
        uint64_t queenChecks  = queenThreats  & safe &  ei->attackedBy[THEM][QUEEN ]
                                                     & ~ei->attackedBy[  US][QUEEN ];

        count  = ei->kingAttackersCount[THEM] * ei->kingAttackersWeight[THEM];

        count += KSAttackValue     * scaledAttackCounts
               + KSWeakSquares     * popcount(weak & ei->kingAreas[US])
               + KSFriendlyPawns   * popcount(myPawns & ei->kingAreas[US])
               + KSNoEnemyQueens   * !enemyQueens
               + KSSafeQueenCheck  * !!queenChecks
               + KSSafeRookCheck   * !!rookChecks
               + KSSafeBishopCheck * !!bishopChecks
               + KSSafeKnightCheck * !!knightChecks
               + KSAdjustment;

        // Convert safety to an MG and EG score, if we are unsafe
        if (count > 0) eval -= MakeScore(count * count / 720, count / 20);
    }

    // Shelter eval is already stored in the Pawn King Table. evaluatePawns()
    // returns the associated score, so we only need to return the usual eval
    if (ei->pkentry != NULL) return eval;

    // Evaluate King Shelter. Look at our king's file, as well as the possible two adjacent
    // files. We evaluate based on distance between our king's rang and the nearest friendly
    // pawn that is placed ahead or on rank with our king. Use a distance of 7 to denote
    // configurations which have no such pawn. 7 is not a legal distance normally.
    for (int file = MAX(0, kingFile - 1); file <= MIN(FILE_NB - 1, kingFile + 1); file++){

        uint64_t filePawns = myPawns & Files[file] & ranksAtOrAboveMasks(US, kingRank);

        int distance = !filePawns   ? 7
                     :  US == WHITE ? rankOf(getlsb(filePawns)) - kingRank
                                    : kingRank - rankOf(getmsb(filePawns));

        pkeval += KingShelter[file == kingFile][file][distance];
        if (TRACE) T.KingShelter[file == kingFile][file][distance][US]++;
    }

    ei->pkeval[US] += pkeval;

    return eval + pkeval;
}

int evaluatePassedPawns(EvalInfo* ei, Board* board, int colour){

    int sq, rank, canAdvance, safeAdvance, eval = 0;
    uint64_t tempPawns, destination, notEmpty;

    // Fetch Passed Pawns from the Pawn King Entry if we have one
    if (ei->pkentry != NULL) ei->passedPawns = ei->pkentry->passed;

    tempPawns = board->colours[colour] & ei->passedPawns;
    notEmpty  = board->colours[WHITE ] | board->colours[BLACK];

    // Evaluate each passed pawn
    while (tempPawns != 0ull){

        // Pop off the next passed Pawn
        sq = poplsb(&tempPawns);

        // Determine the releative rank
        rank = (colour == BLACK) ? (7 - rankOf(sq)) : rankOf(sq);

        // Determine where the pawn would advance to
        destination = (colour == BLACK) ? ((1ull << sq) >> 8): ((1ull << sq) << 8);

        // Destination does not have any pieces on it
        canAdvance = !(destination & notEmpty);

        // Destination is not attacked by the opponent
        safeAdvance = !(destination & ei->attacked[!colour]);

        eval += PassedPawn[canAdvance][safeAdvance][rank];
        if (TRACE) T.PassedPawn[canAdvance][safeAdvance][rank][colour]++;
    }

    return eval;
}

int evaluateThreats(EvalInfo* ei, Board* board, int colour){

    int count, eval = 0;

    uint64_t pawns   = board->colours[colour] & board->pieces[PAWN  ];
    uint64_t knights = board->colours[colour] & board->pieces[KNIGHT];
    uint64_t bishops = board->colours[colour] & board->pieces[BISHOP];
    uint64_t rooks   = board->colours[colour] & board->pieces[ROOK  ];
    uint64_t queens  = board->colours[colour] & board->pieces[QUEEN ];

    uint64_t attacksByPawns  = ei->attackedBy[!colour][PAWN  ];
    uint64_t attacksByMajors = ei->attackedBy[!colour][ROOK  ] | ei->attackedBy[!colour][QUEEN ];

    // A friendly minor / major is overloaded if attacked and defended by exactly one
    uint64_t overloaded = (knights | bishops | rooks | queens)
                        & ei->attacked[ colour] & ~ei->attackedBy2[ colour]
                        & ei->attacked[!colour] & ~ei->attackedBy2[!colour];

    // Penalty for each unsupported pawn on the board
    count = popcount(pawns & ~ei->attacked[colour] & ei->attacked[!colour]);
    eval += count * ThreatPawnAttackedByOne;
    if (TRACE) T.ThreatPawnAttackedByOne[colour] += count;

    // Penalty for pawn threats against our minors
    count = popcount((knights | bishops) & attacksByPawns);
    eval += count * ThreatMinorAttackedByPawn;
    if (TRACE) T.ThreatMinorAttackedByPawn[colour] += count;

    // Penalty for all major threats against our unsupported knights and bishops
    count = popcount((knights | bishops) & ~ei->attacked[colour] & attacksByMajors);
    eval += count * ThreatMinorAttackedByMajor;
    if (TRACE) T.ThreatMinorAttackedByMajor[colour] += count;

    // Penalty for any threat against our queens
    count = popcount(queens & ei->attacked[!colour]);
    eval += count * ThreatQueenAttackedByOne;
    if (TRACE) T.ThreatQueenAttackedByOne[colour] += count;

    // Penalty for any overloaded minors or majors
    count = popcount(overloaded);
    eval += count * ThreatOverloadedPieces;
    if (TRACE) T.ThreatOverloadedPieces[colour] += count;

    return eval;
}

void initializeEvalInfo(EvalInfo* ei, Board* board, PawnKingTable* pktable){

    uint64_t white   = board->colours[WHITE];
    uint64_t black   = board->colours[BLACK];
    uint64_t pawns   = board->pieces[PAWN];
    uint64_t bishops = board->pieces[BISHOP];
    uint64_t rooks   = board->pieces[ROOK];
    uint64_t queens  = board->pieces[QUEEN];
    uint64_t kings   = board->pieces[KING];

    uint64_t whitePawns = white & pawns;
    uint64_t blackPawns = black & pawns;

    int wKingSq = getlsb((white & kings));
    int bKingSq = getlsb((black & kings));

    ei->pawnAttacks[WHITE] = pawnAttackSpan(whitePawns, ~0ull, WHITE);
    ei->pawnAttacks[BLACK] = pawnAttackSpan(blackPawns, ~0ull, BLACK);

    ei->rammedPawns[WHITE] = pawnAdvance(blackPawns, ~whitePawns, BLACK);
    ei->rammedPawns[BLACK] = pawnAdvance(whitePawns, ~blackPawns, WHITE);

    ei->blockedPawns[WHITE] = pawnAdvance(white | black, ~whitePawns, BLACK);
    ei->blockedPawns[BLACK] = pawnAdvance(white | black, ~blackPawns, WHITE);

    ei->kingAreas[WHITE] = kingAttacks(wKingSq) | (1ull << wKingSq) | (kingAttacks(wKingSq) << 8);
    ei->kingAreas[BLACK] = kingAttacks(bKingSq) | (1ull << bKingSq) | (kingAttacks(bKingSq) >> 8);

    ei->mobilityAreas[WHITE] = ~(ei->pawnAttacks[BLACK] | (white & kings) | ei->blockedPawns[WHITE]);
    ei->mobilityAreas[BLACK] = ~(ei->pawnAttacks[WHITE] | (black & kings) | ei->blockedPawns[BLACK]);

    ei->attacked[WHITE] = ei->attackedBy[WHITE][KING] = kingAttacks(wKingSq);
    ei->attacked[BLACK] = ei->attackedBy[BLACK][KING] = kingAttacks(bKingSq);

    ei->occupiedMinusBishops[WHITE] = (white | black) ^ (white & (bishops | queens));
    ei->occupiedMinusBishops[BLACK] = (white | black) ^ (black & (bishops | queens));

    ei->occupiedMinusRooks[WHITE] = (white | black) ^ (white & (rooks | queens));
    ei->occupiedMinusRooks[BLACK] = (white | black) ^ (black & (rooks | queens));

    ei->passedPawns = 0ull;

    ei->kingAttacksCount[WHITE]    = ei->kingAttacksCount[BLACK]    = 0;
    ei->kingAttackersCount[WHITE]  = ei->kingAttackersCount[BLACK]  = 0;
    ei->kingAttackersWeight[WHITE] = ei->kingAttackersWeight[BLACK] = 0;

    if (TEXEL) ei->pkentry = NULL;
    else       ei->pkentry = getPawnKingEntry(pktable, board->pkhash);
}
