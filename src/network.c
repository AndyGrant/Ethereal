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
#include "evaluate.h"
#include "network.h"
#include "thread.h"
#include "types.h"

PKNetwork PKNN;

static char *PKWeights[] = {
    #include "weights/pknet_224x32x2.net"
    ""
};

static int computePKNetworkIndex(int colour, int piece, int sq) {
    return (64 + 48) * colour
         + (48 * (piece == KING))
         + sq - 8 * (piece == PAWN);
}


void initPKNetwork() {

    for (int i = 0; i < PKNETWORK_LAYER1; i++) {

        char weights[strlen(PKWeights[i]) + 1];
        strcpy(weights, PKWeights[i]);
        strtok(weights, " ");

        for (int j = 0; j < PKNETWORK_INPUTS; j++)
            PKNN.inputWeights[j][i] = atof(strtok(NULL, " "));
        PKNN.inputBiases[i] = atof(strtok(NULL, " "));
    }

    for (int i = 0; i < PKNETWORK_OUTPUTS; i++) {

        char weights[strlen(PKWeights[i + PKNETWORK_LAYER1]) + 1];
        strcpy(weights, PKWeights[i + PKNETWORK_LAYER1]);
        strtok(weights, " ");

        for (int j = 0; j < PKNETWORK_LAYER1; j++)
            PKNN.layer1Weights[i][j] = atof(strtok(NULL, " "));
        PKNN.layer1Biases[i] = atof(strtok(NULL, " "));
    }
}

int computePKNetwork(Board *board) {

    uint64_t pawns = board->pieces[PAWN];
    uint64_t kings = board->pieces[KING];
    uint64_t black = board->colours[BLACK];

    float layer1Neurons[PKNETWORK_LAYER1];
    float outputNeurons[PKNETWORK_OUTPUTS];

    // Layer 1: Compute the values in the hidden Neurons of Layer 1
    // by looping over the Kings and Pawns bitboards, and applying
    // the weight which corresponds to each piece. We break the Kings
    // into two nearly duplicate steps, in order to more efficiently
    // set and update the Layer 1 Neurons initially

    { // Do one King first so we can set the Neurons
        int sq = poplsb(&kings);
        int idx = computePKNetworkIndex(testBit(black, sq), KING, sq);
        for (int i = 0; i < PKNETWORK_LAYER1; i++)
            layer1Neurons[i] = PKNN.inputBiases[i] + PKNN.inputWeights[idx][i];
    }

    { // Do the remaining King as we would do normally
        int sq = poplsb(&kings);
        int idx = computePKNetworkIndex(testBit(black, sq), KING, sq);
        for (int i = 0; i < PKNETWORK_LAYER1; i++)
            layer1Neurons[i] += PKNN.inputWeights[idx][i];
    }

    while (pawns) {
        int sq = poplsb(&pawns);
        int idx = computePKNetworkIndex(testBit(black, sq), PAWN, sq);
        for (int i = 0; i < PKNETWORK_LAYER1; i++)
            layer1Neurons[i] += PKNN.inputWeights[idx][i];
    }

    // Layer 2: Trivially compute the Output layer. Apply a ReLU here.
    // We do not apply a ReLU in Layer 1, since we already know that all
    // of the Inputs in Layer 1 are going to be zeros or ones

    for (int i = 0; i < PKNETWORK_OUTPUTS; i++) {
        outputNeurons[i] = PKNN.layer1Biases[i];
        for (int j = 0; j < PKNETWORK_LAYER1; j++)
            if (layer1Neurons[j] >= 0.0)
                outputNeurons[i] += layer1Neurons[j] * PKNN.layer1Weights[i][j];
    }

    assert(PKNETWORK_OUTPUTS == PHASE_NB);
    return MakeScore((int) outputNeurons[MG], (int) outputNeurons[EG]);
}
