#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "bitboards.h"
#include "bitutils.h"
#include "board.h"
#include "castle.h"
#include "evaluate.h"
#include "magics.h"
#include "piece.h"
#include "search.h"
#include "transposition.h"
#include "types.h"
#include "move.h"
#include "movegen.h"
#include "movegentest.h"
#include "zorbist.h"

time_t StartTime;
time_t EndTime;

int TotalNodes;

int EvaluatingPlayer;

uint16_t KillerMoves[MaxHeight][3];
uint16_t KillerCaptures[MaxHeight][3];

TranspositionTable Table;


uint16_t get_best_move(Board * board, int seconds, int logging){
    
    // INITALIZE SEARCH GLOBALS
    StartTime = time(NULL);
    EndTime = StartTime + seconds;
    TotalNodes = 0;    
    EvaluatingPlayer = board->turn;
    init_transposition_table(&Table, 23);
    
    // POPULATE ROOT'S MOVELIST
    MoveList rootMoveList;
    rootMoveList.size = 0;
    gen_all_moves(board,rootMoveList.moves,&(rootMoveList.size));
    
    // PRINT SEARCH DATA TABLE HEADER TO CONSOLE
    if (!logging){
        print_board(board);
        printf("|  Depth  |  Score  |   Nodes   | Elapsed | PV \n");
    }
    
    int depth, value, i, currentValue = evaluate_board(board);
    for (depth = 1; depth < MaxDepth; depth++){
        
        PrincipleVariation localpv = {.length = 0};
        
        // PERFORM FULL SEARCH ON ROOT
        value = full_search(board,&localpv,&rootMoveList,depth);
        
        // LOG RESULTS TO INTERFACE
        if (logging){
            printf("info depth %d score cp %d time %d nodes %d pv ",depth,value,1000*(time(NULL)-StartTime),TotalNodes);
            
            // PRINT THE PRINCIPLE VARIATION
            for (i = 0; i < localpv.length; i++){
                print_move(localpv.line[i]);
                printf(" ");
            } 
            
            printf("\n");
            fflush(stdout);
        }
        
        // LOG RESULTS TO CONSOLE
        else {
            printf("|%9d|%9d|%11d|%9d| ",depth,value,TotalNodes,(time(NULL)-StartTime));
            
            // PRINT THE PRINCIPLE VARIATION
            for (i = 0; i < localpv.length; i++){
                print_move(localpv.line[i]);
                printf(" ");
            } 
            
            printf("\n");
        }
        
        // END THE SEARCH IF THE NEXT DEPTH IS EXPECTED
        // TO TAKE LONGER THAN THE TOTAL ALLOTED TIME
        if ((time(NULL) - StartTime) * 4 >= seconds && value + 25 > currentValue)
            break;        
    }
    
    dump_transposition_table(&Table);
    
    return rootMoveList.bestMove;    
}

