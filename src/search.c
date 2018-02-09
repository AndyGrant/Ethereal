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

uint16_t getBestMove(Thread* threads, Board* board, Limits* limits, double time, double mtg, double inc){
    
    int i, nthreads = threads[0].nthreads;
    
    SearchInfo info; memset(&info, 0, sizeof(SearchInfo));
    
    pthread_t* pthreads = malloc(sizeof(pthread_t) * nthreads);
    
    
    // Some initialization for time management
    info.starttime = getRealTime();
    info.pvStability = 1;
    info.scoreStability = 1;
    
    // Ethereal is responsible for choosing how much time to spend searching
    if (limits->limitedBySelf){
        
        if (mtg >= 0){
            info.idealusage =  0.65 * time / (mtg +  5) + inc;
            info.maxalloc   =  4.00 * time / (mtg +  7) + inc;
            info.maxusage   = 10.00 * time / (mtg + 10) + inc;
        }
        
        else {
            info.idealusage =  0.45 * (time + 23 * inc) / 28;
            info.maxalloc   =  4.00 * (time + 23 * inc) / 27;
            info.maxusage   = 10.00 * (time + 23 * inc) / 25;
        }
        
        info.idealusage = MIN(info.idealusage, time - 100);
        info.maxalloc   = MIN(info.maxalloc,   time -  75);
        info.maxusage   = MIN(info.maxusage,   time -  50);
    }
    
    // UCI command told us to look for exactly X seconds
    if (limits->limitedByTime){
        info.idealusage = limits->timeLimit;
        info.maxalloc   = limits->timeLimit;
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
        // the main thread, based on the number of threads already on some depths
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
        if (limits->limitedBySelf && depth >= 4){
            
            // Increase our time if the score suddently dropped by eight centipawns
            if (info->values[depth-1] > value + 8)
                info->idealusage *= MAX(info->scoreStability, 1.05);
            
            // Decrease our time if the score suddently jumped by eight centipawns
            if (info->values[depth-1] < value - 8)
                info->idealusage *= MAX(0.99, MIN(info->pvStability, 1.00));
            
            // Increase our time if the pv has changed across the last two iterations
            if (info->bestmoves[depth-1] != thread->pv.line[0])
                info->idealusage *= MAX(info->pvStability, 1.30);
            
            // Decrease our time if the pv has stayed the same between iterations
            if (info->bestmoves[depth-1] == thread->pv.line[0])
                info->idealusage *= MAX(0.95, MIN(info->pvStability, 1.00));
            
            // Cap our ideal usage at the max allocation of time
            info->idealusage = MIN(info->idealusage, info->maxalloc);
            
            // Update the Score Stability depending on changes between the score of the current
            // iteration and the last one. Stability is a bit of a misnomer. Score Stability is
            // meant to determine when we should be concered with score drops. If we just found
            // the this iteration to be +50 from the last, we would not be surprised to find that
            // gain fall to something smaller like +30
            info->scoreStability *= 1.00 + (info->values[depth-1] - value) / 320.00;
            
            // Update the PV Stability depending on the best move changing. If the best move is
            // holding stable, we increase the pv stability. This way, if the best move changes
            // after holding for many iterations, more time will be allocated for the search, and
            // less time if the best move is in a constant flucation.
            info->pvStability *= (info->bestmoves[depth-1] != thread->pv.line[0]) ? 0.95 : 1.05;
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
            double timeFactor = info->timeUsage[depth] / MAX(1, info->timeUsage[depth-1]);
            double estimatedUsage = info->timeUsage[depth] * (timeFactor + .40);
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
    int quiets = 0, played = 0, bestWasQuiet = 0; 
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
    
    // Step 1A. Check to see if search time has expired. We will force the search
    // to continue after the search time has been used in the event that we have
    // not yet completed our depth one search, and therefore would have no best move
    if (   (thread->limits->limitedBySelf || thread->limits->limitedByTime)
        && (thread->nodes & 4095) == 4095
        &&  getRealTime() >= thread->info->starttime + thread->info->maxusage
        &&  thread->depth > 1)
        longjmp(thread->jbuffer, 1);
        
    // Step 1B. Check to see if the master thread finished
    if (thread->abort) longjmp(thread->jbuffer, 1);
        
    // If we allow early exits in the root node (even if they should not happen)
    // we run the risk of returning an empty principle variation, which could then
    // be used as the "bestmove" for the current iteration, and pheraps the whole
    // search. Therefore, we will avoid this by not allowing this type of early
    // exit in root nodes. We make the exception that if the return value in the
    // the event of a draw would fall outside the alpha-beta window, we will allow
    // the mate distance pruning and draw detection pruning to occur here
    if (!RootNode || 0 >= beta || 0 <= alpha){
        
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
        
        // Entry move may be good in this position
        ttMove = ttEntry.bestMove;
        
        // Step 6A. Check to see if this entry allows us to exit this
        // node early. We choose not to do this in the PV line, not because
        // we can't, but because don't want truncated PV lines
        if (!PvNode && ttEntry.depth >= depth){

            rAlpha = alpha; rBeta = beta;
            ttValue = valueFromTT(ttEntry.value, height);
            
            switch (ttEntry.type){
                case  PVNODE: return ttValue;
                case CUTNODE: rAlpha = MAX(ttValue, alpha); break;
                case ALLNODE:  rBeta = MIN(ttValue,  beta); break;
            }
            
            // Entry allows early exit
            if (rAlpha >= rBeta) return ttValue;
        }
    }
    
    // Step 7. Some initialization. Determine the check status if we have
    // not already done so (happens when depth was <= 0, and we are in check,
    // thus avoiding the quiescence search). Also, in non PvNodes, we will
    // perform pruning based on the board eval, so we will need that, as well
    // as a futilityMargin calculated based on the eval and current depth
    inCheck = inCheck || !isNotInCheck(board, board->turn);
    if (!PvNode){
        eval = evaluateBoard(board, &ei, &thread->ptable);
        futilityMargin = eval + 70 * depth;
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
        if (value <= rAlpha) return alpha;
    }
    
    // Step 9. Beta Pruning / Reverse Futility Pruning / Static Null
    // Move Pruning. If the eval is few pawns above beta then exit early
    if (   !PvNode
        && !inCheck
        &&  depth <= BetaPruningDepth
        &&  eval - 70 * depth > beta)
        return beta;

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
        
        if (value >= beta) return beta;
    }
    
    // Step 11. ProbCut. If we have a good capture that causes a beta cutoff
    // with a slightly reduced depth search it is likely that this capture is
    // likely going to be good at a full depth. To save some work we will prune
    // captures that won't exceed rbeta or captures that fail at a low depth
    if (   !PvNode
        && !inCheck
        &&  depth >= ProbCutDepth
        &&  eval + bestTacticalMoveValue(board) >= beta + ProbCutMargin){
            
        rBeta = MIN(beta + ProbCutMargin, MATE - MAX_HEIGHT - 1);
            
        initializeMovePicker(&movePicker, thread, NONE_MOVE, height, 1);
        
        while ((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
            
            // Even if we keep the capture piece and or the promotion piece
            // we will fail to exceed rBeta, then we will skip this move
            if (eval + thisTacticalMoveValue(board, currentMove) < rBeta)
                continue;
            
            // Apply and validate move before searching
            applyMove(board, currentMove, undo);
            if (!isNotInCheck(board, !board->turn)){
                revertMove(board, currentMove, undo);
                continue;
            }
            
            // Verify the move is good with a depth zero search (qsearch, unless in check)
            // and then with a slightly reduced search. If both searches still exceed rBeta,
            // we will prune this node's subtree with resonable assurance that we made no error
            if (   -search(thread, &lpv, -rBeta, -rBeta+1,       0, height+1) >= rBeta
                && -search(thread, &lpv, -rBeta, -rBeta+1, depth-4, height+1) >= rBeta){
                    
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
        
        // Probe for the newly found move, and update ttMove
        if (getTranspositionEntry(&Table, board->hash, &ttEntry))
            ttMove = ttEntry.bestMove;
    }
    
    // Step 13. Check Extension at non Root nodes that are PV or low depth
    depth += inCheck && !RootNode && (PvNode || depth <= 6);
    
    
    initializeMovePicker(&movePicker, thread, ttMove, height, 0);
    
    while ((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
        
        // If this move is quiet we will save it to a list of attemped
        // quiets, and we will need a history score for pruning decisions
        if ((isQuiet = !moveIsTactical(board, currentMove))){
            quietsTried[quiets++] = currentMove;
            hist = getHistoryScore(thread->history, currentMove, board->turn);
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
            
            // Baseline R based on number of moves played and current depth
            R = 2 + (played - 4) / 8 + (depth - 4) / 6;
            
            // Increase R by an additional two ply for non PvNodes
            R += 2 * !PvNode;
            
            // Decrease R by an additional ply if we have a quiet move as our best
            // move, or we are looking at an early quiet move in a situation where
            // we either have no table move, or the table move is not the best so far
            R -= bestWasQuiet || (ttMove != bestMove && quiets <= 2);
            
            // Adjust R based on history score. We will not allow history to increase
            // R by more than 1. History scores are within [-16384, 16384], so we can
            // expect an adjustment on the bounds of [+1, -6], with 6 being very rare
            R -= MAX(-1, ((hist + 8192) / 4096) - (hist <= -8192));
            
            // Do not allow the reduction to take us directly into a quiescence search
            // and also ensure that R is at least one, therefore avoiding extensions
            R  = MIN(depth - 1, MAX(R, 1));
            
        } else R = 1;
        
        
        // Step 18A. Search the move with a possibly reduced depth basedon LMR,
        // and a null window unless this is the first move within a PvNode
        value =  (played == 1 || !PvNode)
               ? -search(thread, &lpv,    -beta, -alpha, depth-R, height+1)
               : -search(thread, &lpv, -alpha-1, -alpha, depth-R, height+1);
               
        // Step 18B. Research the move if it improved alpha, and was either a reduced
        // search or a search on a null window. Otherwise, keep the current value
        value =  (value > alpha && (R != 1 || (played != 1 && PvNode)))
               ? -search(thread, &lpv, -beta, -alpha, depth-1, height+1)
               :  value;
        
        // Revert the board state
        revertMove(board, currentMove, undo);
        
        
        // Step 19. Update search stats for the best move and its value. Update
        // our lower bound (alpha) if exceeded, and also update the PV in that case
        if (value > best){
            
            best = value;
            bestMove = currentMove;
            bestWasQuiet = isQuiet;
            
            if (value > alpha){
                alpha = value;
                
                // Copy our child's PV and prepend this move to it
                pv->length = 1 + lpv.length;
                pv->line[0] = currentMove;
                memcpy(pv->line + 1, lpv.line, sizeof(uint16_t) * lpv.length);
            }
        }
        
        // Step 20. Search has failed high. Update Killer Moves and exit search
        if (alpha >= beta){
            
            if (isQuiet && thread->killers[height][0] != currentMove){
                thread->killers[height][1] = thread->killers[height][0];
                thread->killers[height][0] = currentMove;
            }
            
            break;
        }
    }
    
    // Step 21. Stalemate and Checkmate detection. If no moves were found to
    // be legal (search makes sure to play at least one legal move, if any),
    // then we are either mated or stalemated, which we can tell by the inCheck
    // flag. For mates, return a score based on the distance from root, so we
    // can differentiate between close mates and far away mates from the root
    if (played == 0) return inCheck ? -MATE + height : 0;
    
    // Step 22. Update History counters on a fail high for a quiet move
    if (best >= beta && !moveIsTactical(board, bestMove)){
        updateHistory(thread->history, bestMove, board->turn, depth*depth);
        for (i = 0; i < quiets - 1; i++)
            updateHistory(thread->history, quietsTried[i], board->turn, -depth*depth);
    }
    
    // Step 23. Store the results of the search in the transposition table.
    // We must determine a bound for the result based on alpha and beta, and
    // must also convert the search value to a tt value, which handles mates
    storeTranspositionEntry(&Table, depth, (best > oldAlpha && best < beta)
                            ? PVNODE : best >= beta ? CUTNODE : ALLNODE,
                            valueToTT(best, height), bestMove, board->hash);
    return best;
}

int qsearch(Thread* thread, PVariation* pv, int alpha, int beta, int height){
    
    Board* const board = &thread->board;
    
    int eval, value, best;
    uint16_t currentMove;
    Undo undo[1];
    MovePicker movePicker;
    EvalInfo ei;
    
    PVariation lpv;
    lpv.length = 0;
    pv->length = 0;
    
    // Increment the node counter even if we exit early
    thread->nodes += 1;
    
    // Step 1A. Check to see if search time has expired. We will force the search
    // to continue after the search time has been used in the event that we have
    // not yet completed our depth one search, and therefore would have no best move
    if (   (thread->limits->limitedBySelf || thread->limits->limitedByTime)
        && (thread->nodes & 4095) == 4095
        &&  getRealTime() >= thread->info->starttime + thread->info->maxusage
        &&  thread->depth > 1)
        longjmp(thread->jbuffer, 1);
        
    // Step 1B. Check to see if the master thread finished
    if (thread->abort) longjmp(thread->jbuffer, 1);
    
    // Step 2. Max Height Cutoff. If we are at the maximum search height,
    // then end the search here with a static eval of the current board
    if (height >= MAX_HEIGHT)
        return evaluateBoard(board, &ei, &thread->ptable);
    
    // Step 3. Eval Pruning. If a static evaluation of the board will
    // exceed beta, then we can stop the search here. Also, if the static
    // eval exceeds alpha, we can call our static eval the new alpha
    best = value = eval = evaluateBoard(board, &ei, &thread->ptable);
    alpha = MAX(alpha, value);
    if (alpha >= beta) return value;
    
    // Step 4. Delta Pruning. Even the best possible capture and or promotion
    // combo with the additional of the futility margin would still fall below alpha
    if (value + QFutilityMargin + bestTacticalMoveValue(board) < alpha)
        return eval;
    
    // Step 5. Move Generation and Looping. Generate all tactical moves for this
    // position (includes Captures, Promotions, and Enpass) and try them
    initializeMovePicker(&movePicker, thread, NONE_MOVE, height, 1);
    while ((currentMove = selectNextMove(&movePicker, board)) != NONE_MOVE){
        
        // Step 6. Futility Pruning. Similar to Delta Pruning, if this capture in the
        // best case would still fail to beat alpha minus some margin, we can skip it
        if (eval + QFutilityMargin + thisTacticalMoveValue(board, currentMove) < alpha)
            continue;
        
        // Step 7. Weak Capture Pruning. If we are trying to capture a piece which
        // is protected, and we are the sole attacker, then we can be somewhat safe
        // in skipping this move so long as we are capturing a weaker piece
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

int thisTacticalMoveValue(Board* board, uint16_t move){
    
    int value = PieceValues[PieceType(board->squares[MoveTo(move)])][EG];
    
    if (MoveType(move) == PROMOTION_MOVE)
        value += PieceValues[1 + (move >> 14)][EG] - PieceValues[PAWN][EG];
    
    if (MoveType(move) == ENPASS_MOVE)
        value += PieceValues[PAWN][EG];
    
    return value;
}

int bestTacticalMoveValue(Board* board){
    
    int value = 0;
    
    uint64_t targets = board->colours[!board->turn];
    
    if (targets & board->pieces[QUEEN]) value += PieceValues[QUEEN][EG];
    
    else if (targets & board->pieces[ROOK]) value += PieceValues[ROOK][EG];
    
    else if (targets & (board->pieces[KNIGHT] | board->pieces[BISHOP]))
        value += MAX(
            !!(targets & board->pieces[KNIGHT]) * PieceValues[KNIGHT][EG],
            !!(targets & board->pieces[BISHOP]) * PieceValues[BISHOP][EG]
        );
        
    else 
        value += PieceValues[PAWN][EG];
        
        
    if (   board->pieces[PAWN] 
        &  board->colours[board->turn]
        & (board->turn == WHITE ? RANK_7 : RANK_2))
        value += PieceValues[QUEEN][EG] - PieceValues[PAWN][EG];
            
    return value;
} 