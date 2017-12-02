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
#include <stdio.h>
#include <string.h>
#include <assert.h>

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

extern PawnTable PTable;

const int PawnValue[PHASE_NB] = {  87,  96};

const int PawnIsolated[PHASE_NB] = {  -5, -10};

const int PawnStacked[PHASE_NB] = { -12, -18};

const int PawnBackwards[2][PHASE_NB] = { {  -4,  -7}, {  -6,  -9} };

const int PawnConnected32[32][PHASE_NB] = {
    {   0,   0}, {   0,   0}, {   0,   0}, {   0,   0},
    {   1,  -2}, {   3,   0}, {   3,   1}, {   0,   6},
    {   8,   2}, {   5,   2}, {   2,   3}, {   4,   1},
    {   3,   2}, {   4,   5}, {   3,   3}, {   8,   5},
    {   6,  11}, {  11,  14}, {  10,  14}, {  26,  21},
    {  15,  24}, {  25,  29}, {  46,  30}, {  39,  31},
    {  42,  27}, {  46,  28}, {  56,  39}, {  52,  38},
    {   0,   0}, {   0,   0}, {   0,   0}, {   0,   0},
};


const int KnightValue[PHASE_NB] = { 319, 306};

const int KnightOutpost[2][PHASE_NB] = { {  13,  -8}, {  29,  11} };

const int KnightMobility[9][PHASE_NB] = {
    { -73, -64}, { -28, -33}, {  -9,  -6},
    {   6,   3}, {  11,   7}, {  13,  17},
    {  22,  17}, {  29,  21}, {  31,   9},
};


const int BishopValue[PHASE_NB] = { 317, 307};

const int BishopWings[PHASE_NB] = {  -2,  14};

const int BishopPair[PHASE_NB] = {  41,  54};

const int BishopOutpost[2][PHASE_NB] = { {  11,  -7}, {  27,  -3} };

const int BishopMobility[14][PHASE_NB] = {
    { -38, -46}, { -39, -30}, { -17, -19}, {   0,  -4},
    {  11,   6}, {  20,  14}, {  25,  20}, {  28,  22},
    {  30,  25}, {  28,  24}, {  28,  27}, {  32,  21},
    {  37,  30}, {  22,  13},
};


const int RookValue[PHASE_NB] = { 466, 490};

const int RookFile[2][PHASE_NB] = { {  10,  11}, {  27,   8} };

const int RookOnSeventh[PHASE_NB] = {   4,  12};

const int RookMobility[15][PHASE_NB] = {
    {-112, -86}, { -57, -46}, {  -8, -24}, {  -6,  -8},
    {  -3,   4}, {   1,  15}, {   0,  24}, {   2,  28},
    {   8,  35}, {  14,  37}, {  18,  40}, {  21,  43},
    {  21,  45}, {  22,  44}, {  17,  41},
};


const int QueenValue[PHASE_NB] = { 866, 894};

const int QueenMobility[28][PHASE_NB] = {
    {-121, -50}, { -68,-187}, { -84, -99}, { -48, -53},
    { -18, -54}, { -12, -20}, {  -2, -18}, {   0, -15},
    {   2, -14}, {   4,  -9}, {   7,   0}, {   9,   4},
    {  11,  11}, {  13,  16}, {  14,  21}, {  15,  30},
    {  15,  34}, {  15,  36}, {  17,  37}, {  19,  42},
    {  21,  44}, {  30,  45}, {  31,  47}, {  35,  45},
    {  38,  47}, {  31,  39}, {  48,  53}, {  30,  45},
};


