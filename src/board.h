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

#ifndef _BOARD_H
#define _BOARD_H

#include "types.h"

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
    int midgame;
    int endgame;
    int numMoves;
    uint64_t history[512];
} Board;

typedef struct Undo {
    uint64_t hash;
    uint64_t phash;
    int turn;
    int castleRights;
    int epSquare;
    int fiftyMoveRule;
    int midgame;
    int endgame;
    int captureSquare;
    int capturePiece;
} Undo;

void initializeBoard(Board* board, char* fen);
void printBoard(Board* board);
uint64_t perft(Board* board, int depth);
void runBenchmark(Thread* threads, int depth);

#endif