int full_search(Board * board, PrincipleVariation * pv, MoveList * moveList, int depth){
    
    int alpha = -2*Mate, beta = 2*Mate;
    int i, valid, best =-2*Mate, value;
    int currentNodes, bestIndex;
    PrincipleVariation localpv = {.length = 0};
    Undo undo[1];    
   
    for (i = 0; i < moveList->size; i++){
        
        currentNodes = TotalNodes;
        
        // APPLY AND VALIDATE MOVE BEFORE SEARCHING
        apply_move(board, moveList->moves[i], undo);
        if (!is_not_in_check(board, !board->turn)){
            revert_move(board, moveList->moves[i], undo);
            moveList->values[i] = -6 * Mate;
            continue;
        }
        
        // INCREMENT COUNTER OF VALID MOVES FOUND
        valid++;
        
        // FULL WINDOW SEARCH ON FIRST MOVE
        if (valid == 1)
            value = -search(board, &localpv, -beta, -alpha, depth-1, 1, PVNODE);
        
        // NULL WINDOW SEARCH ON NON-FIRST MOVES
        else{
            value = -search(board, &localpv, -alpha-1, -alpha, depth-1, 1, CUTNODE);
            
            // NULL WINDOW FAILED HIGH, RESEARCH
            if (value > alpha)
                value = -search(board, &localpv, -beta, -alpha, depth-1, 1, PVNODE);
        }
        
        // REVERT MOVE FROM BOARD
        revert_move(board, moveList->moves[i], undo);
        
        if (value <= alpha)
            moveList->values[i] = -(1<<28) + (TotalNodes - currentNodes); // UPPER VALUE
        else if (value >= beta)
            moveList->values[i] = beta;  // LOWER VALUE
        else
            moveList->values[i] = value; // EXACT VALUE
        
        
        // IMPROVED CURRENT VALUE
        if (value > best){
            best = value;
            bestIndex = i;
            moveList->bestMove = moveList->moves[i];
            
            // IMPROVED CURRENT LOWER VALUE
            if (value > alpha){
                alpha = value;
                
                // UPDATE THE PRINCIPLE VARIATION
                if (localpv.length != -1){
                    pv->line[0] = moveList->moves[i];
                    memcpy(pv->line + 1, localpv.line, sizeof(uint16_t) * localpv.length);
                    pv->length = localpv.length + 1;
                }
            }
        }
        
        // IMPROVED AND FAILED HIGH
        if (alpha >= beta)
            break;        
    }
    
    // SORT MOVELIST FOR NEXT ITERATION
    sort_move_list(moveList);
    return best;
}

