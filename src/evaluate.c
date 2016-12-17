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

int PawnPassedMid[8] = { 0,  10, 10, 23, 39, 58, 70, 0};
int PawnPassedEnd[8] = { 0,  12, 12, 26, 44, 66, 90, 0};

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
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

int BishopHasWings[PHASE_NB] = {13, 36};

int BishopPair[PHASE_NB] = {46, 64};

int evaluateBoard(Board * board, PawnTable * ptable){
    
    int mid = 0, end = 0;
    int curPhase, midEval, endEval, eval;
    
    int knightCount = 0;
    int bishopCount = 0;
    int rookCount   = 0;
    int queenCount  = 0;
    
    uint64_t white   = board->colours[WHITE];
    uint64_t black   = board->colours[BLACK];
    uint64_t pawns   = board->pieces[PAWN];
    uint64_t knights = board->pieces[KNIGHT];
    uint64_t bishops = board->pieces[BISHOP];
    uint64_t rooks   = board->pieces[ROOK];
    uint64_t queens  = board->pieces[QUEEN];
    uint64_t kings   = board->pieces[KING];
    
    // CHECK FOR RECOGNIZED DRAWS
    if (pawns == 0 && rooks == 0 && queens == 0){
        
        // K v K
        if (kings == (white | black))
            return 0;
        
        // K+B v K and K+N v K
        if (countSetBits(white & (knights | bishops)) <= 1 && 
            countSetBits(black) == 1)
            return 0;
        else if (countSetBits(black & (knights | bishops)) <= 1 && 
            countSetBits(white) == 1)
            return 0;
         
        // K+B+N v K 
        /* Pawel Koziol pointed out that this should not be here! Will add
            a PSQT table to find the mates 
        if (countSetBits(black) == 1 &&
            countSetBits(white & bishops) == 1 &&
            countSetBits(white & knights) == 1)
            return 0;
        else if (countSetBits(white) == 1 &&
            countSetBits(black & bishops) == 1 &&
            countSetBits(black & knights) == 1)
            return 0;
        */
        
        // K+N+N v K
        if (countSetBits(black) == 1 &&
            countSetBits(white & knights) == 2 &&
            countSetBits(white & bishops) == 0)
            return 0;
        else if (countSetBits(white) == 1 &&
            countSetBits(black & knights) == 2 &&
            countSetBits(black & bishops) == 0)
            return 0;
    }
    
    
    // ATTEMPT TO USE PREVIOUS PAWN EVALUATION
    PawnEntry * pentry = getPawnEntry(ptable, board->phash);
    
    // PAWN ENTRY FOUND
    if (pentry != NULL){
        mid = pentry->mg;
        end = pentry->eg;
    } 
    
    // NO ENTRY FOUND, EVALUATE AND STORE
    else {
        evaluatePawns(&mid, &end, board);
        storePawnEntry(ptable, board->phash, mid, end);
    }
    
    evaluatePieces(&mid, &end, board, &knightCount, &bishopCount, &rookCount, &queenCount);
    
    midEval = board->opening + mid;
    endEval = board->endgame + end;
    
    midEval += (board->turn == WHITE) ? 5 : -5;
    endEval += (board->turn == WHITE) ? 7 : -7;
    
    curPhase = 24 - (1 * (knightCount + bishopCount))
                  - (2 * rookCount)
                  - (4 * queenCount);
    curPhase = (curPhase * 256 + 12) / 24;
    
    eval = ((midEval * (256 - curPhase)) + (endEval * curPhase)) / 256;
    
    return board->turn == WHITE ? eval : -eval;
}

void evaluatePawns(int * mid, int * end, Board * board){
    
    uint64_t allPawns = board->pieces[PAWN];
    uint64_t myPawns, enemyPawns, allMyPawns;
    
    int mg = 0, eg = 0;
    int colour, i, sq, file, rank;
    
    for (colour = BLACK; colour >= WHITE; colour--){
        
        mg = -mg;
        eg = -eg;
        
        myPawns = allPawns & board->colours[colour];
        allMyPawns = myPawns;
        enemyPawns = allPawns ^ myPawns;
        
        for (i = 0; myPawns != 0; i++){
            
            sq = getLSB(myPawns);
            myPawns ^= (1ull << sq);
            
            file = sq & 7;
            rank = (colour == BLACK) ? (7 - (sq >> 3)) : (sq >> 3);
            
            if (!(PassedPawnMasks[colour][sq] & enemyPawns)){
                mg += PawnPassedMid[rank];
                eg += PawnPassedEnd[rank];
            }
            
            if (!(IsolatedPawnMasks[sq] & myPawns)){
                mg -= PAWN_ISOLATED_MID;
                eg -= PAWN_ISOLATED_END;
            }
            
            else if (Files[file] & myPawns){
                mg -= PAWN_STACKED_MID;
                eg -= PAWN_STACKED_END;
            }
            
            if (PawnConnectedMasks[colour][sq] & allMyPawns){
                mg += PawnConnected[colour][sq];
                eg += PawnConnected[colour][sq];
            }
        }
    }
    
    *mid += mg;
    *end += eg;
}

