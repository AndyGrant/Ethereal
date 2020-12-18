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
#include <math.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "attacks.h"
#include "bitboards.h"
#include "board.h"
#include "evaluate.h"
#include "pyrrhic/tbprobe.h"
#include "history.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "search.h"
#include "syzygy.h"
#include "thread.h"
#include "time.h"
#include "transposition.h"
#include "types.h"
#include "uci.h"
#include "windows.h"

int LMRTable[64][64];      // Late Move Reductions
volatile int ABORT_SIGNAL; // Global ABORT flag for threads
volatile int IS_PONDERING; // Global PONDER flag for threads
volatile int ANALYSISMODE; // Whether to make some changes for Analysis

void initSearch() {

    // Init Late Move Reductions Table
    for (int depth = 1; depth < 64; depth++)
        for (int played = 1; played < 64; played++)
            LMRTable[depth][played] = 0.75 + log(depth) * log(played) / 2.25;
}

void getBestMove(Thread *threads, Board *board, Limits *limits, uint16_t *best, uint16_t *ponder) {

    SearchInfo info = {0};
    pthread_t pthreads[threads->nthreads];

    // Allow Syzygy to refine the move list for optimal results
    if (!limits->limitedByMoves && limits->multiPV == 1)
        if (tablebasesProbeDTZ(board, limits, best, ponder))
            return;

    // Minor house keeping for starting a search
    updateTT(); // Table has an age component
    ABORT_SIGNAL = 0; // Otherwise Threads will exit
    initTimeManagment(&info, limits);
    newSearchThreadPool(threads, board, limits, &info);

    // Create a new thread for each of the helpers and reuse the current
    // thread for the main thread, which avoids some overhead and saves
    // us from having the current thread eating CPU time while waiting
    for (int i = 1; i < threads->nthreads; i++)
        pthread_create(&pthreads[i], NULL, &iterativeDeepening, &threads[i]);
    iterativeDeepening((void*) &threads[0]);

    // When the main thread exits it should signal for the helpers to
    // shutdown. Wait until all helpers have finished before moving on
    ABORT_SIGNAL = 1;
    for (int i = 1; i < threads->nthreads; i++)
        pthread_join(pthreads[i], NULL);

    // The main thread will update SearchInfo with results
    *best = info.bestMoves[info.depth];
    *ponder = info.ponderMoves[info.depth];
}

void* iterativeDeepening(void *vthread) {

    Thread *const thread   = (Thread*) vthread;
    SearchInfo *const info = thread->info;
    Limits *const limits   = thread->limits;
    const int mainThread   = thread->index == 0;

    // Bind when we expect to deal with NUMA
    if (thread->nthreads > 8)
        bindThisThread(thread->index);

    // Perform iterative deepening until exit conditions
    for (thread->depth = 1; thread->depth < MAX_PLY; thread->depth++) {

        // If we abort to here, we stop searching
        if (setjmp(thread->jbuffer)) break;

        // Perform a search for the current depth for each requested line of play
        for (thread->multiPV = 0; thread->multiPV < limits->multiPV; thread->multiPV++)
            aspirationWindow(thread);

        // Helper threads need not worry about time and search info updates
        if (!mainThread) continue;

        // Update SearchInfo and report some results
        info->depth                    = thread->depth;
        info->values[info->depth]      = thread->values[0];
        info->bestMoves[info->depth]   = thread->bestMoves[0];
        info->ponderMoves[info->depth] = thread->ponderMoves[0];

        // Update time allocation based on score and pv changes
        updateTimeManagment(info, limits);

        // Don't want to exit while pondering
        if (IS_PONDERING) continue;

        // Check for termination by any of the possible limits
        if (   (limits->limitedBySelf  && terminateTimeManagment(info))
            || (limits->limitedBySelf  && elapsedTime(info) > info->maxUsage)
            || (limits->limitedByTime  && elapsedTime(info) > limits->timeLimit)
            || (limits->limitedByDepth && thread->depth >= limits->depthLimit))
            break;
    }

    return NULL;
}

