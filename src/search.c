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
#include <pthread.h>
#include <setjmp.h>
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
#include "thread.h"
#include "transposition.h"
#include "types.h"
#include "time.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "uci.h"

pthread_mutex_t LOCK = PTHREAD_MUTEX_INITIALIZER;

extern TransTable Table;

uint16_t getBestMove(Thread* threads, Board* board, Limits* limits, double time, double mtg){
    
    int i, nthreads = threads[0].nthreads;
    
    SearchInfo info; memset(&info, 0, sizeof(SearchInfo));
    
    pthread_t* pthreads = malloc(sizeof(pthread_t) * nthreads);
    
    
    // Save start point of search for reporting and time managment
    info.starttime = getRealTime();
    
    // Ethereal is responsible for choosing how much time to spend searching
    if (limits->limitedBySelf){
        info.idealusage = mtg >= 0 ? 0.5 * (time / (mtg + 3)) : 0.3 * (time / 25);
        info.maxusage   = mtg >= 0 ? 2.8 * (time / (mtg + 1)) : 4.5 * (time / 25);
        info.idealusage = MIN(info.idealusage, time - 20);
        info.maxusage   = MIN(info.maxusage,   time - 20);
    }
    
    // UCI command told us to look for exactly X seconds
    if (limits->limitedByTime){
        info.idealusage = limits->timeLimit;
        info.maxusage   = limits->timeLimit;
    }
    
    // Setup the thread pool for a new search with these parameters
    newSearchThreadPool(threads, board, limits, &info);
    
    // Launch all of the threads
    for (i = 1; i < nthreads; i++)
        pthread_create(&pthreads[i], NULL, &iterativeDeepening, &threads[i]);
    iterativeDeepening((void*) &threads[0]);
    
    // Wait for all (helper) threads to finish
    for (i = 1; i < nthreads; i++)
        pthread_join(pthreads[i], NULL);
    
    // Cleanup pthreads
    free(pthreads);
    
    // Return highest depth best move
    return info.bestmoves[info.depth];
}

void* iterativeDeepening(void* vthread){
    
    Thread* const thread   = (Thread*) vthread;
    
    SearchInfo* const info = thread->info;
    
    Limits* const limits   = thread->limits;
   
    const int mainThread   = thread == &thread->threads[0];
    
    int i, count, value, depth, abort;
    
    
    for (depth = 1; depth < MAX_DEPTH; depth++){
        
        // Always acquire the lock before setting thread->depth. thread->depth
        // is needed by others to determine when to skip certain search iterations
        pthread_mutex_lock(&LOCK);
        
        thread->depth = depth;
        
        // Helper threads are subject to skipping depths in order to better help
        // the main thread, based on the number of threads already some depths
        if (!mainThread){
        
            for (count = 0, i = 1; i < thread->nthreads; i++)
                count += thread != &thread->threads[i] && thread->threads[i].depth >= depth;

            if (depth > 1 && thread->nthreads > 1 && count >= thread->nthreads / 2){
                thread->depth = depth + 1;
                pthread_mutex_unlock(&LOCK);
                continue;
            }
        }
        
        // Drop the lock as we have finished depth scheduling
        pthread_mutex_unlock(&LOCK);
        
        // Set the abort location. If we exit the search unnaturally (very likely)
        // we will know by abort being set, and can therefore indicate an exit
        abort = setjmp(thread->jbuffer);
        if (abort) return NULL;
        
            
        // Perform the actual search for the current depth
        value = aspirationWindow(thread, depth);
        
        // Helper threads need not worry about time and search info updates
        if (!mainThread) continue;
        
        // Update the Search Info structure for the main thread
        info->depth = depth;
        info->values[depth] = value;
        info->bestmoves[depth] = thread->pv.line[0];
        info->timeUsage[depth] = getRealTime() - info->starttime - info->timeUsage[depth-1];
        
        // Send information about this search to the interface
        uciReport(thread->threads, info->starttime, depth, value, &thread->pv);
        
        // If Ethereal is managing the clock, determine if we should be spending
        // more time on this search, based on the score difference between iterations
        // and any changes in the principle variation since the last iteration
        if (limits->limitedBySelf){
            
            // Increase our time if the score suddently dropped by eight centipawns
            if (depth >= 4 && info->values[depth - 1] > value + 8)
                info->idealusage = MIN(info->maxusage, info->idealusage * 1.10);
            
            // Increase our time if the pv has changed across the last two iterations
            if (depth >= 4 && info->bestmoves[depth - 1] != thread->pv.line[0])
                info->idealusage = MIN(info->maxusage, info->idealusage * 1.35);
        }
        
        // Check for termination by any of the possible limits
        if (   (limits->limitedByDepth && depth >= limits->depthLimit)
            || (limits->limitedByTime  && getRealTime() - info->starttime > limits->timeLimit)
            || (limits->limitedBySelf  && getRealTime() - info->starttime > info->maxusage)
            || (limits->limitedBySelf  && getRealTime() - info->starttime > info->idealusage)){
            
            // Terminate all helper threads
            for (i = 0; i < thread->nthreads; i++)
                thread->threads[i].abort = 1;
            return NULL;
        }
        
        // Check to see if we expect to be able to complete the next depth
        if (thread->limits->limitedBySelf){
            double timeFactor = MIN(2, info->timeUsage[depth] / MAX(1, info->timeUsage[depth-1]));
            double estimatedUsage = info->timeUsage[depth] * (timeFactor + .25);
            double estiamtedEndtime = getRealTime() + estimatedUsage - info->starttime;
            
            if (estiamtedEndtime > info->maxusage){
                
                // Terminate all helper threads
                for (i = 0; i < thread->nthreads; i++)
                    thread->threads[i].abort = 1;
                return NULL;
            }
        }
    }
    
    return NULL;
}

