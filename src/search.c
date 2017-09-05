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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bitutils.h"
#include "bitboards.h"
#include "board.h"
#include "castle.h"
#include "evaluate.h"
#include "history.h"
#include "piece.h"
#include "psqt.h"
#include "search.h"
#include "transposition.h"
#include "types.h"
#include "time.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"

uint64_t TotalNodes;
int EvaluatingPlayer;
SearchInfo * Info;
HistoryTable History;

extern TransTable Table;
extern PawnTable PTable;

uint16_t KillerMoves[MAX_HEIGHT][2];

uint16_t getBestMove(SearchInfo * info){
    
    int i, depth, elapsed, hashfull, value = 0;
    
    // Create the main Principle Variation
    PVariation pv;
    pv.length = 0;
    
    // Initialize search globals
    TotalNodes = 0ull;
    EvaluatingPlayer = info->board.turn;
    Info = info;
    
    // Prepare the transposition tables
    updateTranspositionTable(&Table);
    initalizePawnTable(&PTable);
    reduceHistory(History);
    
    // Populate the root's moves
    MoveList rootMoves;
    rootMoves.size = 0;
    genAllLegalMoves(&info->board, rootMoves.moves, &rootMoves.size);
    
    // Perform interative deepening
    for (depth = 1; depth < MAX_DEPTH; depth++){
        
        // Perform full search on Root
        value = aspirationWindow(&pv, &info->board, &rootMoves, depth, value);
        
        // Don't print a partial search
        if (info->terminateSearch) break;
        
        elapsed = (int)(getRealTime() - info->startTime);
        hashfull = (1000 * Table.used) / (Table.numBuckets * BUCKET_SIZE);
        
        printf("info depth %d ", depth);
        printf("score cp %d ", value);
        printf("time %d ", elapsed);
        printf("nodes %"PRIu64" ", TotalNodes);
        printf("nps %d ", (int)(1000 * (TotalNodes / (1 + elapsed))));
        printf("hashfull %d ", hashfull);
        printf("pv ");
        
        // Print the Principle Variation
        for (i = 0; i < pv.length; i++){
            printMove(pv.line[i]);
            printf(" ");
        }
        
        printf("\n");
        fflush(stdout);
        
        // Check for depth based termination
        if (info->searchIsDepthLimited && info->depthLimit == depth)
            break;
            
        // Check for time based termination 
        if (info->searchIsTimeLimited){
            if (getRealTime() > info->endTime2) break;
            if (getRealTime() > info->endTime1) break;
        }
    }
    
    // Free the Pawn Table
    destoryPawnTable(&PTable);
    
    return rootMoves.bestMove;
}

int aspirationWindow(PVariation * pv, Board * board, MoveList * moveList, int depth, int lastScore){
    
    int alpha, beta, value, margin;
    
    // Only use an aspiration window on searches that are greater
    // than 4 depth, and did not recently return a MATE score
    if (depth > 4 && abs(lastScore) < MATE / 2){
        
        // Use the windows [30, 60, 120, 240]
        for (margin = 30; margin < 250; margin *= 2){
            
            // Adjust the bounds. There is some debate about
            // how this should be done after we know value.
            alpha = lastScore - margin;
            beta  = lastScore + margin;
            
            // Perform the search on the modified window
            value = rootSearch(pv, board, moveList, alpha, beta, depth);
            
            // Result was within our window
            if (value > alpha && value < beta)
                return value;
            
            // Result was a mate score, force a full search
            if (abs(value) > MATE/2)
                break;
        }
    }
    
    // No searches scored within our aspiration windows, search full window
    return rootSearch(pv, board, moveList, -MATE, MATE, depth);
}


int rootSearch(PVariation * pv, Board * board, MoveList * moveList, int alpha, int beta, int depth){
    
    Undo undo[1];
    uint64_t currentNodes;
    int i, value, best = -MATE;
    
    // Zero out the root and local principle variations
    pv->length = 0;
    PVariation lpv;
    lpv.length = 0;
   
    // Search through each move in the root's legal move list
    for (i = 0; i < moveList->size; i++){
        
        currentNodes = TotalNodes;
        
        // Apply the current move to the board
        applyMove(board, moveList->moves[i], undo);
        
        // Full window search for the first move
        if (i == 0)
            value = -alphaBetaSearch(&lpv, board, -beta, -alpha, depth-1, 1, PVNODE);
        
        // Null window search on all other moves
        else{
            value = -alphaBetaSearch(&lpv, board, -alpha-1, -alpha, depth-1, 1, CUTNODE);
            
            // Null window failed high, we must search on a full window
            if (value > alpha)
                value = -alphaBetaSearch(&lpv, board, -beta, -alpha, depth-1, 1, PVNODE);
        }
        
        // Revert the board state
        revertMove(board, moveList->moves[i], undo);
        
        
        if (value <= alpha)
            moveList->values[i] = -(1<<28) + (int)(TotalNodes - currentNodes); // UPPER VALUE
        else if (value >= beta)
            moveList->values[i] = beta;  // LOWER VALUE
        else
            moveList->values[i] = value; // EXACT VALUE
        
        
        // Improved current value
        if (value > best){
            best = value;
            moveList->bestMove = moveList->moves[i];
            
            // IMPROVED CURRENT LOWER VALUE
            if (value > alpha){
                alpha = value;
                
                // Update the Principle Variation
                pv->length = 1 + lpv.length;
                pv->line[0] = moveList->moves[i];
                memcpy(pv->line + 1, lpv.line, sizeof(uint16_t) * lpv.length);
            }
        }
        
        // IMPROVED AND FAILED HIGH
        if (alpha >= beta)
            break;
    }
    
    // SORT MOVELIST FOR NEXT ITERATION
    sortMoveList(moveList);
    return best;
}