int search(Board * board, PrincipleVariation * pv, int alpha, int beta, int depth, int height, int node_type){
    int i, valid = 0, value, size = 0, best=-2*Mate, repeated = 0, newDepth;
    int oldAlpha = alpha, usedTableEntry = 0, inCheck, values[256];
    uint16_t moves[256], bestMove, currentMove, tableMove = NoneMove;
    PrincipleVariation localpv = {.length = 0};
    TranspositionEntry * entry;
    Undo undo[1];
    
    // SEARCH TIME HAS EXPIRED
    if (EndTime < time(NULL)){
        localpv.length = -1;
        return board->turn == EvaluatingPlayer ? -Mate : Mate;
    }
    
    // SEARCH HORIZON REACHED, QSEARCH
    if (depth <= 0){
        localpv.length = -1;
        return qsearch(board, alpha, beta, height);    
    }
    
    // INCREMENT TOTAL NODE COUNTER
    TotalNodes++;
    
    // LOOKUP CURRENT POSITION IN TRANSPOSITION TABLE
    entry = get_transposition_entry(&Table, board->hash);
    
    if (entry != NULL
        && board->turn == entry->turn){
        
        // ENTRY MOVE MAY BE CANDIDATE
        tableMove = entry->best_move;
        
        // ENTRY MAY IMPROVE BOUNDS
        if (USE_TRANSPOSITION_TABLE
            && entry->depth >= depth
            && node_type != PVNODE){
            
            // EXACT VALUE STORED
            if (entry->type == PVNODE)
                return entry->value;            
            
            // LOWER BOUND STORED
            else if (entry->type == CUTNODE && entry->value > alpha)
                alpha = entry->value;
            
            // UPPER BOUND STORED
            else if (entry->type == ALLNODE && entry->value < beta)
                beta = entry->value;
            
            // BOUNDS NOW OVERLAP?
            if (alpha >= beta)
                return entry->value;
            
            usedTableEntry = 1;
            oldAlpha = alpha;
        }
    }   
    
    // DETERMINE 3-FOLD REPITION
    // COMPARING HISTORY TO NULLMOVE IS A HACK
    // TO AVOID CALLING TREES WITH 3-NULL MOVES 
    // APPLIED 3-FOLD REPITITIONS
    for (i = 0; i < board->move_num; i++){
        if (board->history[i] == board->hash
            && board->history[i] != NullMove){
            repeated++;
        }
    }
        
    // 3-FOLD REPITION FOUND
    if (repeated >= 2){
        return 0;
    }
    
    // RAZOR PRUNING
    if (USE_RAZOR_PRUNING
        && depth <= 3
        && node_type != PVNODE
        && alpha == beta - 1
        && evaluate_board(board) + QueenValue < beta){
        
        value = qsearch(board, alpha, beta, height);
        
        // EVEN GAINING A QUEEN WOULD FAIL LOW
        if (value < beta)
            return value;
    }        
    
    // USE NULL MOVE PRUNING
    if (USE_NULL_MOVE_PRUNING
        && depth >= 3 
        && abs(beta) < Mate - MaxHeight
        && alpha == beta - 1
        && node_type != PVNODE
        && board->history[board->move_num-1] != NullMove
        && is_not_in_check(board, board->turn)
        && evaluate_board(board) >= beta){
            
        // APPLY NULL MOVE
        board->turn = !board->turn;
        board->history[board->move_num++] == NullMove;
        
        // PERFORM NULL MOVE SEARCH
        value = -search(board, &localpv, -beta, -beta+1, depth-4, height+1, CUTNODE);
        
        // REVERT NULL MOVE
        board->move_num--;
        board->turn = !board->turn;
        
        if (value >= beta)
            return value;
    }
    
    // INTERNAL ITERATIVE DEEPING
    if (USE_INTERNAL_ITERATIVE_DEEPENING
        && depth >= 3
        && tableMove == NoneMove
        && node_type == PVNODE){
        
        // SEARCH AT A LOWER DEPTH
        value = search(board, &localpv, alpha, beta, depth-3, height, PVNODE);
        if (value <= alpha)
            value = search(board, &localpv, -Mate, beta, depth-3, height, PVNODE);
        
        // GET CANDIDATE MOVE FROM LOCAL PRINCIPLE VARIATION
        tableMove = localpv.line[0];
    }
    
    // GENERATE AND PREPARE MOVE ORDERING
    gen_all_moves(board, moves, &size);
    evaluate_moves(board, values, moves, size, height, tableMove, pv->line[height]);
    
    // DETERMINE CHECK STATUS FOR LATE MOVE REDUCTIONS
    inCheck = !is_not_in_check(board, board->turn);
    
    for (i = 0; i < size; i++){
        
        currentMove = get_next_move(moves, values, i, size);
        
        // APPLY AND VALIDATE MOVE BEFORE SEARCHING
        apply_move(board, currentMove, undo);
        if (!is_not_in_check(board, !board->turn)){
            revert_move(board, currentMove, undo);
            continue;
        }   
    
        // INCREMENT COUNTER OF VALID MOVES FOUND
        valid++;
        
        // DETERMINE IF WE CAN USE LATE MOVE REDUCTIONS
        if (USE_LATE_MOVE_REDUCTIONS
            && usedTableEntry
            && valid >= 4
            && depth >= 3
            && !inCheck
            && node_type != PVNODE
            && MOVE_TYPE(currentMove) == NormalMove
            && undo[0].capture_piece == Empty
            && is_not_in_check(board, board->turn))
            newDepth = depth-2;
        else
            newDepth = depth-1;
        
            
        // FULL WINDOW SEARCH ON FIRST MOVE
        if (valid == 1 || node_type != PVNODE){
            value = -search(board, &localpv, -beta, -alpha, newDepth, height+1, node_type);
            
            // IMPROVED BOUND, BUT WAS REDUCED DEPTH?
            if (value > alpha
                && newDepth == depth-2){
                    
                value = -search(board, &localpv, -beta, -alpha, depth-1, height+1, node_type);
            }
        }
        
        // NULL WINDOW SEARCH ON NON-FIRST / PV MOVES
        else{
            value = -search(board, &localpv, -alpha-1, -alpha, newDepth, height+1, CUTNODE);
            
            // NULL WINDOW FAILED HIGH, RESEARCH
            if (value > alpha){
                value = -search(board, &localpv, -beta, -alpha, depth-1, height+1, PVNODE);

            }
        }
        
        // REVERT MOVE FROM BOARD
        revert_move(board, currentMove, undo);
        
        // IMPROVED CURRENT VALUE
        if (value > best){
            best = value;
            bestMove = currentMove;
            
            // IMPROVED CURRENT LOWER VALUE
            if (value > alpha){
                alpha = value;
                
                // UPDATE THE PRINCIPLE VARIATION
                if (localpv.length != -1){
                    pv->line[0] = currentMove;
                    memcpy(pv->line + 1, localpv.line, sizeof(uint16_t) * localpv.length);
                    pv->length = localpv.length + 1;
                }
            }
        }
        
        // IMPROVED AND FAILED HIGH
        if (alpha >= beta){
            
            // UPDATE QUIET-KILLER MOVES
            if (undo[0].capture_piece == Empty || MOVE_TYPE(currentMove) != NormalMove){
                KillerMoves[height][2] = KillerMoves[height][1];
                KillerMoves[height][1] = KillerMoves[height][0];
                KillerMoves[height][0] = currentMove;
            }
            
            // UPDATE NOISY-KILLER MOVES
            else {
                KillerCaptures[height][2] = KillerCaptures[height][1];
                KillerCaptures[height][1] = KillerCaptures[height][0];
                KillerCaptures[height][0] = currentMove;
            }
            
            goto Cut;
        }
    }
    
    // BOARD IS STALEMATE OR CHECKMATE
    if (valid == 0){
        
        // BOARD IS STALEMATE
        if (is_not_in_check(board, board->turn))
            return 0;
        
        // BOARD IS CHECKMATE
        else 
            return -Mate+height;
        
    }
    
    Cut:
    
    // STORE RESULTS IN TRANSPOSITION TABLE
    
    // EXACT NODE FOUND, REPLACE EVEN IF WE USED AN ENTRY HERE
    if (best > oldAlpha && best < beta)
        store_transposition_entry(&Table, depth, board->turn, PVNODE, best, bestMove, board->hash);
    
    // CUT OR ALL NODE, REPLACE ONLY IF WE DID NOT USE AN ENTRY HERE
    else {
        
        // UPPER BOUND
        if (best >= beta)
            store_transposition_entry(&Table, depth, board->turn, CUTNODE, best, bestMove, board->hash);
        
        // LOWER BOUND
        else if (best <= oldAlpha)
            store_transposition_entry(&Table, depth, board->turn, ALLNODE, best, bestMove, board->hash);
    }
    
    return best;    
}