int aspirationWindow(Thread* thread, int depth){
    
    int alpha, beta, value, upper, lower;
    
    int* const values = thread->info->values;
    
    int mainDepth = MAX(5, 1 + thread->info->depth);
    
    // Aspiration window only after we have completed the first four
    // depths, and so long as the last score is not near a mate score
    if (depth > 4 && abs(values[mainDepth-1]) < MATE / 2){
        
        // Dynamically compute the upper margin based on previous scores
        upper = MAX(    4,  1.6 * (values[mainDepth-1] - values[mainDepth-2]));
        upper = MAX(upper,  2.0 * (values[mainDepth-2] - values[mainDepth-3]));
        upper = MAX(upper,  0.8 * (values[mainDepth-3] - values[mainDepth-4]));
        
        // Dynamically compute the lower margin based on previous scores
        lower = MAX(    4, -1.6 * (values[mainDepth-1] - values[mainDepth-2]));
        lower = MAX(lower, -2.0 * (values[mainDepth-2] - values[mainDepth-3]));
        lower = MAX(lower, -0.8 * (values[mainDepth-3] - values[mainDepth-4])); 
        
        // Create the aspiration window
        alpha = values[mainDepth-1] - lower;
        beta  = values[mainDepth-1] + upper;
        
        // Try windows until lower or upper bound exceeds a limit
        for (; lower <= 640 && upper <= 640; lower *= 2, upper *= 2){
            
            // Perform the search on the modified window
            value = search(thread, &thread->pv, alpha, beta, depth, 0);
            
            // Result was within our window
            if (value > alpha && value < beta)
                return value;
            
            // Search failed low
            if (value <= alpha){
                beta  = (alpha + beta) / 2;
                alpha = alpha - 2 * lower;
            }
            
            // Search failed high
            if (value >= beta){
                alpha = (alpha + beta) / 2;
                beta  = beta + 2 * upper;
            }
            
            // Result was a near mate score, force a full search
            if (abs(value) > MATE / 2)
                break;
        }
    }
    
    // Full window search when near mate or when depth is below or equal to 4
    return search(thread, &thread->pv, -MATE, MATE, depth, 0);
}

