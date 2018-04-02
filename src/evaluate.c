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

#include "board.h"
#include "bitboards.h"
#include "bitutils.h"
#include "castle.h"
#include "evaluate.h"
#include "magics.h"
#include "masks.h"
#include "movegen.h"
#include "piece.h"
#include "square.h"
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

// Undefined after all evaluation terms
#define S(mg, eg) (MakeScore((mg), (eg)))


// Definition of Values for each Piece type

const int PawnValue   = S( 100, 121);
const int KnightValue = S( 459, 390);
const int BishopValue = S( 465, 412);
const int RookValue   = S( 630, 711);
const int QueenValue  = S(1272,1317);
const int KingValue   = S( 165, 165);

const int PieceValues[8][PHASE_NB] = {
    { 100, 121}, { 459, 390}, { 465, 412}, { 630, 711},
    {1272,1317}, { 165, 165}, {   0,   0}, {   0,   0},
};


// Definition of evaluation terms related to Pawns

const int PawnIsolated = S(  -3,  -6);

const int PawnStacked = S( -11, -31);

const int PawnBackwards[2] = { S(   6,  -3), S( -13, -11) };

const int PawnConnected32[32] = {
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
    S(   0, -16), S(   6,   1), S(   3,  -3), S(   5,  19),
    S(   7,   0), S(  21,   0), S(  15,   8), S(  17,  23),
    S(   7,   0), S(  19,   4), S(  16,   9), S(  18,  17),
    S(   6,  14), S(  19,  19), S(  18,  24), S(  38,  25),
    S(  24,  55), S(  30,  63), S(  80,  60), S(  60,  77),
    S( 148,   3), S( 167,  16), S( 203,  29), S( 197,  87),
    S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};


// Definition of evaluation terms related to Knights

const int KnightAttackedByPawn = S( -49, -32);

const int KnightRammedPawns = S(   0,   5);

const int KnightOutpost[2] = { S(  19, -34), S(  38,   9) };

const int KnightMobility[9] = {
    S( -86, -98), S( -37, -84), S( -15, -40),
    S(  -5, -11), S(   3, -13), S(   8,   0),
    S(  18,  -1), S(  32,  -1), S(  48, -30),
};


// Definition of evaluation terms related to Bishops

const int BishopPair = S(  43,  68);

const int BishopRammedPawns = S(  -8,  -6);

const int BishopAttackedByPawn = S( -52, -34);

const int BishopOutpost[2] = { S(  20, -16), S(  53, -10) };

const int BishopMobility[14] = {
    S( -58,-120), S( -47, -63), S( -19, -45), S(  -5, -21),
    S(   4,  -7), S(  16,   0), S(  22,   8), S(  29,   4),
    S(  30,  10), S(  36,   4), S(  42,   4), S(  53, -11),
    S(  41,  -1), S(  34, -28),
};


// Definition of evaluation terms related to Rooks

const int RookFile[2] = { S(  12,   2), S(  41,  -8) };

const int RookOnSeventh = S(   0,  23);

const int RookMobility[15] = {
    S(-148, -88), S( -69,-119), S( -16, -66), S(  -9, -26),
    S(  -8,  -3), S(  -8,  15), S(  -7,  26), S(  -3,  32),
    S(   0,  37), S(   6,  36), S(   9,  42), S(  19,  48),
    S(  19,  50), S(  24,  47), S(  16,  44),
};


// Definition of evaluation terms related to Queens

const int QueenChecked = S( -35, -32);

const int QueenCheckedByPawn = S( -49, -45);

const int QueenMobility[28] = {
    S( -60,-258), S(-169,-232), S( -39,-187), S( -35,-174),
    S( -19,-118), S( -25, -60), S( -19, -89), S( -18, -86),
    S( -15, -58), S( -10, -53), S(  -9, -27), S(  -6, -29),
    S(  -4, -15), S(  -2, -11), S(   1,  -8), S(   0,   4),
    S(   2,  15), S(   0,  14), S(  11,  25), S(   0,  26),
    S(   5,  29), S(  15,  29), S(  23,  14), S(  36,  17),
    S(  51,  25), S(  47,   1), S(  -5,   1), S(  24,  13),
};


// Definition of evaluation terms related to Kings

int KingSafety[64]; // Defined by the Polynomial below

const double KingPolynomial[6] = {
    0.00000011, -0.00009948,  0.00797308,
    0.03141319,  2.18429452, -3.33669140,
};

