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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboards.h"
#include "board.h"
#include "move.h"
#include "network.h"
#include "thread.h"
#include "types.h"

PKNetwork PKNN;

static char *PKWeights[] = {
    #include "weights/pknet_224x32x1.net"
    ""
};

static void vectorizePKNetwork(const Board *board, bool *inputs) {

    int index = 0;

    for (int colour = WHITE; colour <= BLACK; colour++) {

        uint64_t ours  = board->colours[colour];
        uint64_t pawns = ours & board->pieces[PAWN];
        uint64_t kings = ours & board->pieces[KING];

        for (int sq = 0; sq < SQUARE_NB; sq++)
            if (!testBit(PROMOTION_RANKS, sq))
                inputs[index++] = testBit(pawns, sq);

        for (int sq = 0; sq < SQUARE_NB; sq++)
            inputs[index++] = testBit(kings, sq);
    }

    assert(index == PKNETWORK_INPUTS);
}

static int computePKNetworkIndex(int colour, int piece, int sq) {
    return (64 + 48) * colour
         + (48 * (piece == KING))
         + sq - 8 * (piece == PAWN);
}


void initPKNetwork() {

    for (int i = 0; i < PKNETWORK_LAYER1; i++) {

        // Grab the next line and tokenize it
        char weights[strlen(PKWeights[i]) + 1];
        strcpy(weights, PKWeights[i]);
        strtok(weights, " ");

        for (int j = 0; j < PKNETWORK_INPUTS; j++)
            PKNN.inputWeights[i][j] = atof(strtok(NULL, " "));
        PKNN.inputBiases[i] = atof(strtok(NULL, " "));
    }

    for (int i = 0; i < PKNETWORK_OUTPUTS; i++) {

        // Grab the next line and tokenize it
        char weights[strlen(PKWeights[i + PKNETWORK_LAYER1]) + 1];
        strcpy(weights, PKWeights[i + PKNETWORK_LAYER1]);
        strtok(weights, " ");

        for (int j = 0; j < PKNETWORK_LAYER1; j++)
            PKNN.layer1Weights[i][j] = atof(strtok(NULL, " "));
        PKNN.layer1Biases[i] = atof(strtok(NULL, " "));
    }
}

int fullyComputePKNetwork(Thread *thread) {

    bool inputsNeurons[PKNETWORK_INPUTS];
    float layer1Neurons[PKNETWORK_LAYER1];
    float outputNeurons[PKNETWORK_OUTPUTS];

    vectorizePKNetwork(&thread->board, inputsNeurons);

    for (int i = 0; i < PKNETWORK_LAYER1; i++) {
        layer1Neurons[i] = PKNN.inputBiases[i];
        for (int j = 0; j < PKNETWORK_INPUTS; j++)
            layer1Neurons[i] += inputsNeurons[j] * PKNN.inputWeights[i][j];
    }

    for (int i = 0; i < PKNETWORK_OUTPUTS; i++) {
        outputNeurons[i] = PKNN.layer1Biases[i];
        for (int j = 0; j < PKNETWORK_LAYER1; j++)
            if (layer1Neurons[j] >= 0.0)
                outputNeurons[i] += layer1Neurons[j] * PKNN.layer1Weights[i][j];
    }

    return outputNeurons[0];
}

int partiallyComputePKNetwork(Thread *thread) {

    float *layer1Neurons = thread->pknnlayer1[thread->pknndepth];
    float outputNeurons[PKNETWORK_OUTPUTS];

    for (int i = 0; i < PKNETWORK_OUTPUTS; i++) {
        outputNeurons[i] = PKNN.layer1Biases[i];
        for (int j = 0; j < PKNETWORK_LAYER1; j++)
            if (layer1Neurons[j] >= 0.0)
                outputNeurons[i] += layer1Neurons[j] * PKNN.layer1Weights[i][j];
    }

    return outputNeurons[0];
}


void initPKNetworkCollector(Thread *thread) {

    assert(thread->pknndepth == 0);

    bool inputsNeurons[PKNETWORK_INPUTS];
    vectorizePKNetwork(&thread->board, inputsNeurons);

    for (int i = 0; i < PKNETWORK_LAYER1; i++) {
        thread->pknnlayer1[0][i] = PKNN.inputBiases[i];
        for (int j = 0; j < PKNETWORK_INPUTS; j++)
            thread->pknnlayer1[0][i] += inputsNeurons[j] * PKNN.inputWeights[i][j];
    }
}

void updatePKNetworkIndices(Thread *thread, int changes, int indexes[3], int signs[3]) {

    float *layer1Neurons_d1 = thread->pknnlayer1[thread->pknndepth];
    float *layer1Neurons    = thread->pknnlayer1[++thread->pknndepth];

    thread->pknnchanged[thread->height-1] = 1;
    memcpy(layer1Neurons, layer1Neurons_d1, sizeof(float) * PKNETWORK_LAYER1);

    for (int j = 0; j < changes; j++)
        for (int i = 0; i < PKNETWORK_LAYER1; i++)
            layer1Neurons[i] += signs[j] * PKNN.inputWeights[i][indexes[j]];
}

void updatePKNetworkAfterMove(Thread *thread, uint16_t move) {

    int to     =  MoveTo(move);
    int from   =  MoveFrom(move);
    int type   =  MoveType(move);
    int colour = !thread->board.turn;
    int moved  =  pieceType(thread->board.squares[to]);
    int taken  =  pieceType(thread->undoStack[thread->height-1].capturePiece);

    int changes = 0, indexes[3], signs[3];

    thread->pknnchanged[thread->height-1] = 0;

    if (move == NULL_MOVE)
        return;

    if (type == NORMAL_MOVE) {

        if (moved == PAWN || moved == KING) {
            indexes[changes++] = computePKNetworkIndex(colour, moved, from);
            indexes[changes++] = computePKNetworkIndex(colour, moved, to  );
            signs[0] = -1; signs[1] = 1;
        }

        if (taken == PAWN) {
            assert(0);
            indexes[changes++] = computePKNetworkIndex(!colour, taken, to);
            signs[changes - 1] = -1;
        }
    }

    else if (type == CASTLE_MOVE) {
        indexes[changes++] = computePKNetworkIndex(colour, KING, from);
        indexes[changes++] = computePKNetworkIndex(colour, KING, castleKingTo(from, to));
        signs[0] = -1; signs[1] = 1;
    }

    else if (type == ENPASS_MOVE) {
        indexes[changes++] = computePKNetworkIndex(colour, PAWN, from);
        indexes[changes++] = computePKNetworkIndex(colour, PAWN, to);
        indexes[changes++] = computePKNetworkIndex(!colour, PAWN, to ^ 8);
        signs[0] = -1; signs[1] = 1; signs[2] = -1;
    }

    else if (type == PROMOTION_MOVE) {
        indexes[changes++] = computePKNetworkIndex(colour, PAWN, from);
        signs[0] = -1;
    }

    if (changes)
        updatePKNetworkIndices(thread, changes, indexes, signs);
}
