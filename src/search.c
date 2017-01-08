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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "bitutils.h"
#include "bitboards.h"
#include "board.h"
#include "castle.h"
#include "evaluate.h"
#include "piece.h"
#include "psqt.h"
#include "search.h"
#include "transposition.h"
#include "types.h"
#include "time.h"
#include "move.h"
#include "movegen.h"

int TotalNodes;

int EvaluatingPlayer;

uint16_t KillerMoves[MAX_HEIGHT][2];

TransTable Table;
PawnTable PTable;

int HistoryGood[0x10000];
int HistoryTotal[0x10000];

SearchInfo * Info;

uint16_t getBestMove(SearchInfo * info){
    
    int i, depth, value = 0;
    
    // INITALIZE SEARCH GLOBALS
    TotalNodes = 0;
    EvaluatingPlayer = info->board.turn;
    initalizeTranspositionTable(&Table, 22);
    initalizePawnTable(&PTable);
    Info = info;
    
    // POPULATE ROOT'S MOVELIST
    MoveList rootMoveList;
    rootMoveList.size = 0;
    genAllMoves(&(info->board),rootMoveList.moves,&(rootMoveList.size));
    
    // CLEAR HISTORY COUNTERS
    for (i = 0; i < 0x10000; i++){
        HistoryGood[i] = 1;
        HistoryTotal[i] = 1;
    }
    
    // PERFORM ITERATIVE DEEPENING
    for (depth = 1; depth < MAX_DEPTH; depth++){
        
        // Perform full search on Root
        value = aspirationWindow(&(info->board), &rootMoveList, depth, value);
        
        // Don't print a partial search
        if (info->terminateSearch) break;
        
        printf("info depth %d ", depth);
        printf("score cp %d ", value);
        printf("time %d ", (int)(getRealTime() - info->startTime));
        printf("nodes %d ", (int)(TotalNodes));
        printf("nps %d ", (int)(1000 * (TotalNodes / (1 + getRealTime() - info->startTime))));
        printf("hashfull %d ", (250 * Table.used) / Table.maxSize);
        
        printf("pv ");
        printMove(rootMoveList.bestMove);
        printf("\n");
        
        fflush(stdout);
        
        if (info->searchIsDepthLimited && info->depthLimit == depth)
            break;
            
        if (info->searchIsTimeLimited){
            
            if (getRealTime() > info->endTime2)
                break;
            
            if (getRealTime() > info->endTime1)
                break;
        }
    }
    
    // Free the Pawn Table
    destoryPawnTable(&PTable);
    
    return rootMoveList.bestMove;
}

int aspirationWindow(Board * board, MoveList * moveList, int depth, int previousScore){
    
    int alpha, beta, value, margin;
    
    if (depth > 4 && abs(previousScore) < MATE / 2){
        for (margin = 30; margin < 250; margin *= 2){
            alpha = previousScore - margin;
            beta  = previousScore + margin;
            
            value = rootSearch(board, moveList, alpha, beta, depth);
            
            if (value > alpha && value < beta)
                return value;
            
            if (abs(value) > MATE/2)
                break;
        }
    }
    
    return rootSearch(board, moveList, -MATE*2, MATE*2, depth);
}