int qsearch(Board * board, int alpha, int beta, int height){
    int i, delta, size = 0, value, best = -2*Mate, values[256];
    uint16_t moves[256], bestMove, currentMove;
    TranspositionEntry * entry;
    Undo undo[1];
    
    // MAX HEIGHT REACHED, STOP HERE
    if (height >= MaxHeight)
        return evaluate_board(board);
    
    // GET A STANDING-EVAL OF THE CURRENT BOARD
    value = evaluate_board(board);
    
    // UPDATE LOWER BOUND
    if (value > alpha)
        alpha = value;
    
    // BOUNDS NOW OVERLAP?
    if (alpha >= beta)
        return value;
    
    // DETERMINE DELTA VALUE
    delta = QueenValue;
    if (board->pieceBitBoards[0] & board->colourBitBoards[0] & RANK_8
        || board->pieceBitBoards[0] & board->colourBitBoards[1] & RANK_1)
        delta += QueenValue - PawnValue;

    // DELTA PRUNING
    if (value + delta < alpha)
        return alpha;
    
    // INCREMENT TOTAL NODE COUNTER
    TotalNodes++;
    
    // GENERATE AND PREPARE QUIET MOVE ORDERING
    gen_all_non_quiet(board, moves, &size);
    evaluate_moves(board, values, moves, size, height, NoneMove, NoneMove);
    
    best = value;
    
    for (i = 0; i < size; i++){
        currentMove = get_next_move(moves, values, i, size);
        
        // APPLY AND VALIDATE MOVE BEFORE SEARCHING
        apply_move(board, currentMove, undo);
        if (!is_not_in_check(board, !board->turn)){
            revert_move(board, currentMove, undo);
            continue;
        }
        
        // SEARCH NEXT DEPTH
        value = -qsearch(board, -beta, -alpha, height+1);
        
        // REVERT MOVE FROM BOARD
        revert_move(board, currentMove, undo);
        
        // IMPROVED CURRENT VALUE
        if (value > best){
            best = value;
            
            // IMPROVED CURRENT LOWER VALUE
            if (value > alpha)
                alpha = value;
        }
        
        // IMPROVED AND FAILED HIGH
        if (alpha >= beta){
            
            // UPDATE NOISY-KILLER MOVES
            KillerCaptures[height][2] = KillerCaptures[height][1];
            KillerCaptures[height][1] = KillerCaptures[height][0];
            KillerCaptures[height][0] = currentMove;
            
            break;
        }
    }
    
    return best;
}

