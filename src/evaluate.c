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

int SafetyTable[100] = { // Taken from CPW / Stockfish
    0,  0,   1,   2,   3,   5,   7,   9,  12,  15,
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

int PawnConnected[COLOUR_NB][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0,
      2, 2, 2, 3, 3, 2, 2, 2,
      4, 4, 5, 6, 6, 5, 4, 4,
      7, 8,10,12,12,10, 8, 7,
     11,14,17,21,21,17,14,11,
     16,21,25,33,33,25,21,16,
     32,42,50,55,55,50,42,32,
      0, 0, 0, 0, 0, 0, 0, 0, },
      
    { 0, 0, 0, 0, 0, 0, 0, 0,
     32,42,50,55,55,50,42,32,
     16,21,25,33,33,25,21,16,
     11,14,17,21,21,17,14,11,
      7, 8,10,12,12,10, 8, 7,
      4, 4, 5, 6, 6, 5, 4, 4,
      2, 2, 2, 3, 3, 2, 2, 2,
      0, 0, 0, 0, 0, 0, 0, 0, }
};

int KnightOutpostValues[PHASE_NB][2] = {{20, 40}, {10, 20}};
int BishopOutpostValues[PHASE_NB][2] = {{15, 30}, { 3,  5}};

// [phase][canAdvance][safeAdvance][rank]
int PawnPassed[PHASE_NB][2][2][RANK_NB] = {

   {{{0,  10, 10, 23, 39, 58, 70, 0},
     {0,  10, 10, 23, 39, 58, 70, 0}},

    {{0,  10, 15, 34, 58, 88,105, 0},
     {0,  18, 20, 40, 76,116,199, 0}}},
     
   {{{0,  12, 12, 26, 44, 66, 90, 0},
     {0,  12, 12, 26, 44, 66, 90, 0}},

    {{0,  20, 21, 42, 70,105,140, 0},
     {0,  25, 27, 52, 98,135,220, 0}}},
};

int PieceValues[8] = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue, 0, 0};

int KnightMobility[PHASE_NB][9] = {
    {-30, -25, -10,   0,  10,  18,  26,  34,  42},
    {-30, -25,   0,   9,  15,  21,  28,  35,  36}
};

int BishopMobility[PHASE_NB][14] = {
    {-30, -20, -15,   0, 15,  21,  26,  31,  34,  36,  37,  38,  38,  38},
    {-30, -20, -15,   0, 15,  21,  26,  31,  34,  36,  37,  38,  38,  38},
};

int RookMobility[PHASE_NB][15] = {
    {-30, -25, -10,  -5,  -3,  -1,   6,  11,  15,  19,  23,  25,  26,  27, 27},
    {-35, -20, -10,   0,  10,  19,  27,  33,  39,  41,  43,  45,  47,  48, 48}
};

int QueenMobility[PHASE_NB][28] = {
    {-50, -40, -20,   0,   2,   4,   6,
       8,  11,  15,  19,  20,  21,  22,
      24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24},
    
    {-50, -40, -20, -10,  0,  4,  8,
      12,  15,  18,  21, 24, 27, 30,
      35,  43,  43,  43, 43, 43, 43,
      43,  43,  43,  43, 43, 43, 43}
};

int BishopHasWings[PHASE_NB] = {13, 36};

int BishopPair[PHASE_NB] = {46, 64};

int evaluateBoard(Board * board, PawnTable * ptable){
    
    uint64_t white   = board->colours[WHITE];
    uint64_t black   = board->colours[BLACK];
    uint64_t pawns   = board->pieces[PAWN];
    uint64_t knights = board->pieces[KNIGHT];
    uint64_t bishops = board->pieces[BISHOP];
    uint64_t rooks   = board->pieces[ROOK];
    uint64_t queens  = board->pieces[QUEEN];
    uint64_t kings   = board->pieces[KING];
    
    // Check for recognized draws
    if ((pawns | rooks | queens) == 0ull){
        
        // K v K
        if (kings == (white | black))
            return 0;
        
        if ((white & kings) == white){
            
            // K vs K+B or K vs K+N
            if (popcount(black & (knights | bishops)) <= 1)
                return 0;
            
            // K vs K+N+N
            if (popcount(black & knights) == 2 
                && popcount(black & bishops) == 0)
                return 0;
        }
        
        if ((black & kings) == black){
            
            // K+B vs K or K+N vs K
            if (popcount(white & (knights | bishops)) <= 1)
                return 0;
            
            // K+N+N vs K
            if (popcount(white & knights) == 2 
                && popcount(white & bishops) == 0)
                return 0;
        }
    }
    
    return evaluatePieces(board, ptable);
}