const int KingDefenders[12] = {
    S( -39,  -4), S( -23,   5), S(   0,   1), S(  10,  -1),
    S(  25,  -1), S(  36,   3), S(  39,   7), S(  33, -76),
    S(  12,   6), S(  12,   6), S(  12,   6), S(  12,   6),
};

const int KingShelter[2][FILE_NB][RANK_NB] = {
  {{S( -15,  17), S(   6,  -8), S(  14,   4), S(  19,   5), S(   3,   0), S(  10,  -2), S( -19, -41), S( -33,   1)},
   {S(   2,   7), S(  16,  -5), S(  17,  -8), S(   0, -11), S( -35,  -3), S( -74,  85), S(  47,  73), S( -34,   0)},
   {S(  14,  14), S(   9,   0), S( -17,   0), S( -11,   0), S( -31,  -2), S(   9, -14), S( -40,  53), S( -16,   1)},
   {S(  14,  26), S(  16,   0), S(  -3,  -8), S(  18, -12), S(  16, -31), S( -26, -33), S( -96,  19), S(  -2,   0)},
   {S(  -8,  18), S(   1,   1), S( -27,   1), S( -14,   2), S( -39, -15), S( -34, -23), S(   2,   0), S( -15,   0)},
   {S(  22,   2), S(  18,  -4), S( -18,  -1), S(  -3, -20), S(   3, -32), S(  14, -47), S(  70, -36), S( -15,   0)},
   {S(  20,   1), S(   3,  -9), S( -26,  -9), S( -21, -13), S( -25, -16), S( -46,  -1), S( -29,  32), S( -32,   9)},
   {S( -13,  -3), S(   0,  -8), S(   5,   0), S(   1,   3), S( -14,  11), S(  -3,  30), S(-136,  69), S( -19,  15)}},
  {{S(   0,   0), S(  -1, -17), S(   2, -17), S( -63,  13), S(  14, -14), S( -30,  30), S(-136,  19), S( -58,   9)},
   {S(   0,   0), S(  16,  -5), S(   6,  -5), S(  -1,  -4), S(   6, -25), S(   2,  81), S(-197,  28), S( -46,   3)},
   {S(   0,   0), S(  24,   1), S(   2,  -5), S(  19, -25), S(  13,  -4), S( -94,  47), S(-133, -84), S( -21,  -1)},
   {S(   0,   0), S(  -2,   9), S(  -6,  13), S( -17,   0), S( -29,  -5), S(-105,  -1), S(  29, -22), S( -25,   0)},
   {S(   0,   0), S(   6,   4), S(   9,  -7), S(  21,  -6), S(   7, -18), S( -52,  13), S( -67, -92), S(  -6,  -3)},
   {S(   0,   0), S(  10,   1), S( -10,  -3), S( -22, -13), S(  11, -32), S( -36,   2), S(  -6,  21), S( -30,   0)},
   {S(   0,   0), S(  13,  -1), S(  -1,   0), S( -19,  -6), S( -25, -16), S(   9,  -5), S( -93, -44), S( -46,  13)},
   {S(   0,   0), S(   8, -27), S(  10, -14), S( -26,   1), S( -32,  -2), S(   1, -24), S(-187, -53), S( -47,  17)}},
};


// Definition of evaluation terms related to Passed Pawns

const int PassedPawn[2][2][RANK_NB] = {
  {{S(   0,   0), S( -33, -30), S( -24,   8), S( -13,  -2), S(  24,   0), S(  66,  -5), S( 160,  32), S(   0,   0)},
   {S(   0,   0), S(  -2,   1), S( -14,  23), S( -15,  35), S(   7,  44), S(  72,  60), S( 194, 129), S(   0,   0)}},
  {{S(   0,   0), S(  -7,  12), S( -12,   6), S( -10,  27), S(  27,  32), S(  86,  63), S( 230, 149), S(   0,   0)},
   {S(   0,   0), S(  -5,   8), S( -12,  17), S( -21,  52), S( -14, 109), S(  28, 202), S( 119, 369), S(   0,   0)}},
};


// Definition of evaluation terms related to general properties

const int Tempo[COLOUR_NB] = { S(  25,  12), S( -25, -12) };


#undef S // Undefine MakeScore


