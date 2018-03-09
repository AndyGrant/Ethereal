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

#include <assert.h>
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

const int PawnValue[PHASE_NB]   = {  95, 114};

const int KnightValue[PHASE_NB] = { 435, 370};

const int BishopValue[PHASE_NB] = { 438, 390};

const int RookValue[PHASE_NB]   = { 587, 672};

const int QueenValue[PHASE_NB]  = {1186,1233};

const int KingValue[PHASE_NB]   = { 156, 156};

const int NoneValue[PHASE_NB]   = {   0,   0};


const int PawnIsolated[PHASE_NB] = {  -3,  -7};

const int PawnStacked[PHASE_NB] = { -12, -29};

const int PawnBackwards[2][PHASE_NB] = { {   3,  -4}, { -15, -10} };

const int PawnConnected32[32][PHASE_NB] = {
    {   0,   0}, {   0,   0}, {   0,   0}, {   0,   0},
    {   0, -15}, {   6,   1}, {   3,  -3}, {   4,  17},
    {   7,   0}, {  20,   0}, {  14,   7}, {  15,  23},
    {   7,   0}, {  18,   4}, {  17,  10}, {  18,  17},
    {   6,  15}, {  17,  17}, {  18,  23}, {  39,  25},
    {  28,  53}, {  34,  59}, { 101,  59}, {  70,  76},
    { 231,  40}, { 168,  26}, { 178,  37}, { 181,  96},
    {   0,   0}, {   0,   0}, {   0,   0}, {   0,   0},
};

const int KnightAttackedByPawn[PHASE_NB] = { -46, -29};

const int KnightOutpost[2][PHASE_NB] = { {  18, -31}, {  40,  12} };

const int KnightMobility[9][PHASE_NB] = {
    { -84, -95}, { -37, -76}, { -10, -37},
    {  -4,  -9}, {   3, -12}, {   9,   1},
    {  18,  -1}, {  31,   0}, {  46, -25},
};

const int BishopPair[PHASE_NB] = {  42,  65};

const int BishopAttackedByPawn[PHASE_NB] = { -50, -28};

const int BishopOutpost[2][PHASE_NB] = { {  20, -14}, {  54,  -9} };

const int BishopMobility[14][PHASE_NB] = {
    { -50,-110}, { -42, -56}, { -18, -42}, {  -4, -20},
    {   4,  -6}, {  15,   0}, {  21,   9}, {  28,   4},
    {  29,  10}, {  35,   6}, {  42,   4}, {  56,  -6},
    {  31,   0}, {  29, -18},
};

const int RookFile[2][PHASE_NB] = { {   9,   3}, {  42,  -7} };

const int RookOnSeventh[PHASE_NB] = {   0,  20};

const int RookMobility[15][PHASE_NB] = {
    {-123, -65}, { -60,-112}, { -15, -62}, { -10, -26},
    {  -9,  -3}, {  -9,  15}, {  -7,  26}, {  -3,  31},
    {   0,  37}, {   6,  34}, {   9,  40}, {  20,  46},
    {  18,  48}, {  23,  45}, {  18,  42},
};

const int QueenChecked[PHASE_NB] = { -31, -31};

const int QueenCheckedByPawn[PHASE_NB] = { -42, -42};

const int QueenMobility[28][PHASE_NB] = {
    { -57,-243}, { -37,-213}, { -26,-162}, { -29,-142},
    { -21,-106}, { -23, -54}, { -21, -84}, { -17, -84},
    { -15, -54}, { -10, -50}, { -10, -26}, {  -6, -28},
    {  -3, -15}, {  -4, -12}, {   1, -10}, {   0,   3},
    {   0,  12}, {  -1,  12}, {  10,  23}, {   0,  25},
    {   4,  28}, {   9,  26}, {  23,  15}, {  34,  17},
    {  45,  25}, {  35,   0}, {   3,   6}, {  29,  17},
};

const int KingDefenders[12][PHASE_NB] = {
    { -34,  -3}, { -26,   4}, {   1,   1}, {  10,  -1},
    {  25,   0}, {  34,   3}, {  34,   3}, {  34,   3},
    {  12,   6}, {  12,   6}, {  12,   6}, {  12,   6},
};

