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

#define NN_EG_NEURONS   8
#define NN_CACHE_SIZE   65536
#define NN_CACHE_MASK   65535

#define NN_RPvRP        0
#define NN_EG_COUNT     1

#define NN_RPvRP_FILE   "weights/RPvRP.net"

typedef struct NNCacheEntry {
    float neurons[NN_EG_NEURONS];
    uint64_t key;
} NNCacheEntry;

typedef NNCacheEntry NNCache[NN_CACHE_SIZE];

typedef struct EGNetwork {
    float **inputWeights;
    float inputBiases[NN_EG_NEURONS];
    float layer1Weights[NN_EG_NEURONS];
    float layer1Bias;
} EGNetwork;

void initEndgameNNs();
void initEndgameNN(EGNetwork *nn, char *weights[], int inputs);

int evaluateEndgames(Board *board);
void computeEndgameNeurons(EGNetwork *nn, NNCacheEntry *entry, Board *board);