int rootSearch(Board * board, MoveList * moveList, int alpha, int beta, int depth){
    
    int i, valid = 0, best =-2*MATE, value;
    int currentNodes;
    Undo undo[1];
   
    for (i = 0; i < moveList->size; i++){
        
        currentNodes = TotalNodes;
        
        // APPLY AND VALIDATE MOVE BEFORE SEARCHING
        applyMove(board, moveList->moves[i], undo);
        if (!isNotInCheck(board, !board->turn)){
            revertMove(board, moveList->moves[i], undo);
            moveList->values[i] = -6 * MATE;
            continue;
        }
        
        // INCREMENT COUNTER OF VALID MOVES FOUND
        valid++;
        
        // FULL WINDOW SEARCH ON FIRST MOVE
        if (valid == 1)
            value = -alphaBetaSearch(board, -beta, -alpha, depth-1, 1, PVNODE);
        
        // NULL WINDOW SEARCH ON NON-FIRST MOVES
        else{
            value = -alphaBetaSearch(board, -alpha-1, -alpha, depth-1, 1, CUTNODE);
            
            // NULL WINDOW FAILED HIGH, RESEARCH
            if (value > alpha)
                value = -alphaBetaSearch(board, -beta, -alpha, depth-1, 1, PVNODE);
        }
        
        // REVERT MOVE FROM BOARD
        revertMove(board, moveList->moves[i], undo);
        
        if (value <= alpha)
            moveList->values[i] = -(1<<28) + (TotalNodes - currentNodes); // UPPER VALUE
        else if (value >= beta)
            moveList->values[i] = beta;  // LOWER VALUE
        else
            moveList->values[i] = value; // EXACT VALUE
        
        
        // IMPROVED CURRENT VALUE
        if (value > best){
            best = value;
            moveList->bestMove = moveList->moves[i];
            
            // IMPROVED CURRENT LOWER VALUE
            if (value > alpha)
                alpha = value;
        }
        
        // IMPROVED AND FAILED HIGH
        if (alpha >= beta)
            break;
    }
    
    // SORT MOVELIST FOR NEXT ITERATION
    sortMoveList(moveList);
    return best;
}