const int KingShelter[2][FILE_NB][RANK_NB][PHASE_NB] = {
  {{{ -14,  17}, {   0,  -7}, {   9,   6}, {  14,   7}, { -14,  -6}, { -15,  -6}, { -51, -45}, { -29,   1}},
   {{  -3,   6}, {  14,  -3}, {  18,  -6}, {   0,  -9}, { -45,  -4}, { -92,  82}, {   4,  40}, { -34,   0}},
   {{  18,  14}, {   7,   1}, { -14,   1}, { -20,   1}, { -34,   0}, {  21, -10}, { -54,  34}, { -15,   0}},
   {{  20,  21}, {  15,   0}, {  -4,  -7}, {  15, -10}, {  17, -25}, { -20, -35}, { -96,   7}, {  -1,   0}},
   {{   4,  17}, {   4,  -1}, { -23,   1}, { -14,   0}, { -42, -14}, {  -9, -20}, { -26,  -1}, { -12,   0}},
   {{  23,   1}, {  17,  -4}, { -15,  -1}, {  -3, -18}, {   1, -29}, {  21, -39}, {  98, -23}, { -15,   1}},
   {{  21,   0}, {   4,  -9}, { -21,  -9}, { -18, -12}, { -23, -14}, { -62, -10}, { -84,  14}, { -34,   9}},
   {{  -7,  -3}, {  -1,  -7}, {   6,   0}, {   1,   3}, { -15,   9}, { -14,  26}, { -56,  60}, { -17,  14}}},
  {{{   0,   0}, { -18, -20}, { -15, -21}, { -82,   6}, {   7, -20}, { -76,  12}, { -86,  37}, { -60,   7}},
   {{   0,   0}, {  10,  -4}, {   1,  -3}, {   0,   0}, {   3, -20}, { -62,  70}, { -79,  31}, { -42,   3}},
   {{   0,   0}, {  23,   3}, {   4,  -4}, {  18, -23}, {   1,  -3}, { -42,  50}, { -64, -62}, { -17,   0}},
   {{   0,   0}, {   4,  10}, {   1,  12}, { -18,  -4}, { -25, -10}, { -96,  -9}, {  35,   6}, { -21,  -1}},
   {{   0,   0}, {   7,   3}, {   9,  -7}, {  17,  -7}, {   7, -14}, { -53,  20}, { -20, -90}, {  -6,  -3}},
   {{   0,   0}, {  10,   0}, {   0,  -4}, { -28, -12}, {   0, -29}, { -54,  -7}, { -62,   4}, { -26,   0}},
   {{   0,   0}, {  12,  -1}, {   1,   0}, { -14,  -6}, { -29, -18}, {  -1, -12}, { -98, -31}, { -43,  12}},
   {{   0,   0}, {   7, -25}, {  10, -12}, { -26,   4}, { -39,   0}, {   1, -25}, { -88, -25}, { -46,  17}}},
};

const int PassedPawn[2][2][RANK_NB][PHASE_NB] = {
  {{{   0,   0}, { -37, -31}, { -26,   7}, { -14,  -3}, {  28,   0}, {  78,  -3}, { 168,  31}, {   0,   0}},
   {{   0,   0}, {  -3,  -1}, { -15,  21}, { -20,  32}, {   6,  40}, {  73,  54}, { 192, 123}, {   0,   0}}},
  {{{   0,   0}, {  -7,  10}, { -14,   3}, { -14,  25}, {  26,  29}, {  96,  60}, { 240, 143}, {   0,   0}},
   {{   0,   0}, {  -3,   7}, { -12,  15}, { -25,  48}, { -20, 100}, {  25, 189}, { 117, 350}, {   0,   0}}},
};

const int KingSafety[100] = { // Taken from CPW / Stockfish
      0,   0,   1,   3,   4,   7,  10,  14,  18,  23, 
     28,  34,  40,  46,  54,  60,  68,  78,  87,  96, 
    106, 117, 128, 132, 139, 151, 164, 176, 190, 204,
    218, 234, 264, 281, 298, 315, 332, 351, 370, 387, 
    406, 425, 442, 460, 479, 498, 515, 534, 553, 571, 
    589, 607, 626, 643, 662, 681, 700, 717, 735, 754, 
    771, 781, 781, 781, 781, 781, 781, 781, 781, 781, 
    781, 781, 781, 781, 781, 781, 781, 781, 781, 781,
    781, 781, 781, 781, 781, 781, 781, 781, 781, 781,
    781, 781, 781, 781, 781, 781, 781, 781, 781, 781
};

