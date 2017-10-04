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

#include "castle.h"
#include "movegen.h"
#include "magics.h"
#include "masks.h"
#include "types.h"
#include "bitboards.h"
#include "bitutils.h"
#include "transposition.h"
#include "evaluate.h"
#include "piece.h"

extern PawnTable PTable;

const int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

const int KingHasCastled = 25;

const int KingCanCastle = 10;

const int RookOpenFile[2][PHASE_NB] = {{12, 12}, {35, 20}};

const int RookOnSeventh[PHASE_NB] = {10, 15};

const int PawnStacked[PHASE_NB] = {10, 20};

const int PawnIsolated[PHASE_NB] = {10, 20};

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

const int PawnConnected[COLOUR_NB][SQUARE_NB] = {
    { 0, 0, 0, 0, 0, 0, 0, 0,
      2, 2, 2, 3, 3, 2, 2, 2,
      4, 4, 5, 6, 6, 5, 4, 4,
      7, 8,10,12,12,10, 8, 7,
     11,14,17,21,21,17,14,11,
     16,21,25,33,33,25,21,16,
     32,42,50,55,55,50,42,32,
      0, 0, 0, 0, 0, 0, 0, 0,},
      
    { 0, 0, 0, 0, 0, 0, 0, 0,
     32,42,50,55,55,50,42,32,
     16,21,25,33,33,25,21,16,
     11,14,17,21,21,17,14,11,
      7, 8,10,12,12,10, 8, 7,
      4, 4, 5, 6, 6, 5, 4, 4,
      2, 2, 2, 3, 3, 2, 2, 2,
      0, 0, 0, 0, 0, 0, 0, 0,}
};

const int KnightOutpostValues[PHASE_NB][2] = {{20, 40}, {10, 20}};
const int BishopOutpostValues[PHASE_NB][2] = {{15, 30}, { 3,  5}};

const int PawnPassed[2][2][RANK_NB][PHASE_NB] = {
  {{{   0,   0}, {   9,  11}, {   9,  12}, {  19,  24}, {  34,  41}, {  54,  60}, {  65,  83}, {   0,   0}},
   {{   0,   0}, {   8,   9}, {   7,  11}, {  18,  25}, {  33,  40}, {  55,  59}, {  68,  82}, {   0,   0}}},
  {{{   0,   0}, {   8,  18}, {  10,  16}, {  22,  32}, {  42,  49}, {  77,  79}, { 103, 136}, {   0,   0}},
   {{   0,   0}, {  14,  12}, {  11,  15}, {  24,  38}, {  53,  78}, { 110, 150}, { 207, 271}, {   0,   0}}},
};


const int KnightMobility[PHASE_NB][9] = {
    {-30, -25, -10,   0,  10,  18,  26,  34,  42},
    {-30, -25,   0,   9,  15,  21,  28,  35,  36}
};

const int BishopMobility[PHASE_NB][14] = {
    {-30, -20, -15,   0,  15,  21,  26,  31,  34,  36,  37,  38,  38,  38},
    {-30, -20, -15,   0,  15,  21,  26,  31,  34,  36,  37,  38,  38,  38},
};

const int RookMobility[PHASE_NB][15] = {
    {-30, -25, -10,  -5,  -3,  -1,   6,  11,  15,  19,  23,  25,  26,  27,  27},
    {-35, -20, -10,   0,  10,  19,  27,  33,  39,  41,  43,  45,  47,  48,  48}
};

const int QueenMobility[PHASE_NB][28] = {
    {-50, -40, -20,   0,   2,   4,   6,
       8,  11,  15,  19,  20,  21,  22,
      24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24},
    
    {-50, -40, -20, -10,   0,   4,   8,
      12,  15,  18,  21,  24,  27,  30,
      35,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43}
};

const int BishopHasWings[PHASE_NB] = {13, 36};

const int BishopPair[PHASE_NB] = {46, 64};