int alphaBetaSearch(PVariation * pv, Board * board, int alpha, int beta, int depth, int height, int nodeType){
    
    int i, value, inCheck, isQuiet, R;
    int min, max, entryValue, entryType, oldAlpha = alpha;
    int quiets = 0, played = 0, avoidedQS = 0, tableIsTactical = 0; 
    int best = -MATE, eval = -MATE, optimal = -MATE;
    int hist = 0; // Fix bogus GCC warning
    
    uint16_t currentMove, quietsTried[MAX_MOVES];
    uint16_t tableMove = NONE_MOVE, bestMove = NONE_MOVE;
    
    MovePicker movePicker;
    
    TransEntry * entry;
    Undo undo[1];
    
    PVariation lpv;
    lpv.length = 0;
    pv->length = 0;
    
    // Check to see if search time has expired
    if (Info->searchIsTimeLimited && getRealTime() >= Info->endTime2){
        Info->terminateSearch = 1;
        return board->turn == EvaluatingPlayer ? -MATE : MATE;
    }
    
    // Check to see if this line can't improve the mating / mated line
    min = alpha > -MATE + height     ? alpha : -MATE + height;
    max =  beta <  MATE - height - 1 ?  beta :  MATE - height - 1;
    if (min >= max) return min;
    
    // Check for the fifty move rule
    if (board->fiftyMoveRule > 100)
        return 0;
    
    // Check for three-fold repitition
    for (i = board->numMoves-2; i >= 0; i-=2)
        if (board->history[i] == board->hash)
            return 0;
    
    // Search horizon reached, go into Quiescence Search
    if (depth <= 0){
        
        // Don't jump into the qsearch if we are in check
        inCheck = !isNotInCheck(board, board->turn);
        if (inCheck) avoidedQS = 1;
        else return quiescenceSearch(board, alpha, beta, height);
    }
    
    // INCREMENT TOTAL NODE COUNTER
    TotalNodes++;
    
    // Lookup current position in transposition table
    if ((entry = getTranspositionEntry(&Table, board->hash)) != NULL){
        
        // Entry move may be good in this position
        tableMove = EntryMove(*entry);
        
        // Determine if the table move is tactical. This will
        // allow us to increase the LMR on quiet moves later on
        tableIsTactical = moveIsTactical(board, tableMove);
        
        // Determine if Table may cause a cut off. We could perform
        // cutoffs in PVNODEs, but doing so will truncate the PV line
        if (    EntryDepth(*entry) >= depth
            &&  nodeType != PVNODE){
                
            entryValue = valueFromTT(EntryValue(*entry), height);
            entryType = EntryType(*entry);
            
            min = alpha;
            max = beta;
            
            // Exact value stored
            if (entryType == PVNODE)
                return entryValue;
            
            // Lower bound stored
            else if (entryType == CUTNODE)
                min = entryValue > alpha ? entryValue : alpha;
            
            // Upper bound stored
            else if (entryType == ALLNODE)
                max = entryValue < beta ? entryValue : beta;
            
            // Bounds now overlap, therefore we can exit
            if (min >= max) return entryValue;
        }
    }
    
    // Determine check status if not done already
    if (!avoidedQS)
        inCheck = !isNotInCheck(board, board->turn);
    
    // If not in check and not in a PVNODE, we will need
    // a static eval for some pruning decisions later on
    if (nodeType != PVNODE && !inCheck){
        eval = evaluateBoard(board);
        optimal = eval + depth * 1.25 * PawnValue;
    }
    
    // Node Razoring at expected ALLNODEs
    if (    nodeType != PVNODE
        && !inCheck
        &&  depth <= 4
        && !tableIsTactical
        &&  hasNonPawnMaterial(board, board->turn)
        &&  eval + RazorMargins[depth] < alpha){
            
            
        if (depth <= 1)
            return quiescenceSearch(board, alpha, beta, height);
        
        value = quiescenceSearch(board, alpha - RazorMargins[depth], beta - RazorMargins[depth], height);
        
        if (value + RazorMargins[depth] < alpha)
            return value;
    }
    
    // Static null move pruning
    if (    depth <= 3
        &&  nodeType != PVNODE
        && !inCheck){
            
        value = eval - depth * (PawnValue + 15);
        
        if (value > beta)
            return value;
    }
    
    // NULL MOVE PRUNING
    if (    depth >= 2
        &&  nodeType != PVNODE
        &&  hasNonPawnMaterial(board, board->turn)
        && !inCheck
        &&  board->history[board->numMoves-1] != NULL_MOVE
        &&  eval >= beta){
            
        // Perform null move search
        applyNullMove(board, undo);
        value = -alphaBetaSearch(&lpv, board, -beta, -beta+1, depth-4, height+1, CUTNODE);
        revertNullMove(board, undo);
        
        if (value >= beta){
            if (value >= MATE - MAX_HEIGHT)
                value = beta;
            return value;
        }
    }
    
    // INTERNAL ITERATIVE DEEPING
    if (    depth >= 3
        &&  tableMove == NONE_MOVE
        &&  nodeType == PVNODE){
        
        // SEARCH AT A LOWER DEPTH
        value = alphaBetaSearch(&lpv, board, alpha, beta, depth-2, height, nodeType);
        if (value <= alpha)
            value = alphaBetaSearch(&lpv, board, -MATE, beta, depth-2, height, PVNODE);
        
        // Get the best move from the PV
        if (lpv.length >= 1) tableMove = lpv.line[0];
        
        // Update tableIsTactical for LMR
        tableIsTactical = moveIsTactical(board, tableMove);
    }
    
    // CHECK EXTENSION
    depth += inCheck && (nodeType == PVNODE || depth <= 6);
    
    initalizeMovePicker(&movePicker, 0, tableMove, KillerMoves[height][0], KillerMoves[height][1]);
    while ((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
        
        // If this move is quiet we will save it to a list of attemped
        // quiets, and we will need a history score for pruning decisions
        if ((isQuiet = !moveIsTactical(board, currentMove))){
            quietsTried[quiets++] = currentMove;
            hist = getHistoryScore(History, currentMove, board->turn, 128);
        }
        
        // Use futility pruning
        if (    nodeType != PVNODE
            &&  played >= 1
            &&  depth <= 8
            && !inCheck
            &&  isQuiet
            &&  optimal <= alpha)
            continue;
        
        // Apply and validate move before searching
        applyMove(board, currentMove, undo);
        if (!isNotInCheck(board, !board->turn)){
            revertMove(board, currentMove, undo);
            continue;
        }
        
        // Late Move Pruning
        if (    nodeType != PVNODE
            &&  played >= 1
            && !inCheck
            &&  isQuiet
            &&  depth <= 8
            &&  quiets > LateMovePruningCounts[depth]
            &&  isNotInCheck(board, board->turn)){
        
            revertMove(board, currentMove, undo);
            continue;
        }
        
        // Update counter of moves actually played
        played += 1;
    
        // DETERMINE IF WE CAN USE LATE MOVE REDUCTIONS
        if (    played >= 4
            &&  depth >= 3
            && !inCheck
            &&  isQuiet
            &&  isNotInCheck(board, board->turn)){
            
            R = 2;
            R += (played - 4) / 8;
            R += 2 * (nodeType != PVNODE);
            R += tableIsTactical && bestMove == tableMove;
            R -= hist / 24;
            R = R >= 1 ? R : 1;
        }
        
        else {
            R = 1;
        }
         
        // FULL WINDOW SEARCH ON FIRST MOVE
        if (played == 1 || nodeType != PVNODE){
            
            value = -alphaBetaSearch(&lpv, board, -beta, -alpha, depth-R, height+1, nodeType);
            
            // IMPROVED BOUND, BUT WAS REDUCED DEPTH?
            if (value > alpha && R != 1)
                value = -alphaBetaSearch(&lpv, board, -beta, -alpha, depth-1, height+1, nodeType);
        }
        
        // NULL WINDOW SEARCH ON NON-FIRST / PV MOVES
        else{
            value = -alphaBetaSearch(&lpv, board, -alpha-1, -alpha, depth-R, height+1, CUTNODE);
            
            // NULL WINDOW FAILED HIGH, RESEARCH
            if (value > alpha)
                value = -alphaBetaSearch(&lpv, board, -beta, -alpha, depth-1, height+1, PVNODE);
        }
        
        // REVERT MOVE FROM BOARD
        revertMove(board, currentMove, undo);
        
        // Improved current value
        if (value > best){
            best = value;
            bestMove = currentMove;
            
            // IMPROVED CURRENT LOWER VALUE
            if (value > alpha){
                alpha = value;
                
                // Update the Principle Variation
                pv->length = 1 + lpv.length;
                pv->line[0] = currentMove;
                memcpy(pv->line + 1, lpv.line, sizeof(uint16_t) * lpv.length);
            }
        }
        
        // IMPROVED AND FAILED HIGH
        if (alpha >= beta){
            
            // Update killer moves
            if (isQuiet && KillerMoves[height][0] != currentMove){
                KillerMoves[height][1] = KillerMoves[height][0];
                KillerMoves[height][0] = currentMove;
            }
            
            break;
        }
    }
    
    // Board is Checkmate or Stalemate
    if (played == 0) return inCheck ? -MATE + height : 0;
    
    // Update History Scores
    else if (best >= beta && !moveIsTactical(board, bestMove)){
        updateHistory(History, bestMove, board->turn, 1, depth*depth);
        for (i = 0; i < quiets - 1; i++)
            updateHistory(History, quietsTried[i], board->turn, 0, depth*depth);
    }
    
    // Store results in transposition table
    if (!Info->searchIsTimeLimited || getRealTime() < Info->endTime2)
        storeTranspositionEntry(&Table, depth, (best > oldAlpha && best < beta)
                                ? PVNODE : best >= beta ? CUTNODE : ALLNODE,
                                valueToTT(best, height), bestMove, board->hash);
    return best;
}

int quiescenceSearch(Board * board, int alpha, int beta, int height){
    
    int eval, value, best, maxValueGain;
    uint16_t currentMove;
    Undo undo[1];
    MovePicker movePicker;
    
    // Max height reached, stop here
    if (height >= MAX_HEIGHT)
        return evaluateBoard(board);
    
    TotalNodes++;
    
    // Get a standing eval of the current board
    best = value = eval = evaluateBoard(board);
    
    // Update lower bound
    if (value > alpha) alpha = value;
    
    // QSearch can be terminated
    if (alpha >= beta) return value;
    
    // Take a guess at the best case gain for a non promotion capture
    if (board->colours[!board->turn] & board->pieces[QUEEN])
        maxValueGain = QueenValue + 55;
    else if (board->colours[!board->turn] & board->pieces[ROOK])
        maxValueGain = RookValue + 35;
    else
        maxValueGain = BishopValue + 15;
    
    // Delta pruning when no promotions and not extreme late game
    if (    value + maxValueGain < alpha
        &&  popcount(board->colours[WHITE] | board->colours[BLACK]) >= 6
        && !(board->colours[WHITE] & board->pieces[PAWN] & RANK_7)
        && !(board->colours[BLACK] & board->pieces[PAWN] & RANK_2))
        return value;
    
    
    initalizeMovePicker(&movePicker, 1, NONE_MOVE, NONE_MOVE, NONE_MOVE);
    
    while ((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
        
        // Take a guess at the best case value of this current move
        value = eval + 55 + PieceValues[PieceType(board->squares[MoveTo(currentMove)])];
        if (MoveType(currentMove) == PROMOTION_MOVE)
            value += PieceValues[1 + (MovePromoType(currentMove) >> 14)];
        
        // If the best case is not good enough, continue
        if (value < alpha)
            continue;
        
        // Apply and validate move before searching
        applyMove(board, currentMove, undo);
        if (!isNotInCheck(board, !board->turn)){
            revertMove(board, currentMove, undo);
            continue;
        }
        
        // Search next depth
        value = -quiescenceSearch(board, -beta, -alpha, height+1);
        
        // Revert move from board
        revertMove(board, currentMove, undo);
        
        // Improved current value
        if (value > best){
            best = value;
            
            // Improved current lower value
            if (value > alpha)
                alpha = value;
        }
        
        // Search has failed high
        if (alpha >= beta)
            return best;
    }
    
    return best;
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

int moveIsTactical(Board * board, uint16_t move){
    return board->squares[MoveTo(move)] != EMPTY
        || MoveType(move) == PROMOTION_MOVE
        || MoveType(move) == ENPASS_MOVE;
}

int hasNonPawnMaterial(Board * board, int turn){
    uint64_t friendly = board->colours[turn];
    uint64_t kings = board->pieces[KING];
    uint64_t pawns = board->pieces[PAWN];
    return (friendly & (kings | pawns)) != friendly;
}

int valueFromTT(int value, int height){
    return value >=  MATE - MAX_HEIGHT ? value - height
         : value <= -MATE + MAX_HEIGHT ? value + height
         : value;
}

int valueToTT(int value, int height){
    return value >=  MATE - MAX_HEIGHT ? value + height
         : value <= -MATE + MAX_HEIGHT ? value - height
         : value;
}