const int PassedPawn[2][2][RANK_NB][PHASE_NB] = {
  {{{   0,   0}, {  -7,  -5}, { -17,   3}, { -12,   8}, {  16,  28}, {  47,  37}, {  62,  49}, {   0,   0}},
   {{   0,   0}, {  -4,   0}, { -13,   8}, {  -5,  21}, {  16,  37}, {  54,  52}, {  79,  77}, {   0,   0}}},
  {{{   0,   0}, {   0,  10}, {  -7,   7}, {  -1,  22}, {  25,  34}, {  71,  63}, { 121, 137}, {   0,   0}},
   {{   0,   0}, {   3,   5}, {  -5,   8}, {   0,  33}, {  23,  75}, {  89, 166}, { 223, 312}, {   0,   0}}}

};

const int SafetyTable[100] = { // Taken from CPW / Stockfish
       0,   0,   1,   2,   3,   5,   7,   9,  12,  15,
      18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
      68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
     140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
     260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
     377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
     494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
     500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
     500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
     500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

const int KingValue[PHASE_NB] = { 100, 100};

const int NoneValue[PHASE_NB] = {   0,   0};

const int Tempo[COLOUR_NB][PHASE_NB] = { {  20,  10}, { -20, -10} };

const int* PieceValues[8] = {
    PawnValue, KnightValue, BishopValue, RookValue,
    QueenValue, KingValue, NoneValue, NoneValue
};

int evaluateBoard(Board * board){
    
    EvalInfo ei;
    int mg, eg, phase, eval;
    
    // evaluateDraws handles obvious drawn positions
    if (evaluateDraws(board)) return 0;
    
    // Setup and perform the evaluation of all pieces
    initializeEvalInfo(&ei, board);
    evaluatePieces(&ei, board);
        
    // Combine evaluation terms for the mid game
    mg = board->midgame + ei.midgame[WHITE] - ei.midgame[BLACK]
       + ei.pawnMidgame[WHITE] - ei.pawnMidgame[BLACK] + Tempo[board->turn][MG];
       
    // Combine evaluation terms for the end game
    eg = board->endgame + ei.endgame[WHITE] - ei.endgame[BLACK]
       + ei.pawnEndgame[WHITE] - ei.pawnEndgame[BLACK] + Tempo[board->turn][EG];
       
    // Calcuate the game phase based on remaining material (Fruit Method)
    phase = 24 - popcount(board->pieces[QUEEN]) * 4
               - popcount(board->pieces[ROOK]) * 2
               - popcount(board->pieces[KNIGHT] | board->pieces[BISHOP]);
    phase = (phase * 256 + 12) / 24;
          
    // Compute the interpolated evaluation
    eval  = (mg * (256 - phase) + eg * phase) / 256;
    
    // Return the evaluation relative to the side to move
    return board->turn == WHITE ? eval : -eval;
}

int evaluateDraws(Board * board){
    
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

void evaluatePieces(EvalInfo * ei, Board * board){
    
    int pmg, peg;
    
    evaluatePawns(ei, board, WHITE);
    evaluatePawns(ei, board, BLACK);
    
    evaluateKnights(ei, board, WHITE);
    evaluateKnights(ei, board, BLACK);
    
    evaluateBishops(ei, board, WHITE);
    evaluateBishops(ei, board, BLACK);
    
    evaluateRooks(ei, board, WHITE);
    evaluateRooks(ei, board, BLACK);
    
    evaluateQueens(ei, board, WHITE);
    evaluateQueens(ei, board, BLACK);
    
    evaluateKings(ei, board, WHITE);
    evaluateKings(ei, board, BLACK);
    
    // Save a Pawn Eval Entry if we did not find one. We do not make use
    // of the Pawn Evaluation Table when we are doing texel tuning
    if (ei->pentry == NULL){
        
        if (!TEXEL){
            pmg = ei->pawnMidgame[WHITE] - ei->pawnMidgame[BLACK];
            peg = ei->pawnEndgame[WHITE] - ei->pawnEndgame[BLACK];
            storePawnEntry(&PTable, board->phash, ei->passedPawns, pmg, peg);
        }
        
        evaluatePassedPawns(ei, board, WHITE);
        evaluatePassedPawns(ei, board, BLACK);
    }
    
    // Otherwise, grab the evaluated pawn values from the Entry
    else {
        ei->pawnMidgame[WHITE] = ei->pentry->mg;
        ei->pawnEndgame[WHITE] = ei->pentry->eg;
        ei->passedPawns = ei->pentry->passed;
        evaluatePassedPawns(ei, board, WHITE);
        evaluatePassedPawns(ei, board, BLACK);
    }
}

void evaluatePawns(EvalInfo * ei, Board * board, int colour){
    
    const int forward = (colour == WHITE) ? 8 : -8;
    
    int sq, semi;
    uint64_t pawns, myPawns, tempPawns, enemyPawns, attacks;
    
    // Update the attacks array with the pawn attacks. We will use this to
    // determine whether or not passed pawns may advance safely later on.
    attacks = ei->pawnAttacks[colour] & ei->kingAreas[!colour];
    ei->attacked[colour] |= ei->pawnAttacks[colour];
    
    // Update the attack counts and attacker counts for pawns for use in
    // the king safety calculation. We just do this for the pawns as a whole,
    // and not individually, to save time, despite the loss in accuracy.
    if (attacks != 0ull){
        ei->attackCounts[colour] += 2 * popcount(attacks);
        ei->attackerCounts[colour] += 1;
    }
    
    // The pawn table holds the rest of the eval information we will calculate
    if (ei->pentry != NULL) return;
    
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
            ei->pawnMidgame[colour] += PawnIsolated[MG];
            ei->pawnEndgame[colour] += PawnIsolated[EG];
            if (TRACE) T.pawnIsolated[colour]++;
        }
        
        // Apply a penalty if the pawn is stacked
        if (Files[File(sq)] & tempPawns){
            ei->pawnMidgame[colour] += PawnStacked[MG];
            ei->pawnEndgame[colour] += PawnStacked[EG];
            if (TRACE) T.pawnStacked[colour]++;
        }
        
        // Apply a penalty if the pawn is backward
        if (   !(PassedPawnMasks[!colour][sq] & myPawns)
            &&  (ei->pawnAttacks[!colour] & (1ull << (sq + forward)))){
                
            semi = !(Files[File(sq)] & enemyPawns);
            
            ei->pawnMidgame[colour] += PawnBackwards[semi][MG];
            ei->pawnEndgame[colour] += PawnBackwards[semi][EG];
            if (TRACE) T.pawnBackwards[colour][semi]++;
        }
        
        // Apply a bonus if the pawn is connected
        if (PawnConnectedMasks[colour][sq] & myPawns){
            ei->pawnMidgame[colour] += PawnConnected32[relativeSquare32(sq, colour)][MG];
            ei->pawnEndgame[colour] += PawnConnected32[relativeSquare32(sq, colour)][EG];
            if (TRACE) T.pawnConnected[colour][sq]++;
        }
    }
}