const int Tempo[COLOUR_NB][PHASE_NB] = {{5, 7}, {-5, -7}};


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
    
    // Save a Pawn Eval Entry if we didin't find one
    if (ei->pentry == NULL){
        pmg = ei->pawnMidgame[WHITE] - ei->pawnMidgame[BLACK];
        peg = ei->pawnEndgame[WHITE] - ei->pawnEndgame[BLACK];
        storePawnEntry(&PTable, board->phash, ei->passedPawns, pmg, peg);
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
    
    int sq;
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
        sq = getLSB(tempPawns);
        tempPawns ^= 1ull << sq;
        
        // Save the fact that this pawn is passed
        if (!(PassedPawnMasks[colour][sq] & enemyPawns))
            ei->passedPawns |= (1ull << sq);
        
        // Apply a penalty if the pawn is isolated
        if (!(IsolatedPawnMasks[sq] & tempPawns)){
            ei->pawnMidgame[colour] -= PawnIsolated[MG];
            ei->pawnEndgame[colour] -= PawnIsolated[EG];
        }
        
        // Apply a penalty if the pawn is stacked
        if (Files[File(sq)] & tempPawns){
            ei->pawnMidgame[colour] -= PawnStacked[MG];
            ei->pawnEndgame[colour] -= PawnStacked[EG];
        }
        
        // Apply a bonus if the pawn is connected
        if (PawnConnectedMasks[colour][sq] & myPawns){
            ei->pawnMidgame[colour] += PawnConnected[colour][sq];
            ei->pawnEndgame[colour] += PawnConnected[colour][sq];
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
        sq = getLSB(tempKnights);
        tempKnights ^= (1ull << sq);
        
        // Update the attacks array with the knight attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = KnightAttacks(sq, ~0ull);
        ei->attacked[colour] |= attacks;
        
        // Apply a bonus if the knight is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the knight.
        if (    (OutpostRanks[colour] & (1ull << sq))
            && !(OutpostSquareMasks[colour][sq] & enemyPawns)){
                
            defended = !!(ei->pawnAttacks[colour] & (1ull << sq));
            
            ei->midgame[colour] += KnightOutpostValues[MG][defended];
            ei->endgame[colour] += KnightOutpostValues[EG][defended];
        }
        
        // Apply a bonus (or penalty) based on the mobility of the knight
        mobilityCount = popcount((ei->mobilityAreas[colour] & attacks));
        ei->midgame[colour] += KnightMobility[MG][mobilityCount];
        ei->endgame[colour] += KnightMobility[EG][mobilityCount];
        
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
        ei->midgame[colour] += BishopHasWings[MG];
        ei->endgame[colour] += BishopHasWings[EG];
    }
    
    // Apply a bonus for having a pair of bishops
    if ((tempBishops & WHITE_SQUARES) && (tempBishops & BLACK_SQUARES)){
        ei->midgame[colour] += BishopPair[MG];
        ei->endgame[colour] += BishopPair[EG];
    }
    
    // Evaluate each bishop
    while (tempBishops){
        
        // Pop off the next Bishop
        sq = getLSB(tempBishops);
        tempBishops ^= (1ull << sq);
        
        // Update the attacks array with the bishop attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = BishopAttacks(sq, ei->occupiedMinusBishops[colour], ~0ull);
        ei->attacked[colour] |= attacks;
        
        // Apply a bonus if the bishop is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the bishop.
        if (    (OutpostRanks[colour] & (1ull << sq))
            && !(OutpostSquareMasks[colour][sq] & enemyPawns)){
                
            defended = !!(ei->pawnAttacks[colour] & (1ull << sq));
            
            ei->midgame[colour] += BishopOutpostValues[MG][defended];
            ei->endgame[colour] += BishopOutpostValues[EG][defended];
        }
        
        // Apply a bonus (or penalty) based on the mobility of the bishop
        mobilityCount = popcount((ei->mobilityAreas[colour] & attacks));
        ei->midgame[colour] += BishopMobility[MG][mobilityCount];
        ei->endgame[colour] += BishopMobility[EG][mobilityCount];
        
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
        sq = getLSB(tempRooks);
        tempRooks ^= (1ull << sq);
        
        // Update the attacks array with the rooks attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = RookAttacks(sq, ei->occupiedMinusRooks[colour], ~0ull);
        ei->attacked[colour] |= attacks;
        
        // Rook is on a semi-open file if there are no
        // pawns of the Rook's colour on the file. If
        // there are no pawns at all, it is an open file
        if (!(myPawns & Files[File(sq)])){
            open = !(enemyPawns & Files[File(sq)]);
            ei->midgame[colour] += RookOpenFile[open][MG];
            ei->endgame[colour] += RookOpenFile[open][EG];
        }
        
        // Rook gains a bonus for being located
        // on seventh rank relative to its colour
        if (Rank(sq) == (colour == BLACK ? 1 : 6)){
            ei->midgame[colour] += RookOnSeventh[MG];
            ei->endgame[colour] += RookOnSeventh[EG];
        }
        
        // Apply a bonus (or penalty) based on the mobility of the rook
        mobilityCount = popcount((ei->mobilityAreas[colour] & attacks));
        ei->midgame[colour] += RookMobility[MG][mobilityCount];
        ei->endgame[colour] += RookMobility[EG][mobilityCount];
        
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
        sq = getLSB(tempQueens);
        tempQueens ^= (1ull << sq);
        
        // Update the attacks array with the rooks attacks. We will use this to
        // determine whether or not passed pawns may advance safely later on.
        attacks = RookAttacks(sq, ei->occupiedMinusRooks[colour], ~0ull)
                | BishopAttacks(sq, ei->occupiedMinusBishops[colour], ~0ull);
        ei->attacked[colour] |= attacks;
            
        // Apply a bonus (or penalty) based on the mobility of the queen
        mobilityCount = popcount((ei->mobilityAreas[colour] & attacks));
        ei->midgame[colour] += QueenMobility[MG][mobilityCount];
        ei->endgame[colour] += QueenMobility[EG][mobilityCount];
        
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
    
    // Apply a bonus if the king has castled since the root
    if (board->hasCastled[colour]){
        ei->midgame[colour] += KingHasCastled;
        ei->endgame[colour] += KingHasCastled;
    }
    
    // Apply a bonus if the king still has castling rights
    else if (board->castleRights & (3 << (2 * colour))){
        ei->midgame[colour] += KingCanCastle;
        ei->endgame[colour] += KingCanCastle;
    }
    
    // If we have two or more threats to our king area, we will apply a penalty
    // based on the number of squares attacked, and the strength of the attackers
    if (ei->attackerCounts[!colour] >= 2){
        
        // Cap our attackCounts at 99 (SafetyTable has 100 slots)
        attackCounts = ei->attackCounts[!colour];
        attackCounts = attackCounts >= 100 ? 99 : attackCounts;
        
        // Scale down attack count if there are no enemy queens
        if (!(board->colours[!colour] & board->pieces[QUEEN]))
            attackCounts *= .5;
        
        // Scale down attack count if there are no enemy rooks
        if (!(board->colours[!colour] & board->pieces[ROOK]))
            attackCounts *= .8;
    
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
        sq = getLSB(tempPawns);
        tempPawns ^= (1ull << sq);
        
        // Determine the releative rank
        rank = (colour == BLACK) ? (7 - Rank(sq)) : Rank(sq);
        
        // Determine where the pawn would advance to
        destination = (colour == BLACK) ? ((1ull << sq) >> 8): ((1ull << sq) << 8);
            
        // Destination does not have any pieces on it
        canAdvance = !(destination & notEmpty);
        
        // Destination is not attacked by the opponent
        safeAdvance = !(destination & ei->attacked[!colour]);
        
        ei->midgame[colour] += PawnPassed[canAdvance][safeAdvance][rank][MG];
        ei->endgame[colour] += PawnPassed[canAdvance][safeAdvance][rank][EG];
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
    
    int wKingSq = getLSB((white & kings));
    int bKingSq = getLSB((black & kings));
    
    ei->pawnAttacks[WHITE] = ((whitePawns << 9) & ~FILE_A) | ((whitePawns << 7) & ~FILE_H);
    ei->pawnAttacks[BLACK] = ((blackPawns >> 9) & ~FILE_H) | ((blackPawns >> 7) & ~FILE_A);
    
    ei->blockedPawns[WHITE] = ((whitePawns << 8) & black) >> 8;
    ei->blockedPawns[BLACK] = ((blackPawns >> 8) & white) << 8,
    
    ei->kingAreas[WHITE] = KingMap[wKingSq] | (1ull << wKingSq) | (KingMap[wKingSq] << 8);
    ei->kingAreas[BLACK] = KingMap[bKingSq] | (1ull << bKingSq) | (KingMap[bKingSq] >> 8);
    
    ei->mobilityAreas[WHITE] = ~(ei->pawnAttacks[BLACK] | (white & kings) | ei->blockedPawns[WHITE]);
    ei->mobilityAreas[BLACK] = ~(ei->pawnAttacks[WHITE] | (black & kings) | ei->blockedPawns[BLACK]);
    
    ei->attacked[WHITE] = KingAttacks(wKingSq, ~0ull);
    ei->attacked[BLACK] = KingAttacks(bKingSq, ~0ull);
    
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
    
    ei->pentry = getPawnEntry(&PTable, board->phash);
}