const int Tempo[COLOUR_NB][PHASE_NB] = { {  25,  12}, { -25, -12} };

const int* PieceValues[8] = {
    PawnValue, KnightValue, BishopValue, RookValue,
    QueenValue, KingValue, NoneValue, NoneValue
};

int evaluateBoard(Board* board, EvalInfo* ei, PawnKingTable* pktable){
    
    int mg, eg, phase, eval;
    
    // evaluateDraws handles obvious drawn positions
    ei->positionIsDrawn = evaluateDraws(board);
    if (ei->positionIsDrawn) return 0;
    
    // Setup and perform the evaluation of all pieces
    initializeEvalInfo(ei, board, pktable);
    evaluatePieces(ei, board);
    
    // Store a new PawnKing entry if we did not have one (and are not doing Texel)
    if (ei->pkentry == NULL && !TEXEL){
        mg = ei->pawnKingMidgame[WHITE] - ei->pawnKingMidgame[BLACK];
        eg = ei->pawnKingEndgame[WHITE] - ei->pawnKingEndgame[BLACK];
        storePawnKingEntry(pktable, board->pkhash, ei->passedPawns, mg, eg);
    }
    
    // Otherwise, fetch the PawnKing evaluation (if we are not doing Texel)
    else if (!TEXEL){
        ei->pawnKingMidgame[WHITE] = ei->pkentry->mg;
        ei->pawnKingEndgame[WHITE] = ei->pkentry->eg;
    }
        
    // Combine evaluation terms for the mid game
    mg = board->midgame + ei->midgame[WHITE] - ei->midgame[BLACK]
       + ei->pawnKingMidgame[WHITE] - ei->pawnKingMidgame[BLACK] + Tempo[board->turn][MG];
       
    // Combine evaluation terms for the end game
    eg = board->endgame + ei->endgame[WHITE] - ei->endgame[BLACK]
       + ei->pawnKingEndgame[WHITE] - ei->pawnKingEndgame[BLACK] + Tempo[board->turn][EG];
       
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

void evaluatePieces(EvalInfo* ei, Board* board){
    
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
    
    evaluatePassedPawns(ei, board, WHITE);
    evaluatePassedPawns(ei, board, BLACK);
}

void evaluatePawns(EvalInfo* ei, Board* board, int colour){
    
    const int forward = (colour == WHITE) ? 8 : -8;
    
    int sq, semi;
    uint64_t pawns, myPawns, tempPawns, enemyPawns, attacks;
    
    // Update the attacks array with the pawn attacks. We will use this to
    // determine whether or not passed pawns may advance safely later on.
    attacks = ei->pawnAttacks[colour] & ei->kingAreas[!colour];
    ei->attackedBy2[colour] = ei->attacked[colour] & ei->pawnAttacks[colour];
    ei->attacked[colour] |= ei->pawnAttacks[colour];
    ei->attackedNoQueen[colour] |= attacks;
    
    // Update the attack counts and attacker counts for pawns for use in
    // the king safety calculation. We just do this for the pawns as a whole,
    // and not individually, to save time, despite the loss in accuracy.
    if (attacks != 0ull){
        ei->attackCounts[colour] += popcount(attacks);
        ei->attackerCounts[colour] += 1;
    }
    
    // The pawn table holds the rest of the eval information we will calculate
    if (ei->pkentry != NULL) return;
    
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
            ei->pawnKingMidgame[colour] += PawnIsolated[MG];
            ei->pawnKingEndgame[colour] += PawnIsolated[EG];
            if (TRACE) T.pawnIsolated[colour]++;
        }
        
        // Apply a penalty if the pawn is stacked
        if (Files[File(sq)] & tempPawns){
            ei->pawnKingMidgame[colour] += PawnStacked[MG];
            ei->pawnKingEndgame[colour] += PawnStacked[EG];
            if (TRACE) T.pawnStacked[colour]++;
        }
        
        // Apply a penalty if the pawn is backward
        if (   !(PassedPawnMasks[!colour][sq] & myPawns)
            &&  (ei->pawnAttacks[!colour] & (1ull << (sq + forward)))){
                
            semi = !(Files[File(sq)] & enemyPawns);
            
            ei->pawnKingMidgame[colour] += PawnBackwards[semi][MG];
            ei->pawnKingEndgame[colour] += PawnBackwards[semi][EG];
            if (TRACE) T.pawnBackwards[colour][semi]++;
        }
        
        // Apply a bonus if the pawn is connected and not backward
        else if (PawnConnectedMasks[colour][sq] & myPawns){
            ei->pawnKingMidgame[colour] += PawnConnected32[relativeSquare32(sq, colour)][MG];
            ei->pawnKingEndgame[colour] += PawnConnected32[relativeSquare32(sq, colour)][EG];
            if (TRACE) T.pawnConnected[colour][sq]++;
        }
    }
}

