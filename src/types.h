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

#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

#define MATE        (16000)
#define MAX_DEPTH   (64)
#define MAX_HEIGHT  (128)
#define MAX_MOVES   (256)

#define SQUARE_NB   (64)
#define COLOUR_NB   ( 2)
#define RANK_NB     ( 8)
#define FILE_NB     ( 8)
#define PHASE_NB    ( 2)

typedef struct Board {
    uint8_t squares[SQUARE_NB];
    uint64_t pieces[8]; 
    uint64_t colours[3];
    uint64_t hash;
    uint64_t phash;
    int turn;
    int castleRights;
    int epSquare;
    int fiftyMoveRule;
    int opening;
    int endgame;
    int numMoves;
    int hasCastled[2];
    uint64_t history[256];
    
} Board;

typedef struct Undo {    
    uint64_t hash;
    uint64_t phash;
    int turn;
    int castleRights;
    int epSquare;
    int fiftyMoveRule;
    int opening;
    int endgame;
    int captureSquare;
    int capturePiece;
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
    uint64_t numBuckets;
    uint64_t used;
    uint64_t keySize;
    uint8_t generation;
    
} TransTable;

typedef struct MoveList {
    uint16_t moves[MAX_MOVES];
    int values[MAX_MOVES];
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
    uint64_t passed;
    int mg, eg;
    
} PawnEntry;

typedef struct PawnTable {
    PawnEntry * entries;
    
} PawnTable;

typedef struct PVariation {
    uint16_t line[MAX_HEIGHT];
    int length;
    
} PVariation;

#endif