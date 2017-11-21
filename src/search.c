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
    
    uint64_t lastBestMove = NONE_MOVE;
    int i, depth, elapsed, hashfull, lastValue = 0, value = 0;
    
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
    
    // Perform interative deepening
    for (depth = 1; depth < MAX_DEPTH; depth++){
        
        // Perform full search on Root
        value = aspirationWindow(&pv, &info->board, depth, value);
        
        if (depth >= 4 && lastValue > value + 8)
            info->idealTimeUsage = MIN(info->maxTimeUsage, info->idealTimeUsage * 1.10);
        
        if (depth >= 4 && pv.line[0] != lastBestMove)
            info->idealTimeUsage = MIN(info->maxTimeUsage, info->idealTimeUsage * 1.35);
        
        lastValue = value;
        lastBestMove = pv.line[0];
        
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
            if (getRealTime() > info->startTime + info->idealTimeUsage) break;
            if (getRealTime() > info->startTime + info->maxTimeUsage) break;
        }
    }
    
    // Free the Pawn Table
    destoryPawnTable(&PTable);
    
    return pv.line[0];
}

int aspirationWindow(PVariation * pv, Board * board, int depth, int lastScore){
    
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
            value = search(pv, board, alpha, beta, depth, 0);
            
            // Result was within our window
            if (value > alpha && value < beta)
                return value;
            
            // Result was a mate score, force a full search
            if (abs(value) > MATE/2)
                break;
        }
    }
    
    // No searches scored within our aspiration windows, search full window
    return search(pv, board, -MATE, MATE, depth, 0);
}

