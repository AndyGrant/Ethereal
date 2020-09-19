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

#include <stdint.h>

#include "board.h"
#include "types.h"

#define PKNETWORK_INPUTS  (224)
#define PKNETWORK_LAYER1  ( 32)
#define PKNETWORK_OUTPUTS (  1)

typedef struct PKNetwork {

    // PKNetworks are of the form [Input, Hidden Layer 1, Output Layer]
    // Our current Network is [224x32, 32x1]. The Network is trained to
    // output a Score in CentiPawns, and thus Output Neurons need do not
    // need activation functions. The 32x1 operation applys a ReLU, where
    // as the 224x32 layer does not, since inputs are binary 0s and 1s

    ALIGN64 float inputWeights[PKNETWORK_LAYER1][PKNETWORK_INPUTS];
    ALIGN64 float inputBiases[PKNETWORK_LAYER1];

    ALIGN64 float layer1Weights[PKNETWORK_OUTPUTS][PKNETWORK_LAYER1];
    ALIGN64 float layer1Biases[PKNETWORK_OUTPUTS];

} PKNetwork;

void initPKNetwork();
int fullyComputePKNetwork(Thread *thread);
int partiallyComputePKNetwork(Thread *thread);

void initPKNetworkCollector(Thread *thread);
void updatePKNetworkIndices(Thread *thread, int changes, int indexes[3], int signs[3]);
void updatePKNetworkAfterMove(Thread *thread, uint16_t move);