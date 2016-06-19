#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

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

uint16_t KillerMoves[MaxHeight][2];

TransTable Table;
PawnTable PTable;

int HistoryGood[0xFFFF];
int HistoryTotal[0xFFFF];

SearchInfo * Info;

uint16_t getBestMove(SearchInfo * info){
    
    int i, depth, value = 0;
    int centiValue, deltaTime, hashPercent;
    
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
    for (i = 0; i < 0xFFFF; i++){
        HistoryGood[i] = 1;
        HistoryTotal[i] = 1;
    }
    
    printBoard(&(info->board));
    
    // PERFORM ITERATIVE DEEPENING
    for (depth = 1; depth < MaxDepth; depth++){
        
        // PERFORM FULL SEARCH ON ROOT
        //value = rootSearch(&(info->board),&rootMoveList,-Mate*2,Mate*2,depth);
        value = aspirationWindow(&(info->board), &rootMoveList, depth, value);
        
        // LOG RESULTS TO INTERFACE
        centiValue = (100*value)/PawnValue;
        deltaTime = getRealTime() - info->startTime;
        hashPercent = (1000*Table.used)/(4*Table.maxSize);
        printf("info depth %d score cp %d time %d nodes %d hashfull %d pv ",depth,centiValue,deltaTime,TotalNodes,hashPercent);
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
    
    // FREE PAWN TABLE
    destoryPawnTable(&PTable);
    
    // RETURN BEST MOVE
    return rootMoveList.bestMove;
}

int aspirationWindow(Board * board, MoveList * moveList, int depth, int previousScore){
    
    int alpha, beta, value, margin;
    
    if (depth > 4 && previousScore < Mate/2){
        for (margin = 30; margin < 250; margin *= 2){
            alpha = previousScore - margin;
            beta  = previousScore + margin;
            
            value = rootSearch(board, moveList, alpha, beta, depth);
            
            if (value > alpha && value < beta)
                return value;
            
            if (value > Mate/2)
                break;
        }
    }
    
    return rootSearch(board, moveList, -Mate*2, Mate*2, depth);
}

int rootSearch(Board * board, MoveList * moveList, int alpha, int beta, int depth){
    
    int i, valid = 0, best =-2*Mate, value;
    int currentNodes;
    Undo undo[1];
   
    for (i = 0; i < moveList->size; i++){
        
        currentNodes = TotalNodes;
        
        // APPLY AND VALIDATE MOVE BEFORE SEARCHING
        applyMove(board, moveList->moves[i], undo);
        if (!isNotInCheck(board, !board->turn)){
            revertMove(board, moveList->moves[i], undo);
            moveList->values[i] = -6 * Mate;
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
    
    // STORE IN TRANSPOSITION TABLE
    if (!Info->searchIsTimeLimited || getRealTime() < Info->endTime2)
        storeTranspositionEntry(&Table, depth, board->turn, PVNODE, best, moveList->bestMove, board->hash);
    
    // SORT MOVELIST FOR NEXT ITERATION
    sortMoveList(moveList);
    return best;
}

int alphaBetaSearch(Board * board, int alpha, int beta, int depth, int height, int nodeType){
    
    int i, value, newDepth, entryValue, entryType;
    int min, max, inCheck, values[256];
    int valid = 0, size = 0, repeated = 0;
    int oldAlpha = alpha, best = -2*Mate, optimalValue = -Mate;
    
    uint16_t currentMove, tableMove = NoneMove, bestMove = NoneMove;
    uint16_t moves[256], played[256];
    
    TransEntry * entry;
    Undo undo[1];
    
    // SEARCH TIME HAS EXPIRED
    if (Info->searchIsTimeLimited && getRealTime() >= Info->endTime2)
        return board->turn == EvaluatingPlayer ? -Mate : Mate;
    
    // DETERMINE 3-FOLD REPITION
    for (i = board->numMoves-2; i >= 0; i-=2)
        if (board->history[i] == board->hash)
            repeated++;
    if (repeated >= 2)
        return 0;
    
    // SEARCH HORIZON REACHED, QSEARCH
    if (depth <= 0)
        return quiescenceSearch(board, alpha, beta, height);
    
    // INCREMENT TOTAL NODE COUNTER
    TotalNodes++;
    
    // LOOKUP CURRENT POSITION IN TRANSPOSITION TABLE
    entry = getTranspositionEntry(&Table, board->hash, board->turn);
    
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
                max = entryValue < best ? entryValue : beta;
            
            // BOUNDS NOW OVERLAP?
            if (min >= max)
                return entryValue;
        }
    }
    
    // DETERMINE CHECK STATUS
    inCheck = !isNotInCheck(board, board->turn);
    
    // STATIC NULL MOVE PRUNING
    if (USE_STATIC_NULL_PRUNING
        && depth <= 3
        && nodeType != PVNODE
        && alpha == beta - 1
        && !inCheck){
            
        value = evaluateBoard(board, &PTable) - (depth * (PawnValue + 15));
        
        if (value > beta)
            return value;
    }
    
    // RAZOR PRUNING
    if (USE_RAZOR_PRUNING
        && depth <= 3
        && nodeType != PVNODE
        && alpha == beta - 1
        && evaluateBoard(board, &PTable) + RazorMargins[depth] < beta){
        
        value = quiescenceSearch(board, alpha, beta, height);
        
        // EVEN GAINING A LARGE MARGIN WOULD FAIL LOW
        if (value < beta)
            return value;
    }
    
    // NULL MOVE PRUNING
    if (USE_NULL_MOVE_PRUNING
        && depth >= 2
        && nodeType != PVNODE
        && canDoNull(board)
        && !inCheck
        && evaluateBoard(board, &PTable) >= beta){
            
        // APPLY NULL MOVE
        board->turn = !board->turn;
        board->history[board->numMoves++] = NullMove;
        
        int R = (40 + (depth << 2)) >> 4;
        
        // PERFORM NULL MOVE SEARCH
        value = -alphaBetaSearch(board, -beta, -beta+1, depth-R, height+1, CUTNODE);
        
        // REVERT NULL MOVE
        board->numMoves--;
        board->turn = !board->turn;
        
        if (value >= beta)
            return value;
    }
    
    // INTERNAL ITERATIVE DEEPING
    if (USE_INTERNAL_ITERATIVE_DEEPENING
        && depth >= 3
        && tableMove == NoneMove
        && nodeType == PVNODE){
        
        // SEARCH AT A LOWER DEPTH
        value = alphaBetaSearch(board, alpha, beta, depth-2, height, PVNODE);
        if (value <= alpha)
            value = alphaBetaSearch(board, -Mate, beta, depth-2, height, PVNODE);
        
        // GET TABLE MOVE FROM TRANSPOSITION TABLE
        entry = getTranspositionEntry(&Table, board->hash, board->turn);
        if (entry != NULL)
            tableMove = entry->bestMove;
    }
    
    // GENERATE AND PREPARE MOVE ORDERING
    genAllMoves(board, moves, &size);
    evaluateMoves(board, values, moves, size, height, tableMove);
   
    // CHECK EXTENSION
    depth += inCheck;
    
    for (i = 0; i < size; i++){
        
        currentMove = getNextMove(moves, values, i, size);
        
        // USE FUTILITY PRUNING
        if (USE_FUTILITY_PRUNING
            && nodeType != PVNODE
            && valid >= 1
            && depth == 1
            && !inCheck
            && MoveType(currentMove) == NormalMove
            && board->squares[MoveTo(currentMove)] == Empty){
                
            if (optimalValue == -Mate)
                optimalValue = evaluateBoard(board, &PTable) + PawnValue;
            
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
            && depth >= 4
            && played[0] == tableMove
            && !inCheck
            && nodeType != PVNODE
            && MoveType(currentMove) == NormalMove
            && undo[0].capturePiece == Empty
            && isNotInCheck(board, board->turn))
            newDepth = depth-2;
        else
            newDepth = depth-1;
         
        // FULL WINDOW SEARCH ON FIRST MOVE
        if (valid == 1 || nodeType != PVNODE){
            
            value = -alphaBetaSearch(board, -beta, -alpha, newDepth, height+1, nodeType);
            
            // IMPROVED BOUND, BUT WAS REDUCED DEPTH?
            if (value > alpha
                && newDepth == depth-2){
                    
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
            if (MoveType(currentMove) == NormalMove
                && undo[0].capturePiece == Empty
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
            return -Mate+height;
    }
    
    Cut:
    
    if (best >= beta && bestMove != NoneMove){
        HistoryGood[bestMove]++;
    
        for (i = valid - 1; i >= 0; i--){
            HistoryTotal[played[i]]++;
            if (HistoryTotal[played[i]] >= 16384){
                HistoryTotal[played[i]] = (HistoryTotal[played[i]] + 1) / 2;
                HistoryGood[played[i]] = (HistoryGood[played[i]] + 1) / 2;
            }
        }
    }
    
    // STORE RESULTS IN TRANSPOSITION TABLE
    if (!Info->searchIsTimeLimited || getRealTime() < Info->endTime2){
        if (best > oldAlpha && best < beta)
            storeTranspositionEntry(&Table, depth, board->turn,  PVNODE, best, bestMove, board->hash);
        else if (best >= beta)
            storeTranspositionEntry(&Table, depth, board->turn, CUTNODE, best, bestMove, board->hash);
        else if (best <= oldAlpha)
            storeTranspositionEntry(&Table, depth, board->turn, ALLNODE, best, bestMove, board->hash);
    }
    
    return best;
}

int quiescenceSearch(Board * board, int alpha, int beta, int height){
    int i, size = 0, value = -2*Mate, best = -2*Mate, values[256];
    uint16_t moves[256], bestMove, currentMove;
    Undo undo[1];
    
    // MAX HEIGHT REACHED, STOP HERE
    if (height >= MaxHeight)
        return evaluateBoard(board, &PTable);
    
    // INCREMENT TOTAL NODE COUNTER
    TotalNodes++;
    
    // GET A STANDING-EVAL OF THE CURRENT BOARD
    value = evaluateBoard(board, &PTable);
    
    // UPDATE LOWER BOUND
    if (value > alpha)
        alpha = value;
    
    // BOUNDS NOW OVERLAP?
    if (alpha >= beta)
        return value;
    
    // DELTA PRUNING IN WHEN NO PROMOTIONS AND NOT EXTREME LATE GAME
    if (value + QueenValue < alpha
        && board->numPieces >= 6 
        && !(board->colourBitBoards[0] & board->pieceBitBoards[0] & RANK_7)
        && !(board->colourBitBoards[1] & board->pieceBitBoards[0] & RANK_2))
        return alpha;
    
    
    // GENERATE AND PREPARE QUIET MOVE ORDERING
    genAllNonQuiet(board, moves, &size);
    evaluateMoves(board, values, moves, size, height, NoneMove);
    
    best = value;
    
    for (i = 0; i < size; i++){
        currentMove = getNextMove(moves, values, i, size);
        
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
    int from_type, to_type;
    int from_val, to_val;
    
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
        from_type = PieceType(board->squares[MoveFrom(moves[i])]);
        to_type = PieceType(board->squares[MoveTo(moves[i])]);
        from_val = PieceValues[from_type];
        to_val = PieceValues[to_type];
        
        // ENCOURAGE CAPTURING HIGH VALUE
        value += 5 * to_val;
        
        // ENPASS CAPTURE IS TREATED SEPERATLY
        if (MoveType(moves[i]) == EnpassMove)
            value += 2*PawnValue;
        
        // WE ARE ONLY CONCERED WITH QUEEN PROMOTIONS
        else if (MoveType(moves[i]) & PromoteToQueen)
            value += 5*QueenValue;
        
        // CASTLING IS USUALLY A GOOD MOVE
        else if (MoveType(moves[i]) == CastleMove)
            value += 256;
        
        // PIECE SQUARE TABLE ORDERING
        value += PSQTopening[from_type][MoveTo(moves[i])] - PSQTopening[from_type][MoveFrom(moves[i])];
        
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
    uint64_t friendly = board->colourBitBoards[board->turn];
    uint64_t kings = board->pieceBitBoards[5];
    
    return (friendly & kings) != friendly;
}