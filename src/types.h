#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

#define Mate        (16000)
#define MaxDepth    (32)
#define MaxHeight   (128)

typedef struct Board {
    int squares[64];
    uint64_t colourBitBoards[3];
    uint64_t pieceBitBoards[7]; 
    int turn;
    int castleRights;
    int fiftyMoveRule;
    int epSquare;
    uint64_t phash;
    uint64_t hash;
    int opening;
    int endgame;
    uint64_t history[2048];
    int numMoves;
    int numPieces;
    int hasCastled[2];
    
} Board;

typedef struct Undo {
    int captureSquare;
    int capturePiece;
    int turn;
    int castleRights;
    int opening;
    int endgame;
    int epSquare;
    uint64_t phash;
    uint64_t hash;
    int numPieces;
    
} Undo;

typedef struct TransEntry {
    uint8_t depth;
    uint8_t data;
    int16_t value;
    uint16_t bestMove;
    uint16_t hash16;
    
} TransEntry;

typedef struct TransBucket {
    TransEntry entries[4];
    
} TransBucket;

typedef struct TransTable {
    TransBucket * buckets;
    uint32_t maxSize;
    uint32_t keySize;
    uint8_t generation;
    uint32_t used;
    
} TransTable;

typedef struct MoveList {
    uint16_t moves[256];
    int values[256];
    uint16_t bestMove;
    int size;
    
} MoveList;

typedef struct SearchInfo {
    Board board;
    int searchIsInfinite;
    int searchIsDepthLimited;
    int searchIsTimeLimited;
    int depthLimit;
    int terminateSearch;
    double startTime;
    double endTime1;
    double endTime2;
    
} SearchInfo;

typedef struct PawnEntry {
    uint64_t phash;
    int mg, eg;
    
} PawnEntry;

typedef struct PawnTable {
    PawnEntry * entries;
    
} PawnTable;

#endif