void aspirationWindow(Thread *thread) {

    PVariation *const pv = &thread->pv;
    const int multiPV    = thread->multiPV;
    const int mainThread = thread->index == 0;

    int value, depth = thread->depth;
    int alpha = -MATE, beta = MATE, delta = WindowSize;

    // After a few depths use a previous result to form a window
    if (thread->depth >= WindowDepth) {
        alpha = MAX(-MATE, thread->values[0] - delta);
        beta  = MIN( MATE, thread->values[0] + delta);
    }

    while (1) {

        // Perform a search and consider reporting results
        value = search(thread, pv, alpha, beta, MAX(1, depth));
        if (   (mainThread && value > alpha && value < beta)
            || (mainThread && elapsedTime(thread->info) >= WindowTimerMS))
            uciReport(thread->threads, alpha, beta, value);

        // Search returned a result within our window
        if (value > alpha && value < beta) {
            thread->values[multiPV]      = value;
            thread->bestMoves[multiPV]   = pv->line[0];
            thread->ponderMoves[multiPV] = pv->length > 1 ? pv->line[1] : NONE_MOVE;
            return;
        }

        // Search failed low, adjust window and reset depth
        if (value <= alpha) {
            beta  = (alpha + beta) / 2;
            alpha = MAX(-MATE, alpha - delta);
            depth = thread->depth;
        }

        // Search failed high, adjust window and reduce depth
        else if (value >= beta) {
            beta = MIN(MATE, beta + delta);
            depth = depth - (abs(value) <= MATE / 2);
        }

        // Expand the search window
        delta = delta + delta / 2;
    }
}

