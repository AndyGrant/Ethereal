/******************************************************************************/
/*                                                                            */
/*    Ethereal is a UCI chess playing engine authored by Andrew Grant.        */
/*    <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>        */
/*                                                                            */
/*    Ethereal is free software: you can redistribute it and/or modify        */
/*    it under the terms of the GNU General Public License as published by    */
/*    the Free Software Foundation, either version 3 of the License, or       */
/*    (at your option) any later version.                                     */
/*                                                                            */
/*    Ethereal is distributed in the hope that it will be useful,             */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*    GNU General Public License for more details.                            */
/*                                                                            */
/*    You should have received a copy of the GNU General Public License       */
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>.   */
/*                                                                            */
/******************************************************************************/

#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "accumulator.h"
#include "nnue.h"
#include "types.h"
#include "utils.h"

#include "../bitboards.h"
#include "../board.h"
#include "../thread.h"

#include "../incbin/incbin.h"

#ifdef EVALFILE
const char *NNUEDefault = EVALFILE;
INCBIN(IncWeights, EVALFILE);
#endif

ALIGN64 int16_t in_weights[INSIZE * KPSIZE ];
ALIGN64 int16_t l1_weights[L1SIZE * L2SIZE ];
ALIGN64 float   l2_weights[L2SIZE * L3SIZE ];
ALIGN64 float   l3_weights[L3SIZE * OUTSIZE];

ALIGN64 int16_t in_biases[KPSIZE ];
ALIGN64 int32_t l1_biases[L2SIZE ];
ALIGN64 float   l2_biases[L3SIZE ];
ALIGN64 float   l3_biases[OUTSIZE];


static void scale_weights() {

    // Delayed dequantization forces an upshift of biases in later layers,
    // as the number of delays grows. This nets large speed gains, as well
    // as precision gains, for the slight risk of under flows or over flows.

    for (int i = 0; i < L2SIZE; i++)
        l1_biases[i] *= (1 << SHIFT);

    for (int i = 0; i < L3SIZE; i++)
        l2_biases[i] *= (1 << (2 * SHIFT));

    for (int i = 0; i < OUTSIZE; i++)
        l3_biases[i] *= (1 << (2 * SHIFT));
}

static void quant_transpose(int16_t *matrix, int rows, int cols) {

    // Typical Matrix Transposition using int16_t. Ethereal's trainer
    // stores weights in a way to allow faster updates, not computes

    int16_t *cpy = malloc(sizeof(int16_t) * rows * cols);

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            cpy[j * rows + i] = matrix[i * cols + j];

    memcpy(matrix, cpy, sizeof(int16_t) * rows * cols);
    free(cpy);
}

static void float_transpose(float *matrix, int rows, int cols) {

    // Typical Matrix Transposition using float_t. Ethereal's trainer
    // stores weights in a way to allow faster updates, not computes

    float *cpy = malloc(sizeof(float) * rows * cols);

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            cpy[j * rows + i] = matrix[i * cols + j];

    memcpy(matrix, cpy, sizeof(float) * rows * cols);
    free(cpy);
}


INLINE void halfkp_relu(NNUEAccumulator *accum, int16_t *outputs, int turn) {

    // The accumulation of king-piece values has already been computed.
    // Perform the ReLU operation on each accumuatlor, and place them
    // such that the side-to-move is first, then the non-side-to-move

    assert(KPSIZE % 64 == 0);

    const __m256i zero = _mm256_setzero_si256();

    __m256i *in_white  = (__m256i *) &accum->values[WHITE];
    __m256i *in_black  = (__m256i *) &accum->values[BLACK];

    __m256i *out_white = (__m256i *) (turn == WHITE ? outputs : &outputs[KPSIZE]);
    __m256i *out_black = (__m256i *) (turn == BLACK ? outputs : &outputs[KPSIZE]);

    for (int i = 0; i < KPSIZE / 16; i += 4) {
        out_white[i+0] = _mm256_max_epi16(zero, in_white[i+0]);
        out_white[i+1] = _mm256_max_epi16(zero, in_white[i+1]);
        out_white[i+2] = _mm256_max_epi16(zero, in_white[i+2]);
        out_white[i+3] = _mm256_max_epi16(zero, in_white[i+3]);
    }

    for (int i = 0; i < KPSIZE / 16; i += 4) {
        out_black[i+0] = _mm256_max_epi16(zero, in_black[i+0]);
        out_black[i+1] = _mm256_max_epi16(zero, in_black[i+1]);
        out_black[i+2] = _mm256_max_epi16(zero, in_black[i+2]);
        out_black[i+3] = _mm256_max_epi16(zero, in_black[i+3]);
    }
}