int alphaBetaSearch(Board * board, int alpha, int beta, int depth, int height, int nodeType){
    
    int i, value, newDepth, entryValue, entryType;
    int min, max, inCheck, values[MAX_MOVES];
    int valid = 0, size = 0, avoidedQS = 0, eval = 0;
    int oldAlpha = alpha, best = -2 * MATE, optimalValue = -MATE;
    
    uint16_t currentMove, tableMove = NONE_MOVE, bestMove = NONE_MOVE;
    uint16_t moves[MAX_MOVES], played[MAX_MOVES];
    
    TransEntry * entry;
    Undo undo[1];
    
    // SEARCH TIME HAS EXPIRED
    if (Info->searchIsTimeLimited && getRealTime() >= Info->endTime2){
        Info->terminateSearch = 1;
        return board->turn == EvaluatingPlayer ? -MATE : MATE;
    }
    
    // DETERMINE 3-FOLD REPITION
    for (i = board->numMoves-2; i >= 0; i-=2)
        if (board->history[i] == board->hash)
            return 0;
    
    // SEARCH HORIZON REACHED, QSEARCH
    if (depth <= 0){
        
        // DETERMINE CHECK STATUS
        inCheck = !isNotInCheck(board, board->turn);
        
        // DON'T JUMP INTO THE QSEARCH IF WE ARE IN CHECK
        if (inCheck){
            avoidedQS = 1;
            depth += 1;
        }

        else{
            return quiescenceSearch(board, alpha, beta, height);
        }
    }
    
    // INCREMENT TOTAL NODE COUNTER
    TotalNodes++;
    
    // LOOKUP CURRENT POSITION IN TRANSPOSITION TABLE
    entry = getTranspositionEntry(&Table, board->hash);
    
    if (entry != NULL){
        
        // ENTRY MOVE MAY BE CANDIDATE
        tableMove = EntryMove(*entry);
        
        // ENTRY MAY IMPROVE BOUNDS
        if (USE_TRANSPOSITION_TABLE
            && EntryDepth(*entry) >= depth
            && nodeType != PVNODE){
                
            entryValue = EntryValue(*entry);
            entryType = EntryType(*entry);
            
            min = alpha;
            max = beta;
            
            // EXACT VALUE STORED
            if (entryType == PVNODE)
                return entryValue;
            
            // LOWER BOUND STORED
            else if (entryType == CUTNODE)
                min = entryValue > alpha ? entryValue : alpha;
            
            // UPPER BOUND STORED
            else if (entryType == ALLNODE)
                max = entryValue < beta ? entryValue : beta;
            
            // BOUNDS NOW OVERLAP?
            if (min >= max)
                return entryValue;
        }
    }
    
    // DETERMINE CHECK STATUS
    if (!avoidedQS)
        inCheck = !isNotInCheck(board, board->turn);
    
    if (nodeType != PVNODE)
        eval = evaluateBoard(board, &PTable);
    
    // STATIC NULL MOVE PRUNING
    if (USE_STATIC_NULL_PRUNING
        && depth <= 3
        && nodeType != PVNODE
        && !inCheck){
            
        value = eval - (depth * (PawnValue + 15));
        
        if (value > beta)
            return value;
    }
    
    // NULL MOVE PRUNING
    if (USE_NULL_MOVE_PRUNING
        && depth >= 2
        && nodeType != PVNODE
        && canDoNull(board)
        && !inCheck
        && board->history[board->numMoves-1] != NULL_MOVE
        && eval >= beta){
            
        applyNullMove(board, undo);
        
        // PERFORM NULL MOVE SEARCH
        value = -alphaBetaSearch(board, -beta, -beta+1, depth-4, height+1, CUTNODE);
        
        revertNullMove(board, undo);
        
        if (value >= beta)
            return value;
    }
    
    // INTERNAL ITERATIVE DEEPING
    if (USE_INTERNAL_ITERATIVE_DEEPENING
        && depth >= 3
        && tableMove == NONE_MOVE
        && nodeType == PVNODE){
        
        // SEARCH AT A LOWER DEPTH
        value = alphaBetaSearch(board, alpha, beta, depth-2, height, PVNODE);
        if (value <= alpha)
            value = alphaBetaSearch(board, -MATE, beta, depth-2, height, PVNODE);
        
        // GET TABLE MOVE FROM TRANSPOSITION TABLE
        entry = getTranspositionEntry(&Table, board->hash);
        if (entry != NULL)
            tableMove = entry->bestMove;
    }
    
    // GENERATE AND PREPARE MOVE ORDERING
    genAllMoves(board, moves, &size);
    evaluateMoves(board, values, moves, size, height, tableMove);
   
    // CHECK EXTENSION
    depth += (!avoidedQS && inCheck && (nodeType == PVNODE || depth <= 6));
    
    for (i = 0; i < size; i++){
        
        currentMove = getNextMove(moves, values, i, size);
        
        // USE FUTILITY PRUNING
        if (USE_FUTILITY_PRUNING
            && nodeType != PVNODE
            && valid >= 1
            && depth <= 8
            && !inCheck
            && MoveType(currentMove) == NORMAL_MOVE
            && board->squares[MoveTo(currentMove)] == EMPTY){
                
            if (optimalValue == -MATE)
                optimalValue = eval + (depth * 1.25 * PawnValue);
            
            value = optimalValue;
            
            if (value <= alpha)
                continue;
        }
        
        // APPLY AND VALIDATE MOVE BEFORE SEARCHING
        applyMove(board, currentMove, undo);
        if (!isNotInCheck(board, !board->turn)){
            revertMove(board, currentMove, undo);
            continue;
        }
        
        // STORE MOVE IN PLAYED
        played[valid] = currentMove;
    
        // INCREMENT COUNTER OF VALID MOVES FOUND
        valid++;
        
        // DETERMINE IF WE CAN USE LATE MOVE REDUCTIONS
        if (USE_LATE_MOVE_REDUCTIONS
            && valid >= 5
            && depth >= 3
            && ((100 * HistoryGood[currentMove]) / HistoryTotal[currentMove]) < 80
            && !inCheck
            && MoveType(currentMove) == NORMAL_MOVE
            && undo[0].capturePiece == EMPTY
            && isNotInCheck(board, board->turn))
            newDepth = depth - 2 - (valid >= 12) - (nodeType != PVNODE);
        else
            newDepth = depth-1;
         
        // FULL WINDOW SEARCH ON FIRST MOVE
        if (valid == 1 || nodeType != PVNODE){
            
            value = -alphaBetaSearch(board, -beta, -alpha, newDepth, height+1, nodeType);
            
            // IMPROVED BOUND, BUT WAS REDUCED DEPTH?
            if (value > alpha
                && newDepth != depth-1){
                    
                value = -alphaBetaSearch(board, -beta, -alpha, depth-1, height+1, nodeType);
            }
        }
        
        // NULL WINDOW SEARCH ON NON-FIRST / PV MOVES
        else{
            value = -alphaBetaSearch(board, -alpha-1, -alpha, newDepth, height+1, CUTNODE);
            
            // NULL WINDOW FAILED HIGH, RESEARCH
            if (value > alpha)
                value = -alphaBetaSearch(board, -beta, -alpha, depth-1, height+1, PVNODE);
        }
        
        // REVERT MOVE FROM BOARD
        revertMove(board, currentMove, undo);
        
        // IMPROVED CURRENT VALUE
        if (value > best){
            best = value;
            bestMove = currentMove;
            
            // IMPROVED CURRENT LOWER VALUE
            if (value > alpha)
                alpha = value;
        }
        
        // IMPROVED AND FAILED HIGH
        if (alpha >= beta){
            
            // UPDATE KILLER MOVES
            if (MoveType(currentMove) == NORMAL_MOVE
                && undo[0].capturePiece == EMPTY
                && KillerMoves[height][0] != currentMove){
                KillerMoves[height][1] = KillerMoves[height][0];
                KillerMoves[height][0] = currentMove;
            }
            
            goto Cut;
        }
    }
    
    // BOARD IS STALEMATE OR CHECKMATE
    if (valid == 0){
        
        // BOARD IS STALEMATE
        if (isNotInCheck(board, board->turn))
            return 0;
        
        // BOARD IS CHECKMATE
        else 
            return -MATE+height;
    }
    
    Cut:
    
    if (best >= beta && bestMove != NONE_MOVE)
        HistoryGood[bestMove]++;
    
    for (i = valid - 1; i >= 0; i--){
        HistoryTotal[played[i]]++;
        if (HistoryTotal[played[i]] >= 16384){
            HistoryTotal[played[i]] = (HistoryTotal[played[i]] + 1) / 2;
            HistoryGood[played[i]] = (HistoryGood[played[i]] + 1) / 2;
        }
    }
    
    // STORE RESULTS IN TRANSPOSITION TABLE
    if (!Info->searchIsTimeLimited || getRealTime() < Info->endTime2){
        if (best > oldAlpha && best < beta)
            storeTranspositionEntry(&Table, depth,  PVNODE, best, bestMove, board->hash);
        else if (best >= beta)
            storeTranspositionEntry(&Table, depth, CUTNODE, best, bestMove, board->hash);
        else if (best <= oldAlpha)
            storeTranspositionEntry(&Table, depth, ALLNODE, best, bestMove, board->hash);
    }
    
    return best;
}