int search(Thread *thread, PVariation *pv, int alpha, int beta, int depth) {

    const int PvNode   = (alpha != beta - 1);
    const int RootNode = (thread->height == 0);
    Board *const board = &thread->board;

    unsigned tbresult;
    int hist = 0, cmhist = 0, fmhist = 0;
    int movesSeen = 0, quietsPlayed = 0, capturesPlayed = 0, played = 0;
    int ttHit, ttValue = 0, ttEval = VALUE_NONE, ttDepth = 0, ttBound = 0;
    int R, newDepth, rAlpha, rBeta, oldAlpha = alpha;
    int inCheck, isQuiet, improving, extension, singular, skipQuiets = 0;
    int eval, value = -MATE, best = -MATE, futilityMargin, seeMargin[2];
    uint16_t move, ttMove = NONE_MOVE, bestMove = NONE_MOVE;
    uint16_t quietsTried[MAX_MOVES], capturesTried[MAX_MOVES];
    MovePicker movePicker;
    PVariation lpv;

    // Step 1. Quiescence Search. Perform a search using mostly tactical
    // moves to reach a more stable position for use as a static evaluation
    if (depth <= 0 && !board->kingAttackers)
        return qsearch(thread, pv, alpha, beta);

    // Prefetch TT as early as reasonable
    prefetchTTEntry(board->hash);

    // Ensure a fresh PV
    pv->length = 0;

    // Ensure positive depth
    depth = MAX(0, depth);

    // Updates for UCI reporting
    thread->seldepth = RootNode ? 0 : MAX(thread->seldepth, thread->height);
    thread->nodes++;

    // Step 2. Abort Check. Exit the search if signaled by main thread or the
    // UCI thread, or if the search time has expired outside pondering mode
    if (ABORT_SIGNAL || (terminateSearchEarly(thread) && !IS_PONDERING))
        longjmp(thread->jbuffer, 1);

    // Step 3. Check for early exit conditions. Don't take early exits in
    // the RootNode, since this would prevent us from having a best move
    if (!RootNode) {

        // Draw Detection. Check for the fifty move rule, repetition, or insufficient
        // material. Add variance to the draw score, to avoid blindness to 3-fold lines
        if (boardIsDrawn(board, thread->height)) return 1 - (thread->nodes & 2);

        // Check to see if we have exceeded the maxiumum search draft
        if (thread->height >= MAX_PLY)
            return evaluateBoard(thread, board);

        // Mate Distance Pruning. Check to see if this line is so
        // good, or so bad, that being mated in the ply, or  mating in
        // the next one, would still not create a more extreme line
        rAlpha = alpha > -MATE + thread->height     ? alpha : -MATE + thread->height;
        rBeta  =  beta <  MATE - thread->height - 1 ?  beta :  MATE - thread->height - 1;
        if (rAlpha >= rBeta) return rAlpha;
    }

    // Step 4. Probe the Transposition Table, adjust the value, and consider cutoffs
    if ((ttHit = getTTEntry(board->hash, &ttMove, &ttValue, &ttEval, &ttDepth, &ttBound))) {

        ttValue = valueFromTT(ttValue, thread->height); // Adjust any MATE scores

        // Only cut with a greater depth search, and do not return
        // when in a PvNode, unless we would otherwise hit a qsearch
        if (ttDepth >= depth && (depth == 0 || !PvNode)) {

            // Table is exact or produces a cutoff
            if (    ttBound == BOUND_EXACT
                || (ttBound == BOUND_LOWER && ttValue >= beta)
                || (ttBound == BOUND_UPPER && ttValue <= alpha))
                return ttValue;
        }
    }

    // Step 5. Probe the Syzygy Tablebases. tablebasesProbeWDL() handles all of
    // the conditions about the board, the existance of tables, the probe depth,
    // as well as to not probe at the Root. The return is defined by the Pyrrhic API
    if ((tbresult = tablebasesProbeWDL(board, depth, thread->height)) != TB_RESULT_FAILED) {

        thread->tbhits++; // Increment tbhits counter for this thread

        // Convert the WDL value to a score. We consider blessed losses
        // and cursed wins to be a draw, and thus set value to zero.
        value = tbresult == TB_LOSS ? -TBWIN + thread->height
              : tbresult == TB_WIN  ?  TBWIN - thread->height : 0;

        // Identify the bound based on WDL scores. For wins and losses the
        // bound is not exact because we are dependent on the height, but
        // for draws (and blessed / cursed) we know the tbresult to be exact
        ttBound = tbresult == TB_LOSS ? BOUND_UPPER
                : tbresult == TB_WIN  ? BOUND_LOWER : BOUND_EXACT;

        // Check to see if the WDL value would cause a cutoff
        if (    ttBound == BOUND_EXACT
            || (ttBound == BOUND_LOWER && value >= beta)
            || (ttBound == BOUND_UPPER && value <= alpha)) {

            storeTTEntry(board->hash, NONE_MOVE, valueToTT(value, thread->height), VALUE_NONE, depth, ttBound);
            return value;
        }
    }

    // Step 6. Initialize flags and values used by pruning and search methods

    // We can grab in check based on the already computed king attackers bitboard
    inCheck = !!board->kingAttackers;

    // Save a history of the static evaluations
    eval = thread->evalStack[thread->height]
         = ttEval != VALUE_NONE ? ttEval : evaluateBoard(thread, board);

    // Futility Pruning Margin
    futilityMargin = FutilityMargin * depth;

    // Static Exchange Evaluation Pruning Margins
    seeMargin[0] = SEENoisyMargin * depth * depth;
    seeMargin[1] = SEEQuietMargin * depth;

    // Improving if our static eval increased in the last move
    improving = thread->height >= 2 && eval > thread->evalStack[thread->height-2];

    // Reset Killer moves for our children
    thread->killers[thread->height+1][0] = NONE_MOVE;
    thread->killers[thread->height+1][1] = NONE_MOVE;

    // ------------------------------------------------------------------------
    // All elo estimates as of Ethereal 11.80, @ 12s+0.12 @ 1.275mnps
    // ------------------------------------------------------------------------

    // Step 7 (~32 elo). Beta Pruning / Reverse Futility Pruning / Static
    // Null Move Pruning. If the eval is well above beta, defined by a depth
    // dependent margin, then we assume the eval will hold above beta
    if (   !PvNode
        && !inCheck
        &&  depth <= BetaPruningDepth
        &&  eval - BetaMargin * depth > beta)
        return eval;

    // Step 8 (~3 elo). Alpha Pruning for main search loop. The idea is
    // that for low depths if eval is so bad that even a large static
    // bonus doesn't get us beyond alpha, then eval will hold below alpha
    if (   !PvNode
        && !inCheck
        &&  depth <= AlphaPruningDepth
        &&  eval + AlphaMargin <= alpha)
        return eval;

    // Step 9 (~93 elo). Null Move Pruning. If our position is so good that giving
    // our opponent back-to-back moves is still not enough for them to
    // gain control of the game, we can be somewhat safe in saying that
    // our position is too good to be true. We avoid NMP when we have
    // information from the Transposition Table which suggests it will fail
    if (   !PvNode
        && !inCheck
        &&  eval >= beta
        &&  depth >= NullMovePruningDepth
        &&  thread->moveStack[thread->height-1] != NULL_MOVE
        &&  thread->moveStack[thread->height-2] != NULL_MOVE
        &&  boardHasNonPawnMaterial(board, board->turn)
        && (!ttHit || !(ttBound & BOUND_UPPER) || ttValue >= beta)) {

        R = 4 + depth / 6 + MIN(3, (eval - beta) / 200);

        apply(thread, board, NULL_MOVE);
        value = -search(thread, &lpv, -beta, -beta+1, depth-R);
        revert(thread, board, NULL_MOVE);

        if (value >= beta) return beta;
    }

    // Step 10 (~9 elo). Probcut Pruning. If we have a good capture that causes a cutoff
    // with an adjusted beta value at a reduced search depth, we expect that it will
    // cause a similar cutoff at this search depth, with a normal beta value
    if (   !PvNode
        &&  depth >= ProbCutDepth
        &&  abs(beta) < MATE_IN_MAX
        && (eval >= beta || eval + moveBestCaseValue(board) >= beta + ProbCutMargin)) {

        // Try tactical moves which maintain rBeta
        rBeta = MIN(beta + ProbCutMargin, MATE - MAX_PLY - 1);
        initNoisyMovePicker(&movePicker, thread, rBeta - eval);
        while ((move = selectNextMove(&movePicker, board, 1)) != NONE_MOVE) {

            // Apply move, skip if move is illegal
            if (!apply(thread, board, move)) continue;

            // For high depths, verify the move first with a depth one search
            if (depth >= 2 * ProbCutDepth)
                value = -qsearch(thread, &lpv, -rBeta, -rBeta+1);

            // For low depths, or after the above, verify with a reduced search
            if (depth < 2 * ProbCutDepth || value >= rBeta)
                value = -search(thread, &lpv, -rBeta, -rBeta+1, depth-4);

            // Revert the board state
            revert(thread, board, move);

            // Probcut failed high verifying the cutoff
            if (value >= rBeta) return value;
        }
    }

    // Step 11. Initialize the Move Picker and being searching through each
    // move one at a time, until we run out or a move generates a cutoff
    initMovePicker(&movePicker, thread, ttMove);
    while ((move = selectNextMove(&movePicker, board, skipQuiets)) != NONE_MOVE) {

        // MultiPV and searchmoves may limit our search options
        if (RootNode && moveExaminedByMultiPV(thread, move)) continue;
        if (RootNode &&    !moveIsInRootMoves(thread, move)) continue;

        // Track Moves Seen for Late Move Pruning
        movesSeen += 1;
        isQuiet = !moveIsTactical(board, move);

        // All moves have one or more History scores
        hist = !isQuiet ? getCaptureHistory(thread, move)
             : getHistory(thread, move, &cmhist, &fmhist);

        // Step 12 (~80 elo). Late Move Pruning / Move Count Pruning. If we
        // have seen many moves in this position already, and we don't expect
        // anything from this move, we can skip all the remaining quiets
        if (   best > -MATE_IN_MAX
            && depth <= LateMovePruningDepth
            && movesSeen >= LateMovePruningCounts[improving][depth])
            skipQuiets = 1;

        // Step 13 (~175 elo). Quiet Move Pruning. Prune any quiet move that meets one
        // of the criteria below, only after proving a non mated line exists
        if (isQuiet && best > -MATE_IN_MAX) {

            // Base LMR value that we expect to use later
            R = LMRTable[MIN(depth, 63)][MIN(played, 63)];

            // Step 13A (~3 elo). Futility Pruning. If our score is far below alpha,
            // and we don't expect anything from this move, we can skip all other quiets
            if (   depth <= FutilityPruningDepth
                && eval + futilityMargin <= alpha
                && hist < FutilityPruningHistoryLimit[improving])
                skipQuiets = 1;

            // Step 13B (~2.5 elo). Futility Pruning. If our score is not only far
            // below alpha but still far below alpha after adding the FutilityMargin,
            // we can somewhat safely skip all quiet moves after this one
            if (   depth <= FutilityPruningDepth
                && eval + futilityMargin + FutilityMarginNoHistory <= alpha)
                skipQuiets = 1;

            // Step 13C (~8 elo). Counter Move Pruning. Moves with poor counter
            // move history are pruned at near leaf nodes of the search.
            if (   movePicker.stage > STAGE_COUNTER_MOVE
                && cmhist < CounterMoveHistoryLimit[improving]
                && depth - R <= CounterMovePruningDepth[improving])
                continue;

            // Step 13D (~1.5 elo). Follow Up Move Pruning. Moves with poor
            // follow up move history are pruned at near leaf nodes of the search.
            if (   movePicker.stage > STAGE_COUNTER_MOVE
                && fmhist < FollowUpMoveHistoryLimit[improving]
                && depth - R <= FollowUpMovePruningDepth[improving])
                continue;
        }

        // Step 14 (~42 elo). Static Exchange Evaluation Pruning. Prune moves which fail
        // to beat a depth dependent SEE threshold. The use of movePicker.stage
        // is a speedup, which assumes that good noisy moves have a positive SEE
        if (    best > -MATE_IN_MAX
            &&  depth <= SEEPruningDepth
            &&  movePicker.stage > STAGE_GOOD_NOISY
            && !staticExchangeEvaluation(board, move, seeMargin[isQuiet]))
            continue;

        // Apply move, skip if move is illegal
        if (!apply(thread, board, move))
            continue;

        played += 1;
        if (isQuiet) quietsTried[quietsPlayed++] = move;
        else capturesTried[capturesPlayed++] = move;

        // The UCI spec allows us to output information about the current move
        // that we are going to search. We only do this from the main thread,
        // and we wait a few seconds in order to avoid floiding the output
        if (RootNode && !thread->index && elapsedTime(thread->info) > CurrmoveTimerMS)
            uciReportCurrentMove(board, move, played + thread->multiPV, thread->depth);

        // Identify moves which are candidate singular moves
        singular =  !RootNode
                 &&  depth >= 8
                 &&  move == ttMove
                 &&  ttDepth >= depth - 2
                 && (ttBound & BOUND_LOWER);

        // Step 15 (~60 elo). Extensions. Search an additional ply when the move comes from the
        // Transposition Table and appears to beat all other moves by a fair margin. Otherwise,
        // extend for any position where our King is checked. We also selectivly extend moves
        // with very strong continuation histories, so long as they are along the PV line

        extension = singular ? singularity(thread, &movePicker, ttValue, depth, beta)
                  : inCheck || (isQuiet && PvNode && cmhist > HistexLimit && fmhist > HistexLimit);

        newDepth = depth + (extension && !RootNode);

        // Step 16. MultiCut. Sometimes candidate Singular moves are shown to be non-Singular.
        // If this happens, and the rBeta used is greater than beta, then we have multiple moves
        // which appear to beat beta at a reduced depth. singularity() sets the stage to STAGE_DONE

        if (movePicker.stage == STAGE_DONE) {
            revert(thread, board, move);
            return MAX(ttValue - depth, -MATE);
        }

        // Step 17A (~249 elo). Quiet Late Move Reductions. Reduce the search depth
        // of Quiet moves after we've explored the main line. If a reduced search
        // manages to beat alpha, against our expectations, we perform a research
        if (isQuiet && depth > 2 && played > 1) {

            /// Use the LMR Formula as a starting point
            R  = LMRTable[MIN(depth, 63)][MIN(played, 63)];

            // Increase for non PV, non improving
            R += !PvNode + !improving;

            // Increase for King moves that evade checks
            R += inCheck && pieceType(board->squares[MoveTo(move)]) == KING;

            // Reduce for Killers and Counters
            R -= movePicker.stage < STAGE_QUIET;

            // Adjust based on history scores
            R -= MAX(-2, MIN(2, hist / 5000));

            // Don't extend or drop into QS
            R = MIN(depth - 1, MAX(R, 1));
        }

        // Step 17B (~3 elo). Noisy Late Move Reductions. The same as Step 15A, but
        // only applied to Tactical moves with unusually poor Capture History scores
        else if (!isQuiet && depth > 2 && played > 1) {

            // Initialize R based on Capture History
            R = MIN(3, 3 - (hist + 4000) / 2000);

            // Reduce for moves that give check
            R -= !!board->kingAttackers;

            // Don't extend or drop into QS
            R = MIN(depth - 1, MAX(R, 1));
        }

        // No LMR conditions were met. Use a Standard Reduction
        else R = 1;

        // Step 18A. If we triggered the LMR conditions (which we know by the value of R),
        // then we will perform a reduced search on the null alpha window, as we have no
        // expectation that this move will be worth looking into deeper
        if (R != 1) value = -search(thread, &lpv, -alpha-1, -alpha, newDepth-R);

        // Step 18B. There are two situations in which we will search again on a null window,
        // but without a depth reduction R. First, if the LMR search happened, and failed
        // high, secondly, if we did not try an LMR search, and this is not the first move
        // we have tried in a PvNode, we will research with the normally reduced depth
        if ((R != 1 && value > alpha) || (R == 1 && !(PvNode && played == 1)))
            value = -search(thread, &lpv, -alpha-1, -alpha, newDepth-1);

        // Step 18C. Finally, if we are in a PvNode and a move beat alpha while being
        // search on a reduced depth, we will search again on the normal window. Also,
        // if we did not perform Step 18B, we will search for the first time on the
        // normal window. This happens only for the first move in a PvNode
        if (PvNode && (played == 1 || value > alpha))
            value = -search(thread, &lpv, -beta, -alpha, newDepth-1);

        // Revert the board state
        revert(thread, board, move);

        // Step 19. Update search stats for the best move and its value. Update
        // our lower bound (alpha) if exceeded, and also update the PV in that case
        if (value > best) {

            best = value;
            bestMove = move;

            if (value > alpha) {
                alpha = value;

                // Copy our child's PV and prepend this move to it
                pv->length = 1 + lpv.length;
                pv->line[0] = move;
                memcpy(pv->line + 1, lpv.line, sizeof(uint16_t) * lpv.length);

                // Search failed high
                if (alpha >= beta) break;
            }
        }
    }

    // Prefetch TT for store
    prefetchTTEntry(board->hash);

    // Step 20. Stalemate and Checkmate detection. If no moves were found to
    // be legal (search makes sure to play at least one legal move, if any),
    // then we are either mated or stalemated, which we can tell by the inCheck
    // flag. For mates, return a score based on the distance from root, so we
    // can differentiate between close mates and far away mates from the root
    if (played == 0) return inCheck ? -MATE + thread->height : 0;

    // Step 21 (~760 elo). Update History counters on a fail high for a quiet move.
    // We also update Capture History Heuristics, which augment or replace MVV-LVA.

    if (best >= beta && !moveIsTactical(board, bestMove))
        updateHistoryHeuristics(thread, quietsTried, quietsPlayed, depth);

    if (best >= beta)
        updateCaptureHistories(thread, bestMove, capturesTried, capturesPlayed, depth);

    // Step 22. Store results of search into the Transposition Table. We do
    // not overwrite the Root entry from the first line of play we examined
    if (!RootNode || !thread->multiPV) {
        ttBound = best >= beta    ? BOUND_LOWER
                : best > oldAlpha ? BOUND_EXACT : BOUND_UPPER;
        storeTTEntry(board->hash, bestMove, valueToTT(best, thread->height), eval, depth, ttBound);
    }

    return best;
}