int evaluateBoard(Board* board, EvalInfo* ei, PawnKingTable* pktable){
    
    int phase, eval, pkeval;
    
    // evaluateDraws handles obvious drawn positions
    ei->positionIsDrawn = evaluateDraws(board);
    if (ei->positionIsDrawn) return 0;
    
    // Setup and perform the evaluation of all pieces
    initializeEvalInfo(ei, board, pktable);
    eval = evaluatePieces(ei, board);
    
    // Store a new Pawn King Entry if we did not have one
    if (ei->pkentry == NULL && !TEXEL){
        pkeval = ei->pkeval[WHITE] - ei->pkeval[BLACK];
        storePawnKingEntry(pktable, board->pkhash, ei->passedPawns, pkeval);
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

    return eval;
}

int evaluatePawns(EvalInfo* ei, Board* board, int colour){
    
    const int forward = (colour == WHITE) ? 8 : -8;
    
    int sq, semi, eval = 0;
    uint64_t pawns, myPawns, tempPawns, enemyPawns, attacks;
    
    // Update the attacks array with the pawn attacks. We will use this to
    // determine whether or not passed pawns may advance safely later on.
    // It is also used to compute pawn threats against minors and majors
    ei->attackedBy2[colour]     = ei->pawnAttacks[colour] & ei->attacked[colour];
    ei->attacked[colour]       |= ei->pawnAttacks[colour];
    ei->attackedNoQueen[colour] = ei->pawnAttacks[colour];
    
    // Update the attack counts for our pawns. We will not count squares twice
    // even if they are attacked by two pawns. Also, we do not count pawns
    // torwards our attackers counts, which is used to decide when to look
    // at the King Safety of a position.
    attacks = ei->pawnAttacks[colour] & ei->kingAreas[!colour];
    ei->attackCounts[colour] += popcount(attacks);
    
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
        
        if (TRACE) T.pawnCounts[colour]++;
        if (TRACE) T.pawnPSQT[colour][sq]++;
        
        // Save the fact that this pawn is passed
        if (!(PassedPawnMasks[colour][sq] & enemyPawns))
            ei->passedPawns |= (1ull << sq);
        
        // Apply a penalty if the pawn is isolated
        if (!(IsolatedPawnMasks[sq] & tempPawns)){
            eval += PawnIsolated;
            if (TRACE) T.pawnIsolated[colour]++;
        }
        
        // Apply a penalty if the pawn is stacked
        if (Files[File(sq)] & tempPawns){
            eval += PawnStacked;
            if (TRACE) T.pawnStacked[colour]++;
        }
        
        // Apply a penalty if the pawn is backward
        if (   !(PassedPawnMasks[!colour][sq] & myPawns)
            &&  (ei->pawnAttacks[!colour] & (1ull << (sq + forward)))){
            semi = !(Files[File(sq)] & enemyPawns);
            eval += PawnBackwards[semi];
            if (TRACE) T.pawnBackwards[colour][semi]++;
        }
        
        // Apply a bonus if the pawn is connected and not backward
        else if (PawnConnectedMasks[colour][sq] & myPawns){
            eval += PawnConnected32[relativeSquare32(sq, colour)];
            if (TRACE) T.pawnConnected[colour][sq]++;
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
    
    // Evaluate each knight
    while (tempKnights){
        
        // Pop off the next knight
        sq = poplsb(&tempKnights);
        
        if (TRACE) T.knightCounts[colour]++;
        if (TRACE) T.knightPSQT[colour][sq]++;
        
        // Update the attacks array with the knight attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = knightAttacks(sq, ~0ull);
        ei->attackedBy2[colour] |= ei->attacked[colour] & attacks;
        ei->attacked[colour] |= attacks;
        ei->attackedNoQueen[colour] |= attacks;
        
        // Apply a bonus for the knight based on number of rammed pawns
        count = popcount(ei->rammedPawns[colour]);
        eval += count * KnightRammedPawns;
        if (TRACE) T.knightRammedPawns[colour] += count;
        
        // Apply a penalty if the knight is being attacked by a pawn
        if (ei->pawnAttacks[!colour] & (1ull << sq)){
            eval += KnightAttackedByPawn;
            if (TRACE) T.knightAttackedByPawn[colour]++;
        }
        
        // Apply a bonus if the knight is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the knight.
        if (    (OutpostRanks[colour] & (1ull << sq))
            && !(OutpostSquareMasks[colour][sq] & enemyPawns)){
            defended = !!(ei->pawnAttacks[colour] & (1ull << sq));
            eval += KnightOutpost[defended];
            if (TRACE) T.knightOutpost[colour][defended]++;
        }
        
        // Apply a bonus (or penalty) based on the mobility of the knight
        count = popcount((ei->mobilityAreas[colour] & attacks));
        eval += KnightMobility[count];
        if (TRACE) T.knightMobility[colour][count]++;
        
        // Update the attack and attacker counts for the
        // knight for use in the king safety calculation.
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->attackCounts[colour] += 2 * popcount(attacks);
            ei->attackerCounts[colour] += 1;
        }
    }
   
    return eval;
}

int evaluateBishops(EvalInfo* ei, Board* board, int colour){
    
    int sq, defended, count, eval = 0;
    uint64_t tempBishops, enemyPawns, attacks;
    
    tempBishops = board->pieces[BISHOP] & board->colours[colour];
    enemyPawns = board->pieces[PAWN] & board->colours[!colour];
    
    // Apply a bonus for having a pair of bishops
    if ((tempBishops & WHITE_SQUARES) && (tempBishops & BLACK_SQUARES)){
        eval += BishopPair;
        if (TRACE) T.bishopPair[colour]++;
    }
    
    // Evaluate each bishop
    while (tempBishops){
        
        // Pop off the next Bishop
        sq = poplsb(&tempBishops);
        
        if (TRACE) T.bishopCounts[colour]++;
        if (TRACE) T.bishopPSQT[colour][sq]++;
        
        // Update the attacks array with the bishop attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = bishopAttacks(sq, ei->occupiedMinusBishops[colour], ~0ull);
        ei->attackedBy2[colour] |= ei->attacked[colour] & attacks;
        ei->attacked[colour] |= attacks;
        ei->attackedNoQueen[colour] |= attacks;
        
        // Apply a penalty for the bishop based on number of rammed pawns
        // of our own colour, which reside on the same shade of square as the bishop
        count = popcount(ei->rammedPawns[colour] & (((1ull << sq) & WHITE_SQUARES ? WHITE_SQUARES : BLACK_SQUARES)));
        eval += count * BishopRammedPawns;
        if (TRACE) T.bishopRammedPawns[colour] += count;
        
        // Apply a penalty if the bishop is being attacked by a pawn
        if (ei->pawnAttacks[!colour] & (1ull << sq)){
            eval += BishopAttackedByPawn;
            if (TRACE) T.bishopAttackedByPawn[colour]++;
        }
        
        // Apply a bonus if the bishop is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the bishop.
        if (    (OutpostRanks[colour] & (1ull << sq))
            && !(OutpostSquareMasks[colour][sq] & enemyPawns)){
            defended = !!(ei->pawnAttacks[colour] & (1ull << sq));
            eval += BishopOutpost[defended];
            if (TRACE) T.bishopOutpost[colour][defended]++;
        }
        
        // Apply a bonus (or penalty) based on the mobility of the bishop
        count = popcount((ei->mobilityAreas[colour] & attacks));
        eval += BishopMobility[count];
        if (TRACE) T.bishopMobility[colour][count]++;
        
        // Update the attack and attacker counts for the
        // bishop for use in the king safety calculation.
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->attackCounts[colour] += 2 * popcount(attacks);
            ei->attackerCounts[colour] += 1;
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
    
    // Evaluate each rook
    while (tempRooks){
        
        // Pop off the next rook
        sq = poplsb(&tempRooks);
        
        if (TRACE) T.rookCounts[colour]++;
        if (TRACE) T.rookPSQT[colour][sq]++;
        
        // Update the attacks array with the rooks attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = rookAttacks(sq, ei->occupiedMinusRooks[colour], ~0ull);
        ei->attackedBy2[colour] |= ei->attacked[colour] & attacks;
        ei->attacked[colour] |= attacks;
        ei->attackedNoQueen[colour] |= attacks;
        
        // Rook is on a semi-open file if there are no
        // pawns of the Rook's colour on the file. If
        // there are no pawns at all, it is an open file
        if (!(myPawns & Files[File(sq)])){
            open = !(enemyPawns & Files[File(sq)]);
            eval += RookFile[open];
            if (TRACE) T.rookFile[colour][open]++;
        }
        
        // Rook gains a bonus for being located on seventh rank relative to its
        // colour so long as the enemy king is on the last two ranks of the board
        if (   Rank(sq) == (colour == BLACK ? 1 : 6)
            && Rank(relativeSquare(getlsb(enemyKings), colour)) >= 6){
            eval += RookOnSeventh;
            if (TRACE) T.rookOnSeventh[colour]++;
        }
        
        // Apply a bonus (or penalty) based on the mobility of the rook
        count = popcount((ei->mobilityAreas[colour] & attacks));
        eval += RookMobility[count];
        if (TRACE) T.rookMobility[colour][count]++;
        
        // Update the attack and attacker counts for the
        // rook for use in the king safety calculation.
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->attackCounts[colour] += 3 * popcount(attacks);
            ei->attackerCounts[colour] += 1;
        }
    }
    
    return eval;
}

int evaluateQueens(EvalInfo* ei, Board* board, int colour){
    
    int sq, count, eval = 0;
    uint64_t tempQueens, attacks;
    
    tempQueens = board->pieces[QUEEN] & board->colours[colour];
    
    // Evaluate each queen
    while (tempQueens){
        
        // Pop off the next queen
        sq = poplsb(&tempQueens);
        
        if (TRACE) T.queenCounts[colour]++;
        if (TRACE) T.queenPSQT[colour][sq]++;
        
        // Update the attacks array with the rooks attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = rookAttacks(sq, ei->occupiedMinusRooks[colour], ~0ull)
                | bishopAttacks(sq, ei->occupiedMinusBishops[colour], ~0ull);
        ei->attackedBy2[colour] |= ei->attacked[colour] & attacks;
        ei->attacked[colour] |= attacks;
            
        // Apply a penalty if the queen is under an attack threat
        if ((1ull << sq) & ei->attackedNoQueen[!colour]){
            eval += QueenChecked;
            if (TRACE) T.queenChecked[colour]++;
        }
        
        // Apply a penalty if the queen is under attack by a pawn
        if ((1ull << sq) & ei->pawnAttacks[!colour]){
            eval += QueenCheckedByPawn;
            if (TRACE) T.queenCheckedByPawn[colour]++;
        }
            
        // Apply a bonus (or penalty) based on the mobility of the queen
        count = popcount((ei->mobilityAreas[colour] & attacks));
        eval += QueenMobility[count];
        if (TRACE) T.queenMobility[colour][count]++;
        
        // Update the attack and attacker counts for the queen for use in
        // the king safety calculation. We could the Queen as two attacking
        // pieces. This way King Safety is always used with the Queen attacks
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->attackCounts[colour] += 4 * popcount(attacks);
            ei->attackerCounts[colour] += 2;
        }
    }
    
    return eval;
}