int quiescenceSearch(Board * board, int alpha, int beta, int height){
    
    int i, size = 0, eval, value = -2*MATE, best = -2*MATE, values[256];
    uint16_t moves[MAX_MOVES], currentMove, maxValueGain;
    Undo undo[1];
    
    // MAX HEIGHT REACHED, STOP HERE
    if (height >= MAX_HEIGHT)
        return evaluateBoard(board, &PTable);
    
    // INCREMENT TOTAL NODE COUNTER
    TotalNodes++;
    
    // GET A STANDING-EVAL OF THE CURRENT BOARD
    eval = evaluateBoard(board, &PTable);
    value = eval;
    
    // UPDATE LOWER BOUND
    if (value > alpha)
        alpha = value;
    
    // BOUNDS NOW OVERLAP?
    if (alpha >= beta)
        return value;
    
    
    if (board->colours[!board->turn] & board->pieces[4])
        maxValueGain = QueenValue + 55;
    else
        maxValueGain = RookValue + 35;
    
    // DELTA PRUNING IN WHEN NO PROMOTIONS AND NOT EXTREME LATE GAME
    if (value + maxValueGain < alpha
        && popcount(board->colours[0] | board->colours[1]) >= 6
        && !(board->colours[0] & board->pieces[0] & RANK_7)
        && !(board->colours[1] & board->pieces[0] & RANK_2))
        return value;
    
    
    // GENERATE AND PREPARE QUIET MOVE ORDERING
    genAllNonQuiet(board, moves, &size);
    evaluateMoves(board, values, moves, size, height, NONE_MOVE);
    
    best = value;
    
    for (i = 0; i < size; i++){
        currentMove = getNextMove(moves, values, i, size);
        
        value = eval + 55 + PieceValues[PieceType(board->squares[MoveTo(currentMove)])];
        
        if (MoveType(currentMove) == PROMOTION_MOVE)
            value += PieceValues[PieceType(1 + (MovePromoType(currentMove) >> 14))];
        
        if (value < alpha)
            continue;
        
        // APPLY AND VALIDATE MOVE BEFORE SEARCHING
        applyMove(board, currentMove, undo);
        if (!isNotInCheck(board, !board->turn)){
            revertMove(board, currentMove, undo);
            continue;
        }
        
        // SEARCH NEXT DEPTH
        value = -quiescenceSearch(board, -beta, -alpha, height+1);
        
        // REVERT MOVE FROM BOARD
        revertMove(board, currentMove, undo);
        
        // IMPROVED CURRENT VALUE
        if (value > best){
            best = value;
            
            // IMPROVED CURRENT LOWER VALUE
            if (value > alpha)
                alpha = value;
        }
        
        // IMPROVED AND FAILED HIGH
        if (alpha >= beta)
            break;
    }
    
    return best;
}