void evaluateKnights(EvalInfo * ei, Board * board, int colour){
    
    int sq, defended, mobilityCount;
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
        ei->attacked[colour] |= attacks;
        
        // Apply a bonus if the knight is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the knight.
        if (    (OutpostRanks[colour] & (1ull << sq))
            && !(OutpostSquareMasks[colour][sq] & enemyPawns)){
                
            defended = !!(ei->pawnAttacks[colour] & (1ull << sq));
            
            ei->midgame[colour] += KnightOutpost[defended][MG];
            ei->endgame[colour] += KnightOutpost[defended][EG];
            if (TRACE) T.knightOutpost[colour][defended]++;
        }
        
        // Apply a bonus (or penalty) based on the mobility of the knight
        mobilityCount = popcount((ei->mobilityAreas[colour] & attacks));
        ei->midgame[colour] += KnightMobility[mobilityCount][MG];
        ei->endgame[colour] += KnightMobility[mobilityCount][EG];
        if (TRACE) T.knightMobility[colour][mobilityCount]++;
        
        // Update the attack and attacker counts for the
        // knight for use in the king safety calculation.
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->attackCounts[colour] += 2 * popcount(attacks);
            ei->attackerCounts[colour] += 1;
        }
    }
}

void evaluateBishops(EvalInfo * ei, Board * board, int colour){
    
    int sq, defended, mobilityCount;
    uint64_t tempBishops, myPawns, enemyPawns, attacks;
    
    tempBishops = board->pieces[BISHOP] & board->colours[colour];
    myPawns = board->pieces[PAWN] & board->colours[colour];
    enemyPawns = board->pieces[PAWN] & board->colours[!colour];
    
    // Apply a bonus for having pawn wings and a bishop
    if (tempBishops && (myPawns & LEFT_WING) && (myPawns & RIGHT_WING)){
        ei->midgame[colour] += BishopWings[MG];
        ei->endgame[colour] += BishopWings[EG];
        if (TRACE) T.bishopWings[colour]++;
    }
    
    // Apply a bonus for having a pair of bishops
    if ((tempBishops & WHITE_SQUARES) && (tempBishops & BLACK_SQUARES)){
        ei->midgame[colour] += BishopPair[MG];
        ei->endgame[colour] += BishopPair[EG];
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
        ei->attacked[colour] |= attacks;
        
        // Apply a bonus if the bishop is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the bishop.
        if (    (OutpostRanks[colour] & (1ull << sq))
            && !(OutpostSquareMasks[colour][sq] & enemyPawns)){
                
            defended = !!(ei->pawnAttacks[colour] & (1ull << sq));
            
            ei->midgame[colour] += BishopOutpost[defended][MG];
            ei->endgame[colour] += BishopOutpost[defended][EG];
            if (TRACE) T.bishopOutpost[colour][defended]++;
        }
        
        // Apply a bonus (or penalty) based on the mobility of the bishop
        mobilityCount = popcount((ei->mobilityAreas[colour] & attacks));
        ei->midgame[colour] += BishopMobility[mobilityCount][MG];
        ei->endgame[colour] += BishopMobility[mobilityCount][EG];
        if (TRACE) T.bishopMobility[colour][mobilityCount]++;
        
        // Update the attack and attacker counts for the
        // bishop for use in the king safety calculation.
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->attackCounts[colour] += 2 * popcount(attacks);
            ei->attackerCounts[colour] += 1;
        }
    }
}