int evaluateKings(EvalInfo* ei, Board* board, int colour){
    
    int file, kingFile, kingRank, kingSq;
    int distance, count, eval = 0, pkeval = 0;
    
    uint64_t filePawns;
    
    uint64_t myPawns = board->pieces[PAWN] & board->colours[colour];
    uint64_t myKings = board->pieces[KING] & board->colours[colour];
    
    uint64_t myDefenders  = (board->pieces[PAWN  ] & board->colours[colour])
                          | (board->pieces[KNIGHT] & board->colours[colour])
                          | (board->pieces[BISHOP] & board->colours[colour]);
                          
    kingSq = getlsb(myKings);
    kingFile = File(kingSq);
    kingRank = Rank(kingSq);
    
    // For Tuning Piece Square Table for Kings
    if (TRACE) T.kingPSQT[colour][kingSq]++;
    
    // Bonus for our pawns and minors sitting within our king area
    count = popcount(myDefenders & ei->kingAreas[colour]);
    eval += KingDefenders[count];
    if (TRACE) T.kingDefenders[colour][count]++;
    
    // If we have two or more threats to our king area, we will apply a penalty
    // based on the number of squares attacked, and the strength of the attackers
    if (ei->attackerCounts[!colour] >= 2){
        
        count = ei->attackCounts[!colour];
        
        // Add an extra two attack counts per missing pawn in the king area.
        count += 6 - 2 * popcount(myPawns & ei->kingAreas[colour]);
        
        // Scale down attack count if there are no enemy queens
        if (!(board->colours[!colour] & board->pieces[QUEEN]))
            count *= .25;
    
        eval -= KingSafety[MIN(63, MAX(0, count))];
    }
    
    // Pawn Shelter evaluation is stored in the PawnKing evaluation table
    if (ei->pkentry != NULL) return eval;
    
    // Evaluate Pawn Shelter. We will look at the King's file and any adjacent files
    // to the King's file. We evaluate the distance between the king and the most backward
    // pawn. We will not look at pawns behind the king, and will consider that as having
    // no pawn on the file. No pawn on a file is used with distance equals 7, as no pawn
    // can ever be a distance of 7 from the king. Different bonus is in order when we are
    // looking at the file on which the King sits.
    
    for (file = MAX(0, kingFile - 1); file <= MIN(7, kingFile + 1); file++){
        
        filePawns = myPawns & Files[file] & RanksAtOrAboveMasks[colour][kingRank];
        
        distance = filePawns ? 
                   colour == WHITE ? Rank(getlsb(filePawns)) - kingRank
                                   : kingRank - Rank(getmsb(filePawns))
                                   : 7;

        pkeval += KingShelter[file == kingFile][file][distance];
        if (TRACE) T.kingShelter[colour][file == kingFile][file][distance]++;
    }
    
    ei->pkeval[colour] += pkeval;
    
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
        rank = (colour == BLACK) ? (7 - Rank(sq)) : Rank(sq);
        
        // Determine where the pawn would advance to
        destination = (colour == BLACK) ? ((1ull << sq) >> 8): ((1ull << sq) << 8);
            
        // Destination does not have any pieces on it
        canAdvance = !(destination & notEmpty);
        
        // Destination is not attacked by the opponent
        safeAdvance = !(destination & ei->attacked[!colour]);
        
        eval += PassedPawn[canAdvance][safeAdvance][rank];
        if (TRACE) T.passedPawn[colour][canAdvance][safeAdvance][rank]++;
    }
    
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
    
    ei->pawnAttacks[WHITE] = ((whitePawns << 9) & ~FILE_A) | ((whitePawns << 7) & ~FILE_H);
    ei->pawnAttacks[BLACK] = ((blackPawns >> 9) & ~FILE_H) | ((blackPawns >> 7) & ~FILE_A);
    
    ei->rammedPawns[WHITE] = (blackPawns >> 8) & whitePawns;
    ei->rammedPawns[BLACK] = (whitePawns << 8) & blackPawns;
    
    ei->blockedPawns[WHITE] = ((whitePawns << 8) & (white | black)) >> 8;
    ei->blockedPawns[BLACK] = ((blackPawns >> 8) & (white | black)) << 8,
    
    ei->kingAreas[WHITE] = KingMap[wKingSq] | (1ull << wKingSq) | (KingMap[wKingSq] << 8);
    ei->kingAreas[BLACK] = KingMap[bKingSq] | (1ull << bKingSq) | (KingMap[bKingSq] >> 8);
    
    ei->mobilityAreas[WHITE] = ~(ei->pawnAttacks[BLACK] | (white & kings) | ei->blockedPawns[WHITE]);
    ei->mobilityAreas[BLACK] = ~(ei->pawnAttacks[WHITE] | (black & kings) | ei->blockedPawns[BLACK]);
    
    ei->attacked[WHITE] = kingAttacks(wKingSq, ~0ull);
    ei->attacked[BLACK] = kingAttacks(bKingSq, ~0ull);
    
    ei->occupiedMinusBishops[WHITE] = (white | black) ^ (white & (bishops | queens));
    ei->occupiedMinusBishops[BLACK] = (white | black) ^ (black & (bishops | queens));
    
    ei->occupiedMinusRooks[WHITE] = (white | black) ^ (white & (rooks | queens));
    ei->occupiedMinusRooks[BLACK] = (white | black) ^ (black & (rooks | queens));
    
    ei->passedPawns = 0ull;
    
    ei->attackCounts[WHITE] = ei->attackCounts[BLACK] = 0;
    ei->attackerCounts[WHITE] = ei->attackerCounts[BLACK] = 0;
    
    if (TEXEL) ei->pkentry = NULL;
    else       ei->pkentry = getPawnKingEntry(pktable, board->pkhash);
}

void initializeEvaluation(){
    
    int i;
    
    // Compute values for the King Safety based on the King Polynomial. We only
    // apply King Safety to the midgame score, so as an extra, but non-needed step,
    // we compute a combined score with a zero EG argument.
    for (i = 0; i < 64; i++){
        KingSafety[i] = (int)(
            + KingPolynomial[0] * pow(i, 5) + KingPolynomial[1] * pow(i, 4)
            + KingPolynomial[2] * pow(i, 3) + KingPolynomial[3] * pow(i, 2)
            + KingPolynomial[4] * pow(i, 1) + KingPolynomial[5] * pow(i, 0)
        );
        
        KingSafety[i] = MakeScore(KingSafety[i], 0);
    }
}