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

#pragma once

#include "types.h"

extern const char *PieceLabel[COLOUR_NB];

struct Board {
    uint8_t squares[SQUARE_NB];
    uint64_t pieces[8], colours[3];
    uint64_t hash, pkhash, kingAttackers;
    uint64_t castleRooks, castleMasks[SQUARE_NB];
    int turn, epSquare, halfMoveCounter, fullMoveCounter;
    int psqtmat, numMoves, chess960;
    uint64_t history[512];
    Thread *thread;
};

struct Undo {
    uint64_t hash, pkhash, kingAttackers, castleRooks;
    int epSquare, halfMoveCounter, psqtmat, capturePiece;
};

void squareToString(int sq, char *str);
void boardFromFEN(Board *board, const char *fen, int chess960);
void boardToFEN(Board *board, char *fen);
void printBoard(Board *board);
int boardHasNonPawnMaterial(Board *board, int turn);
int boardIsDrawn(Board *board, int height);
int boardDrawnByFiftyMoveRule(Board *board);
int boardDrawnByRepetition(Board *board, int height);
int boardDrawnByInsufficientMaterial(Board *board);

uint64_t perft(Board *board, int depth);