void evaluateRooks(EvalInfo * ei, Board * board, int colour){
    
    int sq, open, mobilityCount;
    uint64_t tempRooks, myPawns, enemyPawns, attacks;
    
    tempRooks = board->pieces[ROOK] & board->colours[colour];
    myPawns = board->pieces[PAWN] & board->colours[colour];
    enemyPawns = board->pieces[PAWN] & board->colours[!colour];
    
    // Evaluate each rook
    while (tempRooks){
        
        // Pop off the next rook
        sq = poplsb(&tempRooks);
        
        if (TRACE) T.rookCounts[colour]++;
        if (TRACE) T.rookPSQT[colour][sq]++;
        
        // Update the attacks array with the rooks attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = rookAttacks(sq, ei->occupiedMinusRooks[colour], ~0ull);
        ei->attacked[colour] |= attacks;
        
        // Rook is on a semi-open file if there are no
        // pawns of the Rook's colour on the file. If
        // there are no pawns at all, it is an open file
        if (!(myPawns & Files[File(sq)])){
            open = !(enemyPawns & Files[File(sq)]);
            ei->midgame[colour] += RookFile[open][MG];
            ei->endgame[colour] += RookFile[open][EG];
            if (TRACE) T.rookFile[colour][open]++;
            
        }
        
        // Rook gains a bonus for being located
        // on seventh rank relative to its colour
        if (Rank(sq) == (colour == BLACK ? 1 : 6)){
            ei->midgame[colour] += RookOnSeventh[MG];
            ei->endgame[colour] += RookOnSeventh[EG];
            if (TRACE) T.rookOnSeventh[colour]++;
        }
        
        // Apply a bonus (or penalty) based on the mobility of the rook
        mobilityCount = popcount((ei->mobilityAreas[colour] & attacks));
        ei->midgame[colour] += RookMobility[mobilityCount][MG];
        ei->endgame[colour] += RookMobility[mobilityCount][EG];
        if (TRACE) T.rookMobility[colour][mobilityCount]++;
        
        // Update the attack and attacker counts for the
        // rook for use in the king safety calculation.
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->attackCounts[colour] += 3 * popcount(attacks);
            ei->attackerCounts[colour] += 1;
        }
    }
}