int evaluatePieces(Board * board, PawnTable * ptable){
    
    uint64_t white   = board->colours[WHITE];
    uint64_t black   = board->colours[BLACK];
    uint64_t pawns   = board->pieces[PAWN];
    uint64_t knights = board->pieces[KNIGHT];
    uint64_t bishops = board->pieces[BISHOP];
    uint64_t rooks   = board->pieces[ROOK];
    uint64_t queens  = board->pieces[QUEEN];
    uint64_t kings   = board->pieces[KING];
    
    uint64_t myPieces, myPawns, enemyPawns;
    uint64_t tempPawns, tempKnights, tempBishops, tempRooks, tempQueens;
    uint64_t occupiedMinusMyBishops, occupiedMinusMyRooks;
    uint64_t attacks, mobilityArea;
    uint64_t passedPawns = 0ull;
    
    int mg = 0, eg = 0;
    int pawnmg = 0, pawneg = 0;
    int eval, curPhase;
    int mobiltyCount, defended;
    int colour, bit, rank;
    int canAdvance, safeAdvance;
    
    int whiteKingSq = getLSB(white & kings);
    int blackKingSq = getLSB(black & kings);
    
    uint64_t whitePawns = white & pawns;
    uint64_t blackPawns = black & pawns;
    uint64_t notEmpty = white | black;
    
    uint64_t pawnAttacks[COLOUR_NB] = {
        (((whitePawns << 9) & ~FILE_A) | ((whitePawns << 7) & ~FILE_H)),
        (((blackPawns >> 9) & ~FILE_H) | ((blackPawns >> 7) & ~FILE_A))
    };
    
    uint64_t blockedPawns[COLOUR_NB] = {
        (((whitePawns << 8 ) & black) >> 8),
        (((blackPawns >> 8 ) & white) << 8),
    };
    
    uint64_t kingAreas[COLOUR_NB] = {
        ((KingMap[whiteKingSq] | (1ull << whiteKingSq)) | (KingMap[whiteKingSq] << 8)),
        ((KingMap[blackKingSq] | (1ull << blackKingSq)) | (KingMap[blackKingSq] >> 8))
    };
    
    uint64_t allAttackBoards[COLOUR_NB] = {
        KingAttacks(whiteKingSq, ~0ull),
        KingAttacks(blackKingSq, ~0ull)
    };
    
    int attackCounts[COLOUR_NB] = {0, 0};
    int attackerCounts[COLOUR_NB] = {0, 0};
    
    PawnEntry * pentry = getPawnEntry(ptable, board->phash);
    
    for (colour = BLACK; colour >= WHITE; colour--){
        
        // Negate the scores so that the scores are from
        // White's perspective after the loop completes
        mg = -mg; pawnmg = -pawnmg; 
        eg = -eg; pawneg = -pawneg;
        
        myPieces = board->colours[colour];
        myPawns = myPieces & pawns;
        enemyPawns = pawns ^ myPawns;
        
        tempPawns = myPawns;
        tempKnights = myPieces & knights;
        tempBishops = myPieces & bishops;
        tempRooks = myPieces & rooks;
        tempQueens = myPieces & queens;
        
        occupiedMinusMyBishops = notEmpty ^ (myPieces & (bishops | queens));
        occupiedMinusMyRooks = notEmpty ^ (myPieces & (rooks | queens));
        
        // Don't include squares that are attacked by enemy pawns, 
        // occupied by our king, or occupied with our blocked pawns
        // in our mobilityArea. This definition of mobilityArea is
        // derived directly from Stockfish's evaluation features. 
        mobilityArea = ~(pawnAttacks[!colour] | (myPieces & kings) | blockedPawns[colour]);
        
        // Bishop gains a bonus for pawn wings
        if (tempBishops != 0
            && (myPawns & (FILE_A|FILE_B|FILE_C))
            && (myPawns & (FILE_F|FILE_G|FILE_H))){
                
            mg += BishopHasWings[MG];
            eg += BishopHasWings[EG];
        }
        
        // Bishop gains a bonus for being in a pair
        if ((tempBishops & WHITE_SQUARES)
            && (tempBishops & BLACK_SQUARES)){
                
            mg += BishopPair[MG];
            eg += BishopPair[EG];
        }
        
        // King gains a bonus if it has already castled
        if (board->hasCastled[colour]){
            mg += KING_HAS_CASTLED;
            eg += KING_HAS_CASTLED;
        } 
        
        // King gains a bonus if it still may castle
        else if (board->castleRights & (3 << (2*colour))){
            mg += KING_CAN_CASTLE;
            eg += KING_CAN_CASTLE;
        }
        
        
        // Get the attack board for the pawns
        attacks = pawnAttacks[colour] & kingAreas[!colour];
        allAttackBoards[colour] |= pawnAttacks[colour];
        
        // Update the counters for the safety evaluation
        if (attacks != 0ull){
            attackCounts[colour] += 2 * popcount(attacks);
            attackerCounts[colour] += 1;
        }
        
        // If we were able to retrieve a Pawn Entry from the
        // pawn table, we can skip this part of the pawn eval
        if (pentry != NULL) goto AfterPawnLoop;
        
        // Evaluate all of this colour's Pawns
        while(tempPawns != 0ull){
            
            // Pop the next Pawn off
            bit = getLSB(tempPawns);
            tempPawns ^= (1ull << bit);
            
            // Save the fact that this pawn is passed. We will
            // use it later in order to apply a proper bonus
            if (!(PassedPawnMasks[colour][bit] & enemyPawns))
                passedPawns |= (1ull << bit);
            
            // Apply a penalty if the pawn is isolated
            if (!(IsolatedPawnMasks[bit] & tempPawns)){
                pawnmg -= PAWN_ISOLATED_MID;
                pawneg -= PAWN_ISOLATED_END;
            }
            
            // Apply a penalty if the pawn is stacked
            if (Files[bit & 7] & tempPawns){
                pawnmg -= PAWN_STACKED_MID;
                pawneg -= PAWN_STACKED_END;
            }
            
            // Apply a bonus if the pawn is connected
            if (PawnConnectedMasks[colour][bit] & myPawns){
                pawnmg += PawnConnected[colour][bit];
                pawneg += PawnConnected[colour][bit];
            }
            
        } AfterPawnLoop:
        
        // Evaluate all of this colour's Knights
        while (tempKnights != 0){
            
            // Pop the next Knight off
            bit = getLSB(tempKnights);
            tempKnights ^= (1ull << bit);
            
            // Generate the attack board
            attacks = KnightAttacks(bit, ~0ull);
            allAttackBoards[colour] |= attacks;
            
            // Knight is in an outpost square, unable to be
            // attacked by enemy pawns, on or between ranks
            // four through seven, relative to it's colour
            if (OutpostRanks[colour] & (1ull << bit)
                && !(OutpostSquareMasks[colour][bit] & enemyPawns)){
                    
                defended = (pawnAttacks[colour] & (1ull << bit)) != 0;
                
                mg += KnightOutpostValues[MG][defended];
                eg += KnightOutpostValues[EG][defended];
            }
            
            // Knight gains a mobility bonus based off of the number
            // of attacked or defended squares within the mobility area
            mobiltyCount = popcount((mobilityArea & attacks));
            mg += KnightMobility[MG][mobiltyCount];
            eg += KnightMobility[EG][mobiltyCount];
            
            // Get the attack counts for this Knight
            attacks = attacks & kingAreas[!colour];
            if (attacks != 0ull){
                attackCounts[colour] += 2 * popcount(attacks);
                attackerCounts[colour]++;
            }
        }
        
        // Evaluate all of this colour's Bishops
        while (tempBishops != 0){
            
            // Pop the next Bishop off
            bit = getLSB(tempBishops);
            tempBishops ^= (1ull << bit);
            
            // Generate the attack board
            attacks = BishopAttacks(bit, occupiedMinusMyBishops, ~0ull);
            allAttackBoards[colour] |= attacks;
            
            // Bishop is in an outpost square, unable to be
            // attacked by enemy pawns, on or between ranks
            // four through seven, relative to it's colour
            if (OutpostRanks[colour] & (1ull << bit)
                && !(OutpostSquareMasks[colour][bit] & enemyPawns)){
                    
                defended = (pawnAttacks[colour] & (1ull << bit)) != 0;
                
                mg += BishopOutpostValues[MG][defended];
                eg += BishopOutpostValues[EG][defended];
            }
            
            // Bishop gains a mobility bonus based off of the number
            // of attacked or defended squares within the mobility area
            mobiltyCount = popcount((mobilityArea & attacks));
            mg += BishopMobility[MG][mobiltyCount];
            eg += BishopMobility[EG][mobiltyCount];
            
            // Get the attack counts for this Bishop
            attacks = attacks & kingAreas[!colour];
            if (attacks != 0ull){
                attackCounts[colour] += 2 * popcount(attacks);
                attackerCounts[colour]++;
            }
        }
        
        
        // Evaluate all of this colour's Rooks
        while (tempRooks != 0){
            
            // Pop the next Rook off
            bit = getLSB(tempRooks);
            tempRooks ^= (1ull << bit);
            
            // Generate the attack board
            attacks = RookAttacks(bit, occupiedMinusMyRooks, ~0ull);
            allAttackBoards[colour] |= attacks;
            
            // Rook is on a semi-open file if there are no
            // pawns of the Rook's colour on the file. If
            // there are no pawns at all, it is an open file
            if (!(myPawns & Files[bit & 7])){
                if (!(enemyPawns & Files[bit & 7])){
                    mg += ROOK_OPEN_FILE_MID;
                    eg += ROOK_OPEN_FILE_END;
                }
                else{
                    mg += ROOK_SEMI_FILE_MID;
                    eg += ROOK_SEMI_FILE_END;
                }
            }
            
            // Rook gains a bonus for being located
            // on seventh rank relative to its colour
            if ((bit >> 3) == (colour == BLACK ? 1 : 6)){
                mg += ROOK_ON_7TH_MID;
                eg += ROOK_ON_7TH_END;
            }
            
            // Rook gains a mobility bonus based off of the number
            // of attacked or defended squares within the mobility area
            mobiltyCount = popcount((mobilityArea & attacks));
            mg += RookMobility[MG][mobiltyCount];
            eg += RookMobility[EG][mobiltyCount];
            
            // Get the attack counts for this Rook
            attacks = attacks & kingAreas[!colour];
            if (attacks != 0ull){
                attackCounts[colour] += 3 * popcount(attacks);
                attackerCounts[colour]++;
            }
        }
        
        
        // Evaluate all of this colour's Queens
        while (tempQueens != 0){
            
            // Pop the next Queen off
            bit = getLSB(tempQueens);
            tempQueens ^= (1ull << bit);
            
            // Generate the attack board
            attacks = RookAttacks(bit, occupiedMinusMyRooks, ~0ull)
                | BishopAttacks(bit, occupiedMinusMyBishops, ~0ull);
            allAttackBoards[colour] |= attacks;
                
            // Queen gains a mobility bonus based off of the number
            // of attacked or defended squares within the mobility area
            mobiltyCount = popcount((mobilityArea & attacks));
            mg += QueenMobility[MG][mobiltyCount];
            eg += QueenMobility[EG][mobiltyCount];
            
            // Get the attack counts for this Queen
            attacks = attacks & kingAreas[!colour];
            if (attacks != 0ull){
                attackCounts[colour] += 4 * popcount(attacks);
                attackerCounts[colour]++;
            }
        }
    }
    
    // If we were unable to find an entry, store one now
    if (pentry == NULL){
        storePawnEntry(ptable, board->phash, passedPawns, pawnmg, pawneg);
        mg += pawnmg;
        eg += pawneg;
    } 
    
    // Otherwise, read the entry for the needed information
    else {
        mg += pentry->mg;
        eg += pentry->eg;
        passedPawns = pentry->passed;
    }
    
    // Evaluate the passed pawns for both colours
    for (colour = BLACK; colour >= WHITE; colour--){
        
        // Negate the scores so that the scores are from
        // White's perspective after the loop completes
        mg = -mg; eg = -eg;
        
        tempPawns = board->colours[colour] & passedPawns;
        
        while (tempPawns != 0ull){
            
            // Pop off the next Passed Pawn
            bit = getLSB(tempPawns);
            tempPawns ^= (1ull << bit);
            
            rank = (colour == BLACK) ? (7 - (bit >> 3)) : (bit >> 3);
            
            uint64_t dest = (colour == BLACK) 
                ? ((1ull << bit) >> 8)
                : ((1ull << bit) << 8);
                
            canAdvance = (dest & notEmpty) == 0ull;
            safeAdvance = (dest & allAttackBoards[!colour]) == 0ull;
            
            mg += PawnPassed[MG][canAdvance][safeAdvance][rank];
            eg += PawnPassed[EG][canAdvance][safeAdvance][rank];
        }
    }

    for (colour = BLACK; colour >= WHITE; colour--){
        
        // Negate the scores so that the scores are from
        // White's perspective after the loop completes
        mg = -mg; eg = -eg;
        
        if (attackerCounts[!colour] >= 2){
            int n = attackCounts[!colour];
            if (n >= 100) n = 99;
            
            if (!(board->colours[!colour] & queens))
                n *= .5;
            
            if (!(board->colours[!colour] & rooks))
                n *= .8;
        
            mg -= SafetyTable[n];
            eg -= SafetyTable[n];
        }
    }
    
    mg += board->opening;
    eg += board->endgame;
    
    mg += (board->turn == WHITE) ? 5 : -5;
    eg += (board->turn == WHITE) ? 7 : -7;
    
    curPhase = 24 - (popcount(knights | bishops))
                  - (popcount(rooks) << 1)
                  - (popcount(queens) << 2);
                  
    curPhase = (curPhase * 256 + 12) / 24;
    
    eval = ((mg * (256 - curPhase)) + (eg * curPhase)) / 256;
    
    return board->turn == WHITE ? eval : -eval;
}