INLINE void quant_affine_relu(int16_t *weights, int32_t *biases, int16_t *inputs, float *outputs) {

    assert(L1SIZE % 16 == 0 && L2SIZE % 8 == 0);

    const int InChunks  = L1SIZE / 16;
    const int OutChunks = L2SIZE / 8;

    const __m256i zero = _mm256_setzero_si256();

    const __m256i *inp = (__m256i *) inputs;
    const __m256i *bia = (__m256i *) biases;
    const __m256i *wgt = (__m256i *) weights;

    __m256 *out  = (__m256*) outputs;

    for (int i = 0; i < OutChunks; i++) {

        __m256i acc0 = _mm256_madd_epi16(wgt[InChunks * (i * 8 + 0) + 0], inp[0]);
        __m256i acc1 = _mm256_madd_epi16(wgt[InChunks * (i * 8 + 1) + 0], inp[0]);
        __m256i acc2 = _mm256_madd_epi16(wgt[InChunks * (i * 8 + 2) + 0], inp[0]);
        __m256i acc3 = _mm256_madd_epi16(wgt[InChunks * (i * 8 + 3) + 0], inp[0]);

        for (int j = 1; j < InChunks; j++) {
            acc0 = _mm256_add_epi32(acc0, _mm256_madd_epi16(wgt[InChunks * (i * 8 + 0) + j], inp[j]));
            acc1 = _mm256_add_epi32(acc1, _mm256_madd_epi16(wgt[InChunks * (i * 8 + 1) + j], inp[j]));
            acc2 = _mm256_add_epi32(acc2, _mm256_madd_epi16(wgt[InChunks * (i * 8 + 2) + j], inp[j]));
            acc3 = _mm256_add_epi32(acc3, _mm256_madd_epi16(wgt[InChunks * (i * 8 + 3) + j], inp[j]));
        }

        __m256i acc4 = _mm256_madd_epi16(wgt[InChunks * (i * 8 + 4) + 0], inp[0]);
        __m256i acc5 = _mm256_madd_epi16(wgt[InChunks * (i * 8 + 5) + 0], inp[0]);
        __m256i acc6 = _mm256_madd_epi16(wgt[InChunks * (i * 8 + 6) + 0], inp[0]);
        __m256i acc7 = _mm256_madd_epi16(wgt[InChunks * (i * 8 + 7) + 0], inp[0]);

        for (int j = 1; j < InChunks; j++) {
            acc4 = _mm256_add_epi32(acc4, _mm256_madd_epi16(wgt[InChunks * (i * 8 + 4) + j], inp[j]));
            acc5 = _mm256_add_epi32(acc5, _mm256_madd_epi16(wgt[InChunks * (i * 8 + 5) + j], inp[j]));
            acc6 = _mm256_add_epi32(acc6, _mm256_madd_epi16(wgt[InChunks * (i * 8 + 6) + j], inp[j]));
            acc7 = _mm256_add_epi32(acc7, _mm256_madd_epi16(wgt[InChunks * (i * 8 + 7) + j], inp[j]));
        }

        acc0 = _mm256_hadd_epi32(acc0, acc1);
        acc2 = _mm256_hadd_epi32(acc2, acc3);
        acc4 = _mm256_hadd_epi32(acc4, acc5);
        acc6 = _mm256_hadd_epi32(acc6, acc7);

        acc0 = _mm256_hadd_epi32(acc0, acc2);
        acc4 = _mm256_hadd_epi32(acc4, acc6);

        __m128i sumabcd1 = _mm256_extracti128_si256(acc0, 0);
        __m128i sumabcd2 = _mm256_extracti128_si256(acc0, 1);
        __m128i sumefgh1 = _mm256_extracti128_si256(acc4, 0);
        __m128i sumefgh2 = _mm256_extracti128_si256(acc4, 1);

        sumabcd1 = _mm_add_epi32(sumabcd1, sumabcd2);
        sumefgh1 = _mm_add_epi32(sumefgh1, sumefgh2);

        acc0 = _mm256_inserti128_si256(_mm256_castsi128_si256(sumabcd1), sumefgh1, 1);

        const __m256i biased  = _mm256_add_epi32(bia[i], acc0);
        const __m256i relu    = _mm256_max_epi32(zero, biased);

        out[i] = _mm256_cvtepi32_ps(relu);
    }
}