int qsearch(Thread *thread, PVariation *pv, int alpha, int beta) {

    Board *const board = &thread->board;

    int eval, value, best;
    int ttHit, ttValue = 0, ttEval = VALUE_NONE, ttDepth = 0, ttBound = 0;
    uint16_t move, ttMove = NONE_MOVE;
    MovePicker movePicker;
    PVariation lpv;

    // Prefetch TT as early as reasonable
    prefetchTTEntry(board->hash);

    // Ensure a fresh PV
    pv->length = 0;

    // Updates for UCI reporting
    thread->seldepth = MAX(thread->seldepth, thread->height);
    thread->nodes++;

    // Step 1. Abort Check. Exit the search if signaled by main thread or the
    // UCI thread, or if the search time has expired outside pondering mode
    if (ABORT_SIGNAL || (terminateSearchEarly(thread) && !IS_PONDERING))
        longjmp(thread->jbuffer, 1);

    // Step 2. Draw Detection. Check for the fifty move rule, repetition, or insufficient
    // material. Add variance to the draw score, to avoid blindness to 3-fold lines
    if (boardIsDrawn(board, thread->height)) return 1 - (thread->nodes & 2);

    // Step 3. Max Draft Cutoff. If we are at the maximum search draft,
    // then end the search here with a static eval of the current board
    if (thread->height >= MAX_PLY)
        return evaluateBoard(thread, board);

    // Step 4. Probe the Transposition Table, adjust the value, and consider cutoffs
    if ((ttHit = getTTEntry(board->hash, &ttMove, &ttValue, &ttEval, &ttDepth, &ttBound))) {

        ttValue = valueFromTT(ttValue, thread->height); // Adjust any MATE scores

        // Table is exact or produces a cutoff
        if (    ttBound == BOUND_EXACT
            || (ttBound == BOUND_LOWER && ttValue >= beta)
            || (ttBound == BOUND_UPPER && ttValue <= alpha))
            return ttValue;
    }

    // Save a history of the static evaluations
    eval = thread->evalStack[thread->height]
         = ttEval != VALUE_NONE ? ttEval : evaluateBoard(thread, board);

    // Step 5. Eval Pruning. If a static evaluation of the board will
    // exceed beta, then we can stop the search here. Also, if the static
    // eval exceeds alpha, we can call our static eval the new alpha
    best = eval;
    alpha = MAX(alpha, eval);
    if (alpha >= beta) return eval;

    // Step 6. Delta Pruning. Even the best possible capture and or promotion
    // combo, with a minor boost for pawn captures, would still fail to cover
    // the distance between alpha and the evaluation. Playing a move is futile.
    if (MAX(QSDeltaMargin, moveBestCaseValue(board)) < alpha - eval)
        return eval;

    // Step 7. Move Generation and Looping. Generate all tactical moves
    // and return those which are winning via SEE, and also strong enough
    // to beat the margin computed in the Delta Pruning step found above
    initNoisyMovePicker(&movePicker, thread, MAX(1, alpha - eval - QSSeeMargin));
    while ((move = selectNextMove(&movePicker, board, 1)) != NONE_MOVE) {

        // Search the next ply if the move is legal
        if (!apply(thread, board, move)) continue;
        value = -qsearch(thread, &lpv, -beta, -alpha);
        revert(thread, board, move);

        // Improved current value
        if (value > best) {
            best = value;

            // Improved current lower bound
            if (value > alpha) {
                alpha = value;

                // Update the Principle Variation
                pv->length = 1 + lpv.length;
                pv->line[0] = move;
                memcpy(pv->line + 1, lpv.line, sizeof(uint16_t) * lpv.length);
            }
        }

        // Search has failed high
        if (alpha >= beta)
            return best;
    }

    return best;
}