void evaluateQueens(EvalInfo * ei, Board * board, int colour){
    
    int sq, mobilityCount;
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
        ei->attacked[colour] |= attacks;
            
        // Apply a bonus (or penalty) based on the mobility of the queen
        mobilityCount = popcount((ei->mobilityAreas[colour] & attacks));
        ei->midgame[colour] += QueenMobility[mobilityCount][MG];
        ei->endgame[colour] += QueenMobility[mobilityCount][EG];
        if (TRACE) T.queenMobility[colour][mobilityCount]++;
        
        // Update the attack and attacker counts for the
        // queen for use in the king safety calculation.
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->attackCounts[colour] += 4 * popcount(attacks);
            ei->attackerCounts[colour] += 1;
        }
    }
}

void evaluateKings(EvalInfo * ei, Board * board, int colour){
    
    int attackCounts;
    
    if (TRACE) T.kingPSQT[colour][getlsb(board->colours[colour] & board->pieces[KING])]++;
    
    // If we have two or more threats to our king area, we will apply a penalty
    // based on the number of squares attacked, and the strength of the attackers
    if (ei->attackerCounts[!colour] >= 2){
        
        // Cap our attackCounts at 99 (SafetyTable has 100 slots)
        attackCounts = ei->attackCounts[!colour];
        attackCounts = attackCounts >= 100 ? 99 : attackCounts;
        
        // Scale down attack count if there are no enemy queens
        if (!(board->colours[!colour] & board->pieces[QUEEN]))
            attackCounts *= .25;
    
        ei->midgame[colour] -= SafetyTable[attackCounts];
        ei->endgame[colour] -= SafetyTable[attackCounts];
    }
}

void evaluatePassedPawns(EvalInfo * ei, Board * board, int colour){
    
    int sq, rank, canAdvance, safeAdvance;
    uint64_t tempPawns, destination, notEmpty;
    
    tempPawns = board->colours[colour] & ei->passedPawns;
    notEmpty = board->colours[WHITE] | board->colours[BLACK];
    
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
        
        ei->midgame[colour] += PassedPawn[canAdvance][safeAdvance][rank][MG];
        ei->endgame[colour] += PassedPawn[canAdvance][safeAdvance][rank][EG];
        if (TRACE) T.passedPawn[colour][canAdvance][safeAdvance][rank]++;
    }
}

void initializeEvalInfo(EvalInfo * ei, Board * board){
    
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
    
    ei->blockedPawns[WHITE] = ((whitePawns << 8) & black) >> 8;
    ei->blockedPawns[BLACK] = ((blackPawns >> 8) & white) << 8,
    
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
    
    ei->midgame[WHITE] = ei->midgame[BLACK] = 0;
    ei->endgame[WHITE] = ei->endgame[BLACK] = 0;
    
    ei->pawnMidgame[WHITE] = ei->pawnMidgame[BLACK] = 0;
    ei->pawnEndgame[WHITE] = ei->pawnEndgame[BLACK] = 0;
    
    if (TEXEL) ei->pentry = NULL;
    else       ei->pentry = getPawnEntry(&PTable, board->phash);
}