void evaluateKnights(EvalInfo* ei, Board* board, int colour){
    
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
        ei->attackedBy2[colour] |= ei->attacked[colour] & attacks;
        ei->attacked[colour] |= attacks;
        ei->attackedNoQueen[colour] |= attacks;
        
        // Apply a penalty if the knight is being attacked by a pawn
        if (ei->pawnAttacks[!colour] & (1ull << sq)){
            ei->midgame[colour] += KnightAttackedByPawn[MG];
            ei->endgame[colour] += KnightAttackedByPawn[EG];
            if (TRACE) T.knightAttackedByPawn[colour]++;
        }
        
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

void evaluateBishops(EvalInfo* ei, Board* board, int colour){
    
    int sq, defended, mobilityCount;
    uint64_t tempBishops, enemyPawns, attacks;
    
    tempBishops = board->pieces[BISHOP] & board->colours[colour];
    enemyPawns = board->pieces[PAWN] & board->colours[!colour];
    
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
        ei->attackedBy2[colour] |= ei->attacked[colour] & attacks;
        ei->attacked[colour] |= attacks;
        ei->attackedNoQueen[colour] |= attacks;
        
        // Apply a penalty if the bishop is being attacked by a pawn
        if (ei->pawnAttacks[!colour] & (1ull << sq)){
            ei->midgame[colour] += BishopAttackedByPawn[MG];
            ei->endgame[colour] += BishopAttackedByPawn[EG];
            if (TRACE) T.bishopAttackedByPawn[colour]++;
        }
        
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

void evaluateRooks(EvalInfo* ei, Board* board, int colour){
    
    int sq, open, mobilityCount;
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
            ei->midgame[colour] += RookFile[open][MG];
            ei->endgame[colour] += RookFile[open][EG];
            if (TRACE) T.rookFile[colour][open]++;
        }
        
        // Rook gains a bonus for being located on seventh rank relative to its
        // colour so long as the enemy king is on the last two ranks of the board
        if (   Rank(sq) == (colour == BLACK ? 1 : 6)
            && Rank(relativeSquare(getlsb(enemyKings), colour)) >= 6){
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

void evaluateQueens(EvalInfo* ei, Board* board, int colour){
    
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
        ei->attackedBy2[colour] |= ei->attacked[colour] & attacks;
        ei->attacked[colour] |= attacks;
            
        // Apply a penalty if the queen is under an attack threat
        if ((1ull << sq) & ei->attackedNoQueen[!colour]){
            ei->midgame[colour] += QueenChecked[MG];
            ei->endgame[colour] += QueenChecked[EG];
            if (TRACE) T.queenChecked[colour]++;
        }
        
        // Apply a penalty if the queen is under attack by a pawn
        if ((1ull << sq) & ei->pawnAttacks[!colour]){
            ei->midgame[colour] += QueenCheckedByPawn[MG];
            ei->endgame[colour] += QueenCheckedByPawn[EG];
            if (TRACE) T.queenCheckedByPawn[colour]++;
        }
            
        // Apply a bonus (or penalty) based on the mobility of the queen
        mobilityCount = popcount((ei->mobilityAreas[colour] & attacks));
        ei->midgame[colour] += QueenMobility[mobilityCount][MG];
        ei->endgame[colour] += QueenMobility[mobilityCount][EG];
        if (TRACE) T.queenMobility[colour][mobilityCount]++;
        
        // Update the attack and attacker counts for the queen for use in
        // the king safety calculation. We could the Queen as two attacking
        // pieces. This way King Safety is always used with the Queen attacks
        attacks = attacks & ei->kingAreas[!colour];
        if (attacks != 0ull){
            ei->attackCounts[colour] += 4 * popcount(attacks);
            ei->attackerCounts[colour] += 2;
        }
    }
}

void evaluateKings(EvalInfo* ei, Board* board, int colour){
    
    int defenderCounts, attackCounts;
    
    int file, kingFile, kingRank, kingSq, distance;
    
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
    defenderCounts = popcount(myDefenders & ei->kingAreas[colour]);
    ei->midgame[colour] += KingDefenders[defenderCounts][MG];
    ei->endgame[colour] += KingDefenders[defenderCounts][EG];
    if (TRACE) T.kingDefenders[colour][defenderCounts]++;
    
    // If we have two or more threats to our king area, we will apply a penalty
    // based on the number of squares attacked, and the strength of the attackers
    if (ei->attackerCounts[!colour] >= 2){
        
        // Cap our attackCounts at 99 (KingSafety has 100 slots)
        attackCounts = ei->attackCounts[!colour];
        attackCounts = attackCounts >= 100 ? 99 : attackCounts;
        
        // Scale down attack count if there are no enemy queens
        if (!(board->colours[!colour] & board->pieces[QUEEN]))
            attackCounts *= .25;
    
        ei->midgame[colour] -= KingSafety[attackCounts];
    }
    
    // Pawn Shelter evaluation is stored in the PawnKing evaluation table
    if (ei->pkentry != NULL) return;
    
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

        ei->pawnKingMidgame[colour] += KingShelter[file == kingFile][file][distance][MG];
        ei->pawnKingEndgame[colour] += KingShelter[file == kingFile][file][distance][EG];
        if (TRACE) T.kingShelter[colour][file == kingFile][file][distance]++;
    }    
}

void evaluatePassedPawns(EvalInfo* ei, Board* board, int colour){
    
    int sq, rank, canAdvance, safeAdvance;
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
        
        ei->midgame[colour] += PassedPawn[canAdvance][safeAdvance][rank][MG];
        ei->endgame[colour] += PassedPawn[canAdvance][safeAdvance][rank][EG];
        if (TRACE) T.passedPawn[colour][canAdvance][safeAdvance][rank]++;
    }
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
    
    ei->blockedPawns[WHITE] = ((whitePawns << 8) & (white | black)) >> 8;
    ei->blockedPawns[BLACK] = ((blackPawns >> 8) & (white | black)) << 8,
    
    ei->kingAreas[WHITE] = KingMap[wKingSq] | (1ull << wKingSq) | (KingMap[wKingSq] << 8);
    ei->kingAreas[BLACK] = KingMap[bKingSq] | (1ull << bKingSq) | (KingMap[bKingSq] >> 8);
    
    ei->mobilityAreas[WHITE] = ~(ei->pawnAttacks[BLACK] | (white & kings) | ei->blockedPawns[WHITE]);
    ei->mobilityAreas[BLACK] = ~(ei->pawnAttacks[WHITE] | (black & kings) | ei->blockedPawns[BLACK]);
    
    ei->attacked[WHITE] = ei->attackedNoQueen[WHITE] = kingAttacks(wKingSq, ~0ull);
    ei->attacked[BLACK] = ei->attackedNoQueen[BLACK] = kingAttacks(bKingSq, ~0ull);
    
    ei->occupiedMinusBishops[WHITE] = (white | black) ^ (white & (bishops | queens));
    ei->occupiedMinusBishops[BLACK] = (white | black) ^ (black & (bishops | queens));
    
    ei->occupiedMinusRooks[WHITE] = (white | black) ^ (white & (rooks | queens));
    ei->occupiedMinusRooks[BLACK] = (white | black) ^ (black & (rooks | queens));
    
    ei->passedPawns = 0ull;
    
    ei->attackCounts[WHITE] = ei->attackCounts[BLACK] = 0;
    ei->attackerCounts[WHITE] = ei->attackerCounts[BLACK] = 0;
    
    ei->midgame[WHITE] = ei->midgame[BLACK] = 0;
    ei->endgame[WHITE] = ei->endgame[BLACK] = 0;
    
    ei->pawnKingMidgame[WHITE] = ei->pawnKingMidgame[BLACK] = 0;
    ei->pawnKingEndgame[WHITE] = ei->pawnKingEndgame[BLACK] = 0;
    
    if (TEXEL) ei->pkentry = NULL;
    else       ei->pkentry = getPawnKingEntry(pktable, board->pkhash);
}