int staticExchangeEvaluation(Board *board, uint16_t move, int threshold) {

    int from, to, type, colour, balance, nextVictim;
    uint64_t bishops, rooks, occupied, attackers, myAttackers;

    // Unpack move information
    from  = MoveFrom(move);
    to    = MoveTo(move);
    type  = MoveType(move);

    // Next victim is moved piece or promotion type
    nextVictim = type != PROMOTION_MOVE
               ? pieceType(board->squares[from])
               : MovePromoPiece(move);

    // Balance is the value of the move minus threshold. Function
    // call takes care for Enpass, Promotion and Castling moves.
    balance = moveEstimatedValue(board, move) - threshold;

    // If the move doesn't gain enough to beat the threshold, don't look any
    // further. This is only relevant for movepicker SEE calls.
    if (balance < 0) return 0;

    // Worst case is losing the moved piece
    balance -= SEEPieceValues[nextVictim];

    // If the balance is positive even if losing the moved piece,
    // the exchange is guaranteed to beat the threshold.
    if (balance >= 0) return 1;

    // Grab sliders for updating revealed attackers
    bishops = board->pieces[BISHOP] | board->pieces[QUEEN];
    rooks   = board->pieces[ROOK  ] | board->pieces[QUEEN];

    // Let occupied suppose that the move was actually made
    occupied = (board->colours[WHITE] | board->colours[BLACK]);
    occupied = (occupied ^ (1ull << from)) | (1ull << to);
    if (type == ENPASS_MOVE) occupied ^= (1ull << board->epSquare);

    // Get all pieces which attack the target square. And with occupied
    // so that we do not let the same piece attack twice
    attackers = allAttackersToSquare(board, occupied, to) & occupied;

    // Now our opponents turn to recapture
    colour = !board->turn;

    while (1) {

        // If we have no more attackers left we lose
        myAttackers = attackers & board->colours[colour];
        if (myAttackers == 0ull) break;

        // Find our weakest piece to attack with
        for (nextVictim = PAWN; nextVictim <= QUEEN; nextVictim++)
            if (myAttackers & board->pieces[nextVictim])
                break;

        // Remove this attacker from the occupied
        occupied ^= (1ull << getlsb(myAttackers & board->pieces[nextVictim]));

        // A diagonal move may reveal bishop or queen attackers
        if (nextVictim == PAWN || nextVictim == BISHOP || nextVictim == QUEEN)
            attackers |= bishopAttacks(to, occupied) & bishops;

        // A vertical or horizontal move may reveal rook or queen attackers
        if (nextVictim == ROOK || nextVictim == QUEEN)
            attackers |=   rookAttacks(to, occupied) & rooks;

        // Make sure we did not add any already used attacks
        attackers &= occupied;

        // Swap the turn
        colour = !colour;

        // Negamax the balance and add the value of the next victim
        balance = -balance - 1 - SEEPieceValues[nextVictim];

        // If the balance is non negative after giving away our piece then we win
        if (balance >= 0) {

            // As a slide speed up for move legality checking, if our last attacking
            // piece is a king, and our opponent still has attackers, then we've
            // lost as the move we followed would be illegal
            if (nextVictim == KING && (attackers & board->colours[colour]))
                colour = !colour;

            break;
        }
    }

    // Side to move after the loop loses
    return board->turn != colour;
}