void evaluatePieces(int * mid, int * end, Board * board, int * knightCount, int * bishopCount, int * rookCount, int * queenCount){
    
    uint64_t white = board->colours[WHITE];
    uint64_t black = board->colours[BLACK];
    
    uint64_t pawns   = board->pieces[PAWN];
    uint64_t knights = board->pieces[KNIGHT];
    uint64_t bishops = board->pieces[BISHOP];
    uint64_t rooks   = board->pieces[ROOK];
    uint64_t queens  = board->pieces[QUEEN];
    uint64_t kings   = board->pieces[KING];
    
    uint64_t whitePawns = white & pawns;
    uint64_t blackPawns = black & pawns;
    uint64_t notEmpty = white | black;
    
    uint64_t myPieces, myPawns, enemyPawns;
    uint64_t tempKnights, tempBishops, tempRooks, tempQueens;
    uint64_t occupiedMinusMyBishops, occupiedMinusMyRooks;
    uint64_t attacks, mobilityArea;
    
    int mg = 0, eg = 0;
    int mobiltyCount, defended;
    int colour, bit, rank, file;
    
    int whiteKingSq = getLSB(white & kings);
    int blackKingSq = getLSB(black & kings);
    
    int attackCounts[COLOUR_NB] = {0, 0};
    int attackerCounts[COLOUR_NB] = {0, 0};
    
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
    
    
    for (colour = BLACK; colour >= WHITE; colour--){
        
        // Negate the scores so that the scores are from
        // White's perspective after the loop completes
        mg = -mg; eg = -eg;
        
        myPieces = board->colours[colour];
        myPawns = myPieces & pawns;
        enemyPawns = pawns ^ myPawns;
        
        occupiedMinusMyBishops = notEmpty ^ (myPieces & (bishops | queens));
        occupiedMinusMyRooks = notEmpty ^ (myPieces & (rooks | queens));
        
        // Don't include squares that are attacked by enemy pawns, 
        // occupied by our king, or occupied with our blocked pawns
        // in our mobilityArea. This definition of mobilityArea is
        // derived directly from Stockfish's evaluation features. 
        mobilityArea = ~(pawnAttacks[!colour] | (myPieces & kings) | blockedPawns[colour]);
        
        tempKnights = myPieces & knights;
        tempBishops = myPieces & bishops;
        tempRooks = myPieces & rooks;
        tempQueens = myPieces & queens;
        
        // Generate the attack boards for each Knight,
        // and evaluate any other bonuses / penalties
        while (tempKnights != 0){
            
            // Pop the next Knight off
            bit = getLSB(tempKnights);
            tempKnights ^= (1ull << bit);
            (*knightCount)++;
            
            // Generate the attack board
            attacks = KnightAttacks(bit, ~0ull);
            
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
            
            // Get the attack counts for this piece
            attacks = attacks & kingAreas[!colour];
            if (attacks != 0ull){
                attackCounts[colour] += 2 * popcount(attacks);
                attackerCounts[colour]++;
            }
        }
        
        
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
        
        // Generate the attack boards for each Bishop,
        // and evaluate any other bonuses / penalties
        while (tempBishops != 0){
            
            // Pop the next Bishop off
            bit = getLSB(tempBishops);
            tempBishops ^= (1ull << bit);
            (*bishopCount)++;
            
            // Generate the attack board
            attacks = BishopAttacks(bit, occupiedMinusMyBishops, ~0ull);
            
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
            
            // Get the attack counts for this piece
            attacks = attacks & kingAreas[!colour];
            if (attacks != 0ull){
                attackCounts[colour] += 2 * popcount(attacks);
                attackerCounts[colour]++;
            }
        }
        
        // Generate the attack boards for each Rook,
        // and evaluate any other bonuses / penalties
        while (tempRooks != 0){
            
            // Pop the next Rook off
            bit = getLSB(tempRooks);
            tempRooks ^= (1ull << bit);
            (*rookCount)++;
            
            // Generate the attack board
            attacks = RookAttacks(bit, occupiedMinusMyRooks, ~0ull);
            
            rank = bit >> 3;
            file = bit & 7;
            
            // Rook is on a semi-open file if there are no
            // pawns of the rook's colour on the file. If
            // there are no pawns at all, it is an open file
            if (!(myPawns & Files[file])){
                if (!(enemyPawns & Files[file])){
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
            if (rank == (colour == BLACK ? 1 : 6)){
                mg += ROOK_ON_7TH_MID;
                eg += ROOK_ON_7TH_END;
            }
            
            // Rook gains a mobility bonus based off of the number
            // of attacked or defended squares within the mobility area
            mobiltyCount = popcount((mobilityArea & attacks));
            mg += RookMobility[MG][mobiltyCount];
            eg += RookMobility[EG][mobiltyCount];
            
            // Get the attack counts for this piece
            attacks = attacks & kingAreas[!colour];
            if (attacks != 0ull){
                attackCounts[colour] += 3 * popcount(attacks);
                attackerCounts[colour]++;
            }
        }
        
        while (tempQueens != 0){
            bit = getLSB(tempQueens);
            tempQueens ^= (1ull << bit);
            (*queenCount)++;
            
            attacks = RookAttacks(bit, occupiedMinusMyRooks, ~0ull)
                | BishopAttacks(bit, occupiedMinusMyBishops, ~0ull);
                
            //// Queen gains a mobility bonus based off of the number
            //// of attacked or defended squares within the mobility area
            //mobiltyCount = countSetBits((mobilityArea & attacks));
            //mg += QueenMobility[MG][mobiltyCount];
            //eg += QueenMobility[EG][mobiltyCount];
            //
            
            // Get the attack counts for this piece
            attacks = attacks & kingAreas[!colour];
            if (attacks != 0ull){
                attackCounts[colour] += 5 * popcount(attacks);
                attackerCounts[colour]++;
            }
        }
    }
    
    for (colour = BLACK; colour >= WHITE; colour--){
        
        // Negate the scores so that the scores are from
        // White's perspective after the loop completes
        mg = -mg; eg = -eg;
        
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
        
        if (attackerCounts[!colour] > 2){
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
    
    *mid += mg;
    *end += eg;
}