int search(PVariation * pv, Board * board, int alpha, int beta, int depth, int height){
    
    const int PvNode   = (alpha != beta - 1);
    const int RootNode = (height == 0);
    
    int i, value, inCheck = 0, isQuiet, R, repetitions;
    int rAlpha, rBeta, ttValue, ttType, oldAlpha = alpha;
    int quiets = 0, played = 0, ttTactical = 0; 
    int best = -MATE, eval = -MATE, futilityMargin = -MATE;
    int hist = 0; // Fix bogus GCC warning
    
    uint16_t currentMove, quietsTried[MAX_MOVES];
    uint16_t ttMove = NONE_MOVE, bestMove = NONE_MOVE;
    
    MovePicker movePicker;
    
    TransEntry * ttEntry;
    Undo undo[1];
    
    PVariation lpv;
    lpv.length = 0;
    pv->length = 0;
    
    // Step 1. Check to see if search time has expired
    if (    Info->searchIsTimeLimited 
        && (TotalNodes & 8191) == 8191
        &&  getRealTime() >= Info->startTime + Info->maxTimeUsage){
        Info->terminateSearch = 1;
        return board->turn == EvaluatingPlayer ? -MATE : MATE;
    }
    
    // Step 2. Distance Mate Pruning. Check to see if this line is so
    // good, or so bad, that being mated in the ply, or  mating in 
    // the next one, would still not create a more extreme line
    rAlpha = alpha > -MATE + height     ? alpha : -MATE + height;
    rBeta  =  beta <  MATE - height - 1 ?  beta :  MATE - height - 1;
    if (rAlpha >= rBeta) return rAlpha;
    
    // Step 3. Check for the Fifty Move Rule
    if (board->fiftyMoveRule > 100)
        return 0;
    
    // Step 4. Check for three fold repetition. If the repetition occurs since
    // the root move of this search, we will exit early as if it was a draw.
    // Otherwise, we will look for an actual three fold repetition draw.
    for (repetitions = 0, i = board->numMoves - 2; i >= 0; i -= 2){
        
        // We can't have repeated positions before the most recent
        // move which triggered a reset of the fifty move rule counter
        if (i < board->numMoves - board->fiftyMoveRule) break;
        
        if (board->history[i] == board->hash){
            
            // Repetition occured after the root
            if (i > board->numMoves - height)
                return 0;
            
            // An actual three fold repetition
            if (++repetitions == 2)
                return 0;
        }
    }
    
    // Step 5. Go into the Quiescence Search if we have reached
    // the search horizon and are not currently in check
    if (depth <= 0){
        inCheck = !isNotInCheck(board, board->turn);
        if (!inCheck) return qsearch(pv, board, alpha, beta, height);
    }
    
    // INCREMENT TOTAL NODE COUNTER
    TotalNodes++;
    
    // Step 6. Probe the Transposition Table for an entry
    if ((ttEntry = getTranspositionEntry(&Table, board->hash)) != NULL){
        
        // Entry move may be good in this position. If it is tactical,
        // we may use it to increase reductions later on in LMR.
        ttMove = EntryMove(*ttEntry);
        ttTactical = moveIsTactical(board, ttMove);
        
        // Step 6A. Check to see if this entry allows us to exit this
        // node early. We choose not to do this in the PV line, not because
        // we can't, but because don't want truncated PV lines
        if (!PvNode && EntryDepth(*ttEntry) >= depth){
                
            ttValue = valueFromTT(EntryValue(*ttEntry), height);
            ttType = EntryType(*ttEntry);
            rAlpha = alpha;
            rBeta = beta;
            
            switch (ttType){
                case  PVNODE: return ttValue;
                case CUTNODE: rAlpha = ttValue > alpha ? ttValue : alpha; break;
                case ALLNODE:  rBeta = ttValue <  beta ? ttValue :  beta; break;
            }
            
            // Entry allows early exit
            if (rAlpha >= rBeta) return ttValue;
        }
    }
    
    // Step 7. Determine check status, and calculate the futility margin.
    // We only need the futility margin if we are not in check, and we
    // are not looking at a PV Node, as those are not subject to futility.
    // Determine check status if not done already
    inCheck = inCheck || !isNotInCheck(board, board->turn);
    if (!PvNode && !inCheck){
        eval = evaluateBoard(board);
        futilityMargin = eval + depth * 0.95 * PieceValues[PAWN][EG];
    }
    
    // Step 8. Razoring. If a Quiescence Search for the current position
    // still falls way below alpha, we will assume that the score from
    // the Quiescence search was sufficient. For depth 1, we will just
    // return a Quiescence Search score because it is unlikely a quiet
    // move would close the massive gap between the evaluation and alpha
    if (   !PvNode
        && !inCheck
        && !ttTactical
        &&  depth <= RazorDepth
        &&  eval + RazorMargins[depth] < alpha
        &&  hasNonPawnMaterial(board, board->turn)){
            
        if (depth <= 1)
            return qsearch(pv, board, alpha, beta, height);
        
        rAlpha = alpha - RazorMargins[depth];
        value = qsearch(pv, board, rAlpha, rAlpha + 1, height);
        if (value <= rAlpha) return value;
    }
    
    // Step 9. Beta Pruning / Reverse Futility Pruning / Static Null
    // Move Pruning. If the eval is few pawns above beta then exit early
    if (   !PvNode
        && !inCheck
        && depth <= BetaPruningDepth){
            
        value = eval - depth * 0.95 * PieceValues[PAWN][EG];
        
        if (value > beta)
            return value;
    }
    
    // Step 10. Null Move Pruning. If our position is so good that
    // giving our opponent back-to-back moves is still not enough
    // for them to gain control of the game, we can be somewhat safe
    // in saying that our position is too good to be true
    if (   !PvNode
        && !inCheck
        &&  depth >= NullMovePruningDepth
        &&  eval >= beta
        &&  hasNonPawnMaterial(board, board->turn)
        &&  board->history[board->numMoves-1] != NULL_MOVE){
            
        applyNullMove(board, undo);
        
        value = -search(&lpv, board, -beta, -beta+1, depth-4, height+1);
        
        revertNullMove(board, undo);
        
        if (value >= beta){
            if (value >= MATE - MAX_HEIGHT)
                value = beta;
            return value;
        }
    }
    
    // Step 11. Internal Iterative Deepening. Searching PV nodes without
    // a known good move can be expensive, so a reduced search first
    if (    PvNode
        &&  ttMove == NONE_MOVE
        &&  depth >= InternalIterativeDeepeningDepth){
        
        // Search with a reduced depth
        value = search(&lpv, board, alpha, beta, depth-2, height);
        
        // Probe for the newly found move, and update ttMove / ttTactical
        if ((ttEntry = getTranspositionEntry(&Table, board->hash)) != NULL){
            ttMove = EntryMove(*ttEntry);
            ttTactical = moveIsTactical(board, ttMove);
        }
    }
    
    // Step 12. Check Extension
    depth += inCheck && !RootNode && (PvNode || depth <= 6);
    
    initalizeMovePicker(&movePicker, 0, ttMove, KillerMoves[height][0], KillerMoves[height][1]);
    while ((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
        
        // If this move is quiet we will save it to a list of attemped
        // quiets, and we will need a history score for pruning decisions
        if ((isQuiet = !moveIsTactical(board, currentMove))){
            quietsTried[quiets++] = currentMove;
            hist = getHistoryScore(History, currentMove, board->turn, 128);
        }
        
        // Step 13. Futility Pruning. If our score is far below alpha,
        // and we don't expect anything from this move, skip it.
        if (   !PvNode
            && !inCheck
            &&  isQuiet
            &&  played >= 1
            &&  futilityMargin <= alpha
            &&  depth <= FutilityPruningDepth)
            continue;
        
        // Apply and validate move before searching
        applyMove(board, currentMove, undo);
        if (!isNotInCheck(board, !board->turn)){
            revertMove(board, currentMove, undo);
            continue;
        }
        
        // Step 14. Late Move Pruning / Move Count Pruning. If we have
        // tried many quiets in this position already, and we don't expect
        // anything from this move, we can undo it and move on.
        if (   !PvNode
            && !inCheck
            &&  isQuiet
            &&  played >= 1
            &&  depth <= LateMovePruningDepth
            &&  quiets > LateMovePruningCounts[depth]
            &&  isNotInCheck(board, board->turn)){
        
            revertMove(board, currentMove, undo);
            continue;
        }
        
        // Update counter of moves actually played
        played += 1;
    
        // Step 15. Late Move Reductions. We will search some moves at a
        // lower depth. If they look poor at a lower depth, then we will
        // move on. If they look good, we will search with a full depth.
        if (    played >= 4
            &&  depth >= 3
            && !inCheck
            &&  isQuiet
            &&  isNotInCheck(board, board->turn)){
            
            R = 2;
            R -= RootNode;
            R += (played - 4) / 8;
            R += (depth  - 4) / 6;
            R += 2 * !PvNode;
            R += ttTactical && bestMove == ttMove;
            R -= hist / 24;
            R = R >= 1 ? R : 1;
        }
        
        else {
            R = 1;
        }
        
        // Search the move with a possibly reduced depth, on a full or null window
        value =  (played == 1 || !PvNode)
               ? -search(&lpv, board, -beta, -alpha, depth-R, height+1)
               : -search(&lpv, board, -alpha-1, -alpha, depth-R, height+1);
               
        // If the search beat alpha, we may need to research, in the event that
        // the previous search was not the full window, or was a reduced depth
        value =  (value > alpha && (R != 1 || (played != 1 && PvNode)))
               ? -search(&lpv, board, -beta, -alpha, depth-1, height+1)
               :  value;
        
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
    if (!Info->terminateSearch)
        storeTranspositionEntry(&Table, depth, (best > oldAlpha && best < beta)
                                ? PVNODE : best >= beta ? CUTNODE : ALLNODE,
                                valueToTT(best, height), bestMove, board->hash);
    return best;
}

int qsearch(PVariation * pv, Board * board, int alpha, int beta, int height){
    
    int eval, value, best, maxValueGain;
    uint16_t currentMove;
    Undo undo[1];
    MovePicker movePicker;
    
    PVariation lpv;
    lpv.length = 0;
    pv->length = 0;
    
    // Check to see if search time has expired
    if (    Info->searchIsTimeLimited 
        && (TotalNodes & 8191) == 8191
        &&  getRealTime() >= Info->startTime + Info->maxTimeUsage){
        Info->terminateSearch = 1;
        return board->turn == EvaluatingPlayer ? -MATE : MATE;
    }
    
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
        maxValueGain = PieceValues[QUEEN ][EG] + 55;
    else if (board->colours[!board->turn] & board->pieces[ROOK])
        maxValueGain = PieceValues[ROOK  ][EG] + 35;
    else
        maxValueGain = PieceValues[BISHOP][EG] + 15;
    
    // Delta pruning when no promotions and not extreme late game
    if (    value + maxValueGain < alpha
        &&  popcount(board->colours[WHITE] | board->colours[BLACK]) >= 6
        && !(board->colours[WHITE] & board->pieces[PAWN] & RANK_7)
        && !(board->colours[BLACK] & board->pieces[PAWN] & RANK_2))
        return value;
    
    
    initalizeMovePicker(&movePicker, 1, NONE_MOVE, NONE_MOVE, NONE_MOVE);
    
    while ((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
        
        // Take a guess at the best case value of this current move
        value = eval + 55 + PieceValues[PieceType(board->squares[MoveTo(currentMove)])][EG];
        if (MoveType(currentMove) == PROMOTION_MOVE){
            value += PieceValues[1 + (MovePromoType(currentMove) >> 14)][EG];
            value -= PieceValues[PAWN][EG];
        }
        
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
        value = -qsearch(&lpv, board, -beta, -alpha, height+1);
        
        // Revert move from board
        revertMove(board, currentMove, undo);
        
        // Improved current value
        if (value > best){
            best = value;
            
            // Improved current lower bound
            if (value > alpha){
                alpha = value;
                
                // Update the Principle Variation
                pv->length = 1 + lpv.length;
                pv->line[0] = currentMove;
                memcpy(pv->line + 1, lpv.line, sizeof(uint16_t) * lpv.length);
            }
        }
        
        // Search has failed high
        if (alpha >= beta)
            return best;
    }
    
    return best;
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
