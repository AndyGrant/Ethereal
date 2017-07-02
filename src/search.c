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

TransTable Table;
PawnTable PTable;

uint16_t KillerMoves[MAX_HEIGHT][2];

/**
 * Determine the best move for the current position. Information about
 * the position, as well as the parameters of the search, are provided
 * in the info parameter. We will continue searching deeper and deeper
 * until one of our terminatiation conditions becomes true.
 *
 * @param   info    Information about the Board and the search parameters
 *
 * @return          The best move we can come up with
 */
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
    clearHistory(History);
    
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

/**
 * Wrap the calls to rootSearch within a series of updating apsiration windows.
 * Current window margins of [30, 60, 120, 240] are quite arbitrary. Extensive
 * testing could likely find a better initial window and window updates. If no
 * window returns a valid score, we are forced to do a full windowed search.
 * 
 * @param   pv          Main principle variation line
 * @param   board       Board that we are searching on
 * @param   moveList    List of moves from the root of the search
 * @param   depth       Depth of this particular search
 * @param   lastScore   Score from the previous depth
 *
 * @return              Value of the search
 */
int aspirationWindow(PVariation * pv, Board * board, MoveList * moveList, 
                                               int depth, int lastScore){
    
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


int rootSearch(PVariation * pv, Board * board, MoveList * moveList, int alpha,
                                                         int beta, int depth){
    
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

int alphaBetaSearch(PVariation * pv, Board * board, int alpha, int beta, 
                                   int depth, int height, int nodeType){
    
    int i, value, newDepth, entryValue, entryType;
    int min, max, inCheck;
    int valid = 0, avoidedQS = 0, eval = 0;
    int oldAlpha = alpha, best = -MATE, optimalValue = -MATE;
    
    uint16_t currentMove, tableMove = NONE_MOVE, bestMove = NONE_MOVE;
    uint16_t killer1, killer2, played[MAX_MOVES];
    
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
    
    // Check for the fifty move rule
    if (board->fiftyMoveRule > 100)
        return 0;
    
    // Check for three-fold repitition
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

        else
            return quiescenceSearch(board, alpha, beta, height);
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
    
    if (nodeType != PVNODE && !inCheck)
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
        value = -alphaBetaSearch(&lpv, board, -beta, -beta+1, depth-4, height+1, CUTNODE);
        
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
        value = alphaBetaSearch(&lpv, board, alpha, beta, depth-2, height, nodeType);
        if (value <= alpha)
            value = alphaBetaSearch(&lpv, board, -MATE, beta, depth-2, height, PVNODE);
        
        // Get the best move from the PV
        tableMove = lpv.line[0];
    }
    
    // CHECK EXTENSION
    depth += !avoidedQS && inCheck && (nodeType == PVNODE || depth <= 6);
    
    // Setup the Move Picker
    killer1 = KillerMoves[height][0];
    killer2 = KillerMoves[height][1];
    initalizeMovePicker(&movePicker, 0, tableMove, killer1, killer2);
    
    while((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
        
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
            && !inCheck
            && MoveType(currentMove) == NORMAL_MOVE
            && undo[0].capturePiece == EMPTY
            && getHistoryScore(History, currentMove, !board->turn, 100) < 80
            && isNotInCheck(board, board->turn))
            newDepth = depth - 2 - (valid >= 12) - (nodeType != PVNODE);
        else
            newDepth = depth-1;
         
        // FULL WINDOW SEARCH ON FIRST MOVE
        if (valid == 1 || nodeType != PVNODE){
            
            value = -alphaBetaSearch(&lpv, board, -beta, -alpha, newDepth, height+1, nodeType);
            
            // IMPROVED BOUND, BUT WAS REDUCED DEPTH?
            if (value > alpha
                && newDepth != depth-1){
                    
                value = -alphaBetaSearch(&lpv, board, -beta, -alpha, depth-1, height+1, nodeType);
            }
        }
        
        // NULL WINDOW SEARCH ON NON-FIRST / PV MOVES
        else{
            value = -alphaBetaSearch(&lpv, board, -alpha-1, -alpha, newDepth, height+1, CUTNODE);
            
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
            
            // UPDATE KILLER MOVES
            if (MoveType(currentMove) == NORMAL_MOVE
                && undo[0].capturePiece == EMPTY
                && KillerMoves[height][0] != currentMove){
                KillerMoves[height][1] = KillerMoves[height][0];
                KillerMoves[height][0] = currentMove;
            }
            
            break;
        }
    }
    
    // Board is Checkmate or Stalemate
    if (valid == 0) return inCheck ? -MATE + height : 0;
    
    if (best >= oldAlpha && bestMove != NONE_MOVE)
        updateHistory(History, bestMove, board->turn, 1, depth*depth);
    
    for (i = valid - 1; i >= 0; i--)
        if (played[i] != bestMove)
            updateHistory(History, played[i], board->turn, 0, depth*depth);
    
    
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
    
    int eval, value, best, maxValueGain;
    uint16_t currentMove;
    Undo undo[1];
    MovePicker movePicker;
    
    // Max height reached, stop here
    if (height >= MAX_HEIGHT)
        return evaluateBoard(board, &PTable);
    
    TotalNodes++;
    
    // Get a standing eval of the current board
    best = value = eval = evaluateBoard(board, &PTable);
    
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
    if (value + maxValueGain < alpha
        && popcount(board->colours[WHITE] | board->colours[BLACK]) >= 6
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

int canDoNull(Board * board){
    uint64_t friendly = board->colours[board->turn];
    uint64_t kings = board->pieces[5];
    uint64_t pawns = board->pieces[0];
    
    return (friendly & (kings | pawns)) != friendly;
}