INLINE void float_affine_relu(float *weights, float *biases, float *inputs, float *outputs) {

    assert(L2SIZE % 8 == 0 && L3SIZE % 8 == 0);

    const int InChunks  = L2SIZE / 8;
    const int OutChunks = L3SIZE / 8;

    const __m256 zero = _mm256_setzero_ps();

    const __m256 *inp = (__m256 *) inputs;
    const __m256 *bia = (__m256 *) biases;
    const __m256 *wgt = (__m256 *) weights;
    __m256 *const out = (__m256 *) outputs;

    for (int i = 0; i < OutChunks; i++) {

        __m256 acc0 = _mm256_mul_ps(wgt[InChunks * (i * 8 + 0) + 0], inp[0]);
        __m256 acc1 = _mm256_mul_ps(wgt[InChunks * (i * 8 + 1) + 0], inp[0]);
        __m256 acc2 = _mm256_mul_ps(wgt[InChunks * (i * 8 + 2) + 0], inp[0]);
        __m256 acc3 = _mm256_mul_ps(wgt[InChunks * (i * 8 + 3) + 0], inp[0]);
        __m256 acc4 = _mm256_mul_ps(wgt[InChunks * (i * 8 + 4) + 0], inp[0]);
        __m256 acc5 = _mm256_mul_ps(wgt[InChunks * (i * 8 + 5) + 0], inp[0]);
        __m256 acc6 = _mm256_mul_ps(wgt[InChunks * (i * 8 + 6) + 0], inp[0]);
        __m256 acc7 = _mm256_mul_ps(wgt[InChunks * (i * 8 + 7) + 0], inp[0]);

        for (int j = 1; j < InChunks; j++) {
            acc0 = _mm256_fmadd_ps(wgt[InChunks * (i * 8 + 0) + j], inp[j], acc0);
            acc1 = _mm256_fmadd_ps(wgt[InChunks * (i * 8 + 1) + j], inp[j], acc1);
            acc2 = _mm256_fmadd_ps(wgt[InChunks * (i * 8 + 2) + j], inp[j], acc2);
            acc3 = _mm256_fmadd_ps(wgt[InChunks * (i * 8 + 3) + j], inp[j], acc3);
            acc4 = _mm256_fmadd_ps(wgt[InChunks * (i * 8 + 4) + j], inp[j], acc4);
            acc5 = _mm256_fmadd_ps(wgt[InChunks * (i * 8 + 5) + j], inp[j], acc5);
            acc6 = _mm256_fmadd_ps(wgt[InChunks * (i * 8 + 6) + j], inp[j], acc6);
            acc7 = _mm256_fmadd_ps(wgt[InChunks * (i * 8 + 7) + j], inp[j], acc7);
        }

        acc0 = _mm256_hadd_ps(acc0, acc1);
        acc2 = _mm256_hadd_ps(acc2, acc3);
        acc4 = _mm256_hadd_ps(acc4, acc5);
        acc6 = _mm256_hadd_ps(acc6, acc7);

        acc0 = _mm256_hadd_ps(acc0, acc2);
        acc4 = _mm256_hadd_ps(acc4, acc6);

        __m128 sumabcd1 = _mm256_extractf128_ps(acc0, 0);
        __m128 sumabcd2 = _mm256_extractf128_ps(acc0, 1);
        __m128 sumefgh1 = _mm256_extractf128_ps(acc4, 0);
        __m128 sumefgh2 = _mm256_extractf128_ps(acc4, 1);

        sumabcd1 = _mm_add_ps(sumabcd1, sumabcd2);
        sumefgh1 = _mm_add_ps(sumefgh1, sumefgh2);

        acc0 = _mm256_insertf128_ps(_mm256_castps128_ps256(sumabcd1), sumefgh1, 1);
        out[i] = _mm256_max_ps(zero, _mm256_add_ps(bia[i], acc0));
    }
}

INLINE void output_transform(float *weights, float *biases, float *inputs, float *outputs) {

    assert(L3SIZE % 8 == 0);

    const int InChunks = L3SIZE / 8;

    const __m256 *inp  = (__m256 *) inputs;
    const __m256 *wgt  = (__m256 *) weights;

    __m256 acc = _mm256_mul_ps(wgt[0], inp[0]);
    for (int i = 1; i < InChunks; i++)
        acc = _mm256_fmadd_ps(wgt[i], inp[i], acc);

    const __m128 hiQuad  = _mm256_extractf128_ps(acc, 1);
    const __m128 loQuad  = _mm256_castps256_ps128(acc);
    const __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);

    const __m128 hiDual  = _mm_movehl_ps(sumQuad, sumQuad);
    const __m128 sumDual = _mm_add_ps(sumQuad, hiDual);

    const __m128 hi      = _mm_shuffle_ps(sumDual, sumDual, 0x1);
    const __m128 sum     = _mm_add_ss(sumDual, hi);

    *outputs = (_mm_cvtss_f32(sum) + *biases);
}