int search(Thread* thread, PVariation* pv, int alpha, int beta, int depth, int height){
    
    const int PvNode   = (alpha != beta - 1);
    const int RootNode = (height == 0);
    
    Board* const board = &thread->board;
    
    int i, value, inCheck = 0, isQuiet, R, repetitions;
    int rAlpha, rBeta, ttValue, oldAlpha = alpha;
    int quiets = 0, played = 0, ttTactical = 0; 
    int best = -MATE, eval = -MATE, futilityMargin = -MATE;
    int hist = 0; // Fix bogus GCC warning
    
    uint16_t currentMove, quietsTried[MAX_MOVES];
    uint16_t ttMove = NONE_MOVE, bestMove = NONE_MOVE;
    
    Undo undo[1];
    EvalInfo ei;
    PVariation lpv;
    TransEntry ttEntry;
    MovePicker movePicker;
    
    lpv.length = 0;
    pv->length = 0;
    
    // Step 1A. Check to see if search time has expired
    if (   (thread->limits->limitedBySelf || thread->limits->limitedByTime)
        && (thread->nodes & 8191) == 8191
        &&  getRealTime() >= thread->info->starttime + thread->info->maxusage)
        longjmp(thread->jbuffer, 1);
        
    // Step 1B. Check to see if the master thread finished
    if (thread->abort) longjmp(thread->jbuffer, 1);
    
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
        if (!inCheck) return qsearch(thread, pv, alpha, beta, height);
        
        // We do not cap reductions, so here we will make
        // sure that depth is within the acceptable bounds
        depth = 0; 
    }
    
    // If we did not exit already, we will call this a node
    thread->nodes += 1;
    
    // Step 6. Probe the Transposition Table for an entry
    if (getTranspositionEntry(&Table, board->hash, &ttEntry)){
        
        // Entry move may be good in this position. If it is tactical,
        // we may use it to increase reductions later on in LMR.
        ttMove = ttEntry.bestMove;
        ttTactical = moveIsTactical(board, ttMove);
        
        // Step 6A. Check to see if this entry allows us to exit this
        // node early. We choose not to do this in the PV line, not because
        // we can't, but because don't want truncated PV lines
        if (!PvNode && ttEntry.depth >= depth){

            rAlpha = alpha; rBeta = beta;
            ttValue = valueFromTT(ttEntry.value, height);
            
            switch (ttEntry.type){
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
    if (!PvNode){
        eval = evaluateBoard(board, &ei, &thread->ptable);
        futilityMargin = eval + depth * 0.95 * PieceValues[PAWN][EG];
    }
    
    // Step 8. Razoring. If a Quiescence Search for the current position
    // still falls way below alpha, we will assume that the score from
    // the Quiescence search was sufficient. For depth 1, we will just
    // return a Quiescence Search score because it is unlikely a quiet
    // move would close the massive gap between the evaluation and alpha
    if (   !PvNode
        && !inCheck
        &&  depth <= RazorDepth
        &&  eval + RazorMargins[depth] < alpha){
            
        if (depth <= 1)
            return qsearch(thread, pv, alpha, beta, height);
        
        rAlpha = alpha - RazorMargins[depth];
        value = qsearch(thread, pv, rAlpha, rAlpha + 1, height);
        if (value <= rAlpha) return value;
    }
    
    // Step 9. Beta Pruning / Reverse Futility Pruning / Static Null
    // Move Pruning. If the eval is few pawns above beta then exit early
    if (   !PvNode
        && !inCheck
        &&  depth <= BetaPruningDepth
        &&  hasNonPawnMaterial(board, board->turn)){
            
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
            
        R = 4 + depth / 6 + (eval - beta + 200) / 400; 
            
        applyNullMove(board, undo);
        
        value = -search(thread, &lpv, -beta, -beta + 1, depth - R, height + 1);
        
        revertNullMove(board, undo);
        
        if (value >= beta){
            if (value >= MATE - MAX_HEIGHT)
                value = beta;
            return value;
        }
    }
    
    // Step 11. ProbCut. If we have a good capture that causes a beta cutoff
    // with a slightly reduced depth search it is likely that this capture is
    // likely going to be good at a full depth. To save some work we will prune
    // captures that won't exceed rbeta or captures that fail at a low depth
    if (   !PvNode
        && !inCheck
        &&  depth >= 5){
            
        int rbeta = MIN(beta + 150, MATE - MAX_HEIGHT - 1);
            
        initializeMovePicker(&movePicker, thread, NONE_MOVE, height, 1);
        
        while ((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
            
            // Skip this capture if the raw value gained from a capture will
            // not exceed rbeta, making it unlikely to cause the desired cutoff
            if (eval + PieceValues[PieceType(board->squares[MoveTo(currentMove)])][MG] <= rbeta)
                continue;
            
            // Apply and validate move before searching
            applyMove(board, currentMove, undo);
            if (!isNotInCheck(board, !board->turn)){
                revertMove(board, currentMove, undo);
                continue;
            }
            
            // Verify the move is good with a depth zero search (qsearch, unless in check)
            // and then with a slightly reduced search. If both searches still exceed rbeta,
            // we will prune this node's subtree with resonable assurance that we made no error
            if (   -search(thread, &lpv, -rbeta, -rbeta+1,       0, height+1) >= rbeta
                && -search(thread, &lpv, -rbeta, -rbeta+1, depth-4, height+1) >= rbeta){
                    
                revertMove(board, currentMove, undo);
                return beta;
            }
             
            // Revert the board state
            revertMove(board, currentMove, undo);
        }
    }
    
    // Step 12. Internal Iterative Deepening. Searching PV nodes without
    // a known good move can be expensive, so a reduced search first
    if (    PvNode
        &&  ttMove == NONE_MOVE
        &&  depth >= InternalIterativeDeepeningDepth){
        
        // Search with a reduced depth
        value = search(thread, &lpv, alpha, beta, depth-2, height);
        
        // Probe for the newly found move, and update ttMove / ttTactical
        if (getTranspositionEntry(&Table, board->hash, &ttEntry)){
            ttMove = ttEntry.bestMove;
            ttTactical = moveIsTactical(board, ttMove);
        }
    }
    
    // Step 13. Check Extension at non Root nodes that are PV or low depth
    depth += inCheck && !RootNode && (PvNode || depth <= 6);
    
    
    initializeMovePicker(&movePicker, thread, ttMove, height, 0);
    
    while ((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
        
        // If this move is quiet we will save it to a list of attemped
        // quiets, and we will need a history score for pruning decisions
        if ((isQuiet = !moveIsTactical(board, currentMove))){
            quietsTried[quiets++] = currentMove;
            hist = getHistoryScore(thread->history, currentMove, board->turn, 128);
        }
        
        // Step 14. Futility Pruning. If our score is far below alpha,
        // and we don't expect anything from this move, skip it.
        if (   !PvNode
            &&  isQuiet
            &&  played >= 1
            &&  futilityMargin <= alpha
            &&  depth <= FutilityPruningDepth)
            continue;
            
        // Step 15. Weak Capture Pruning. Prune this capture if it is capturing
        // a weaker piece which is protected, so long as we do not have any 
        // additional support for the attacker. Don't include capture-promotions
        if (    !PvNode
            &&  !isQuiet
            &&  !inCheck
            &&   played >= 1
            &&   depth <= 5
            &&   MoveType(currentMove) != ENPASS_MOVE
            &&   MoveType(currentMove) != PROMOTION_MOVE
            &&  !ei.positionIsDrawn
            && !(ei.attackedBy2[board->turn] & (1ull << MoveTo(currentMove)))
            &&   PieceValues[PieceType(board->squares[MoveTo  (currentMove)])][MG]
             <   PieceValues[PieceType(board->squares[MoveFrom(currentMove)])][MG]){
                 
          
            // If the target piece has two or more defenders, we will prune up to depth 5
            if (ei.attackedBy2[!board->turn] & (1ull << MoveTo(currentMove)))
                continue;
            
            // Otherwise, if the piece has one defender, we will prune up to depth 3
            if (    depth <= 3
                && (ei.attacked[!board->turn] & (1ull << MoveTo(currentMove))))
                continue;
        }
        
        // Apply and validate move before searching
        applyMove(board, currentMove, undo);
        if (!isNotInCheck(board, !board->turn)){
            revertMove(board, currentMove, undo);
            continue;
        }
        
        // Step 16. Late Move Pruning / Move Count Pruning. If we have
        // tried many quiets in this position already, and we don't expect
        // anything from this move, we can undo it and move on.
        if (   !PvNode
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
    
        // Step 17. Late Move Reductions. We will search some moves at a
        // lower depth. If they look poor at a lower depth, then we will
        // move on. If they look good, we will search with a full depth.
        if (    played >= 4
            &&  depth >= 3
            &&  isQuiet){
            
            R  = 2;
            R += (played - 4) / 8;
            R += (depth  - 4) / 6;
            R += 2 * !PvNode;
            R += ttTactical && bestMove == ttMove;
            R -= hist / 24;
            R  = MIN(depth - 1, MAX(R, 1));
        }
        
        else {
            R = 1;
        }
        
        
        // Search the move with a possibly reduced depth, on a full or null window
        value =  (played == 1 || !PvNode)
               ? -search(thread, &lpv, -beta, -alpha, depth-R, height+1)
               : -search(thread, &lpv, -alpha-1, -alpha, depth-R, height+1);
               
        // If the search beat alpha, we may need to research, in the event that
        // the previous search was not the full window, or was a reduced depth
        value =  (value > alpha && (R != 1 || (played != 1 && PvNode)))
               ? -search(thread, &lpv, -beta, -alpha, depth-1, height+1)
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
            if (isQuiet && thread->killers[height][0] != currentMove){
                thread->killers[height][1] = thread->killers[height][0];
                thread->killers[height][0] = currentMove;
            }
            
            break;
        }
    }
    
    if (played == 0) return inCheck ? -MATE + height : 0;
    
    else if (best >= beta && !moveIsTactical(board, bestMove)){
        updateHistory(thread->history, bestMove, board->turn, 1, depth*depth);
        for (i = 0; i < quiets - 1; i++)
            updateHistory(thread->history, quietsTried[i], board->turn, 0, depth*depth);
    }
    
    storeTranspositionEntry(&Table, depth, (best > oldAlpha && best < beta)
                            ? PVNODE : best >= beta ? CUTNODE : ALLNODE,
                            valueToTT(best, height), bestMove, board->hash);
    return best;
}

int qsearch(Thread* thread, PVariation* pv, int alpha, int beta, int height){
    
    Board* const board = &thread->board;
    
    int eval, value, best, maxValueGain;
    uint16_t currentMove;
    Undo undo[1];
    MovePicker movePicker;
    EvalInfo ei;
    
    PVariation lpv;
    lpv.length = 0;
    pv->length = 0;
    
    // Step 1A. Check to see if search time has expired
    if (   (thread->limits->limitedBySelf || thread->limits->limitedByTime)
        && (thread->nodes & 8191) == 8191
        &&  getRealTime() >= thread->info->starttime + thread->info->maxusage)
        longjmp(thread->jbuffer, 1);
        
    // Step 1B. Check to see if the master thread finished
    if (thread->abort) longjmp(thread->jbuffer, 1);
    
    // Call this a node
    thread->nodes += 1;
    
    // Max height reached, stop here
    if (height >= MAX_HEIGHT)
        return evaluateBoard(board, &ei, &thread->ptable);
    
    // Get a standing eval of the current board
    best = value = eval = evaluateBoard(board, &ei, &thread->ptable);
    
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
    if (     value + maxValueGain < alpha
        && !(board->colours[WHITE] & board->pieces[PAWN] & RANK_7)
        && !(board->colours[BLACK] & board->pieces[PAWN] & RANK_2))
        return value;
    
    
    initializeMovePicker(&movePicker, thread, NONE_MOVE, height, 1);
    
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
        
        // Prune this capture if it is capturing a weaker piece which is protected,
        // so long as we do not have any additional support for the attacker. If
        // the capture is also a promotion we will not perform any pruning here
        if (     MoveType(currentMove) != PROMOTION_MOVE
            &&  !ei.positionIsDrawn
            &&  (ei.attacked[!board->turn]   & (1ull << MoveTo(currentMove)))
            && !(ei.attackedBy2[board->turn] & (1ull << MoveTo(currentMove)))
            &&  PieceValues[PieceType(board->squares[MoveTo  (currentMove)])][MG]
             <  PieceValues[PieceType(board->squares[MoveFrom(currentMove)])][MG])
            continue;
        
        // Apply and validate move before searching
        applyMove(board, currentMove, undo);
        if (!isNotInCheck(board, !board->turn)){
            revertMove(board, currentMove, undo);
            continue;
        }
        
        // Search next depth
        value = -qsearch(thread, &lpv, -beta, -alpha, height+1);
        
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

int moveIsTactical(Board* board, uint16_t move){
    return board->squares[MoveTo(move)] != EMPTY
        || MoveType(move) == PROMOTION_MOVE
        || MoveType(move) == ENPASS_MOVE;
}

int hasNonPawnMaterial(Board* board, int turn){
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
