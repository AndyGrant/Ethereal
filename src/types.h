#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

#define Mate        (100000)
#define MaxDepth    (32)
#define MaxHeight   (128)

typedef struct Board {
    
    // Easy Lookups for piece types
    int squares[64];
    
    // White + Black + Empty
    uint64_t colourBitBoards[3];
    
    // Each Piece + Empty
    uint64_t pieceBitBoards[7]; 
    
    // Necessary data members
    int turn;
    int castleRights;
    int fiftyMoveRule;
    int epSquare;
    
    // Zorbist Key
    uint64_t hash;
    
    // Material + PST values
    int opening;
    int endgame;
    
    // Move history
    uint64_t history[2048];
    int numMoves;
    
    // Keep Track of number of pieces
    int numPieces;
    
} Board;

typedef struct Undo {
    
    // Fast undo of captures
    int captureSquare;
    int capturePiece;
    
    // Previous turn and Castle Rights
    int turn;
    int castleRights;
    
    // Previous Material + PST values
    int opening;
    int endgame;
    
    // Previous EP Square
    int epSquare;
    
    // Previous Zorbist key
    uint64_t hash;
    
    // Keep Track of number of pieces
    int numPieces;
    
} Undo;

typedef struct TranspositionEntry {
    int8_t age;
    int8_t depth;
    int8_t turn;
    int8_t type;
    int value;
    uint16_t bestMove;
    uint64_t hash;  
    
} TranspositionEntry;

typedef struct TranspositionBucket {
    TranspositionEntry entry1;
    TranspositionEntry entry2;
    
} TranspositionBucket;

typedef struct TranspositionTable {
    TranspositionBucket * buckets;
    int maxSize;
    int keySize;
    
} TranspositionTable;

typedef struct MoveList {
    uint16_t moves[256];
    int values[256];
    uint16_t bestMove;
    int size;
    
} MoveList;

#endif