void nnue_init(const char* fname) {

    // Reads an NNUE file specificed by a User. If the datasize does not match
    // the compiled NNUE configuration, abort. Afterwords, scale some weights
    // for speed optimizations, and transpose the weights in L1 and L2

    FILE *fin = fopen(fname, "rb");

    if (   fread(in_biases, sizeof(int16_t), KPSIZE, fin) != (size_t) KPSIZE
        || fread(in_weights, sizeof(int16_t), INSIZE * KPSIZE, fin) != (size_t) INSIZE * KPSIZE)
        printf("info string Unable to read NNUE file\n"), exit(EXIT_FAILURE);

    if (   fread(l1_biases, sizeof(int32_t), L2SIZE, fin) != (size_t) L2SIZE
        || fread(l1_weights, sizeof(int16_t), L1SIZE * L2SIZE, fin) != (size_t) L1SIZE * L2SIZE)
        printf("info string Unable to read NNUE file\n"), exit(EXIT_FAILURE);

    if (   fread(l2_biases, sizeof(float), L3SIZE, fin) != (size_t) L3SIZE
        || fread(l2_weights, sizeof(float), L2SIZE * L3SIZE, fin) != (size_t) L2SIZE * L3SIZE)
        printf("info string Unable to read NNUE file\n"), exit(EXIT_FAILURE);

    if (   fread(l3_biases, sizeof(float), OUTSIZE, fin) != (size_t) OUTSIZE
        || fread(l3_weights, sizeof(float), L3SIZE * OUTSIZE, fin) != (size_t) L3SIZE * OUTSIZE)
        printf("info string Unable to read NNUE file\n"), exit(EXIT_FAILURE);

    scale_weights();
    quant_transpose(l1_weights, L1SIZE, L2SIZE);
    float_transpose(l2_weights, L2SIZE, L3SIZE);
    fclose(fin);
}

void nnue_incbin_init() {

    // Inits from an NNUE file compiled into the binary. Assume the compiled
    // data is correct for the given NNUE config. Afterwords, scale some
    // weights for speed optimizations, and transpose the weights in L1 and L2

    #ifdef EVALFILE

    int16_t *data16; int32_t *data32; float *dataf;

    data16 = (int16_t*) gIncWeightsData;
    for (int i = 0; i < KPSIZE; i++)
        in_biases[i] = *(data16++);

    for (int i = 0; i < INSIZE * KPSIZE; i++)
        in_weights[i] = *(data16++);

    data32 = (int32_t*) data16;
    for (int i = 0; i < L2SIZE; i++)
        l1_biases[i] = *(data32++);

    data16 = (int16_t*) data32;
    for (int i = 0; i < L1SIZE * L2SIZE; i++)
        l1_weights[i] = *(data16++);

    dataf = (float*) data16;
    for (int i = 0; i < L3SIZE; i++)
        l2_biases[i] = *(dataf++);

    for (int i = 0; i < L2SIZE * L3SIZE; i++)
        l2_weights[i] = *(dataf++);

    for (int i = 0; i < OUTSIZE; i++)
        l3_biases[i] = *(dataf++);

    for (int i = 0; i < L3SIZE * OUTSIZE; i++)
        l3_weights[i] = *(dataf++);

    scale_weights();
    quant_transpose(l1_weights, L1SIZE, L2SIZE);
    float_transpose(l2_weights, L2SIZE, L3SIZE);

    #endif
}

int nnue_evaluate(Thread *thread, Board *board) {

    const uint64_t white = board->colours[WHITE];
    const uint64_t black = board->colours[BLACK];
    const uint64_t kings = board->pieces[KING];

    board->ksquares[WHITE] = getlsb(white & kings);
    board->ksquares[BLACK] = getlsb(black & kings);

    // For optimizations, auto-flag KvK as drawn
    if (kings == (white | black)) return 0;

    // Large enough to handle layer computations
    ALIGN64 int16_t out16[L1SIZE];
    ALIGN64 float outN1[L1SIZE], outN2[L1SIZE];

    NNUEAccumulator *accum = &thread->nnueStack[thread->height];

    if (!accum->accurate) {

        // Possible to recurse and incrementally update each
        if (nnue_can_update(accum, board))
            nnue_update_accumulator(accum, board);

        // History is missing, we must refresh completely
        else
            nnue_refresh_accumulators(accum, board);
    }

    // Feed-forward the entire evaluation function
    halfkp_relu(accum, out16, board->turn);
    quant_affine_relu(l1_weights, l1_biases, out16, outN1);
    float_affine_relu(l2_weights, l2_biases, outN1, outN2);
    output_transform(l3_weights, l3_biases, outN2, outN1);

    // Finally perform the dequantization step
    return (int)(outN1[0]) >> (2 * SHIFT);
}