void evaluateMoves(Board * board, int * values, uint16_t * moves, int size, int height, uint16_t tableMove){
    
    int i, value;
    int fromType, toType;
    int toVal;
    
    // GET KILLER MOVES
    uint16_t killer1 = KillerMoves[height][0];
    uint16_t killer2 = KillerMoves[height][1];
    
    for (i = 0; i < size; i++){
        
        // TABLEMOVE FIRST
        value  = 16384 * ( tableMove == moves[i]);
        
        // THEN KILLERS, UNLESS OTHER GOOD CAPTURE
        value += 256   * (   killer1 == moves[i]);
        value += 256   * (   killer2 == moves[i]);
        
        // INFO FOR POSSIBLE CAPTURE
        fromType = PieceType(board->squares[MoveFrom(moves[i])]);
        toType = PieceType(board->squares[MoveTo(moves[i])]);
        toVal = PieceValues[toType];
        
        // ENCOURAGE CAPTURING HIGH VALUE
        value += 5 * toVal;
        
        // ENPASS CAPTURE IS TREATED SEPERATLY
        if (MoveType(moves[i]) == ENPASS_MOVE)
            value += 2*PawnValue;
        
        // WE ARE ONLY CONCERED WITH QUEEN PROMOTIONS
        else if (MoveType(moves[i]) & PROMOTE_TO_QUEEN)
            value += 5*QueenValue;
        
        // CASTLING IS USUALLY A GOOD MOVE
        else if (MoveType(moves[i]) == CASTLE_MOVE)
            value += 256;
        
        // PIECE SQUARE TABLE ORDERING
        value += PSQTopening[fromType][MoveTo(moves[i])] - PSQTopening[fromType][MoveFrom(moves[i])];
        
        // HISTORY ORDERING
        value += ((512 * HistoryGood[moves[i]]) / HistoryTotal[moves[i]]);
        
        values[i] = value;
    }
}

uint16_t getNextMove(uint16_t * moves, int * values, int index, int size){
    int i, best = 0;
    uint16_t bestMove;
    
    // FIND GREATEST VALUE
    for (i = 1; i < size - index; i++)
        if (values[i] > values[best])
            best = i;
        
    bestMove = moves[best];
    
    // MOVE LAST PAIR TO BEST SO WE
    // CAN REDUCE THE EFFECTIVE LIST SIZE
    moves[best] = moves[size-index-1];
    values[best] = values[size-index-1];
    
    return bestMove;
}

void sortMoveList(MoveList * moveList){
    int i, j, tempVal;
    uint16_t tempMove;
    
    for (i = 0; i < moveList->size; i++){
        for (j = i+1; j < moveList->size; j++){
            if (moveList->values[j] > moveList->values[i]){
                
                tempVal = moveList->values[j];
                tempMove = moveList->moves[j];
                
                moveList->values[j] = moveList->values[i];
                moveList->moves[j] = moveList->moves[i];
                
                moveList->values[i] = tempVal;
                moveList->moves[i] = tempMove;
            }
        }
    }
}

int canDoNull(Board * board){
    uint64_t friendly = board->colours[board->turn];
    uint64_t kings = board->pieces[5];
    uint64_t pawns = board->pieces[0];
    
    return (friendly & (kings | pawns)) != friendly;
}