int singularity(Thread *thread, MovePicker *mp, int ttValue, int depth, int beta) {

    uint16_t move;
    int skipQuiets = 0, quiets = 0, tacticals = 0;
    int value = -MATE, rBeta = MAX(ttValue - depth, -MATE);

    MovePicker movePicker;
    PVariation lpv; lpv.length = 0;
    Board *const board = &thread->board;

    // Table move was already applied
    revert(thread, board, mp->tableMove);

    // Iterate over each move, except for the table move
    initSingularMovePicker(&movePicker, thread, mp->tableMove);
    while ((move = selectNextMove(&movePicker, board, skipQuiets)) != NONE_MOVE) {

        assert(move != mp->tableMove); // Skip the table move

        // Perform a reduced depth search on a null rbeta window
        if (!apply(thread, board, move)) continue;
        value = -search(thread, &lpv, -rBeta-1, -rBeta, depth / 2 - 1);
        revert(thread, board, move);

        // Move failed high, thus mp->tableMove is not singular
        if (value > rBeta) break;

        // Start skipping quiets after a few have been tried
        moveIsTactical(board, move) ? tacticals++ : quiets++;
        skipQuiets = quiets >= SingularQuietLimit;

        // Start skipping bad captures after a few have been tried
        if (skipQuiets && tacticals >= SingularTacticalLimit) break;
    }

    // MultiCut. We signal the Move Picker to terminate the search
    if (value > rBeta && rBeta >= beta) {
        if (!moveIsTactical(board, move))
            updateKillerMoves(thread, move);
        mp->stage = STAGE_DONE;
    }

    // Reapply the table move we took off
    applyLegal(thread, board, mp->tableMove);

    // Move is singular if all other moves failed low
    return value <= rBeta;
}