void evaluate_moves(Board * board, int * values, uint16_t * moves, int size, int height, uint16_t tableMove, uint16_t pvMove){
    int i, value;
    int from_type, to_type;
    int from_val, to_val;
    
    // GET KILLER MOVES
    uint16_t killer1 = KillerMoves[height][0];
    uint16_t killer2 = KillerMoves[height][1];
    uint16_t killer3 = KillerMoves[height][2];
    
    uint16_t killer4 = KillerCaptures[height][0];
    uint16_t killer5 = KillerCaptures[height][1];
    uint16_t killer6 = KillerCaptures[height][2];
    
    for (i = 0; i < size; i++){
        
        // TABLEMOVE FIRST
        value  = 8192 * ( tableMove == moves[i]);
        
        // PRINCIPLE VARIATION NEXT
        value += 4096 * (    pvMove == moves[i]);
        
        // THEN KILLERS, UNLESS OTHER GOOD CAPTURE
        value += 128  * (   killer1 == moves[i]);
        value += 128  * (   killer2 == moves[i]);
        value += 128  * (   killer3 == moves[i]);
        value += 256  * (   killer4 == moves[i]);
        value += 256  * (   killer5 == moves[i]);
        value += 256  * (   killer6 == moves[i]);
        
        // INFO FOR POSSIBLE CAPTURE
        from_type = PIECE_TYPE(board->squares[MOVE_FROM(moves[i])]);
        to_type = PIECE_TYPE(board->squares[MOVE_TO(moves[i])]);
        from_val = PieceValues[from_type];
        to_val = PieceValues[to_type];
        
        // ENCOURAGE CAPTURING HIGH VALUE WITH LOW VALUE
        value += 5 * to_val;
        value -= 1 * from_val;
        
        // ENPASS CAPTURE IS TREATED SEPERATLY
        if (MOVE_TYPE(moves[i]) == EnpassMove)
            value += 2*PawnValue;
        
        // WE ARE ONLY CONCERED WITH QUEEN PROMOTIONS
        if (MOVE_TYPE(moves[i]) == PromotionMove)
            value += QueenValue * (moves[i] & PromoteToQueen);
        
        values[i] = value;
    }
}

uint16_t get_next_move(uint16_t * moves, int * values, int index, int size){
    int i, best = 0;
    uint16_t bestMove;
    
    // FIND GREATEST VALUE
    for (i = 1; i < size-index; i++)
        if (values[i] > values[best])
            best = i;
        
    bestMove = moves[best];
    
    // MOVE LAST PAIR TO BEST SO WE
    // CAN REDUCE THE EFFECTIVE LIST SIZE
    moves[best] = moves[size-index-1];
    values[best] = values[size-index-1];
    
    return bestMove;
}

void sort_move_list(MoveList * moveList){
    int i, j, temp_val;
    uint16_t temp_move;
    
    for (i = 0; i < moveList->size; i++){
        for (j = i+1; j < moveList->size; j++){
            if (moveList->values[j] > moveList->values[i]){
                temp_val = moveList->values[j];
                temp_move = moveList->moves[j];
                
                moveList->values[j] = moveList->values[i];
                moveList->moves[j] = moveList->moves[i];
                
                moveList->values[i] = temp_val;
                moveList->moves[i] = temp_move;
            }
        }
    }    
}