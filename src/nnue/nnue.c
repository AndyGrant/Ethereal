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
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>    */
/*                                                                            */
/******************************************************************************/

#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>

#include "accumulator.h"
#include "nnue.h"
#include "types.h"
#include "utils.h"

#include "../bitboards.h"
#include "../board.h"
#include "../thread.h"

#include "../incbin/incbin.h"

#define SHIFT 6

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

    const vepi16 zero = vepi16_zero();

    vepi16 *in_white  = (vepi16 *) &accum->values[WHITE];
    vepi16 *in_black  = (vepi16 *) &accum->values[BLACK];

    vepi16 *out_white = (vepi16 *) (turn == WHITE ? outputs : &outputs[KPSIZE]);
    vepi16 *out_black = (vepi16 *) (turn == BLACK ? outputs : &outputs[KPSIZE]);

    for (int i = 0; i < KPSIZE / vepi16_cnt; i += 4) {
        out_white[i+0] = vepi16_max(zero, in_white[i+0]);
        out_white[i+1] = vepi16_max(zero, in_white[i+1]);
        out_white[i+2] = vepi16_max(zero, in_white[i+2]);
        out_white[i+3] = vepi16_max(zero, in_white[i+3]);
    }

    for (int i = 0; i < KPSIZE / vepi16_cnt; i += 4) {
        out_black[i+0] = vepi16_max(zero, in_black[i+0]);
        out_black[i+1] = vepi16_max(zero, in_black[i+1]);
        out_black[i+2] = vepi16_max(zero, in_black[i+2]);
        out_black[i+3] = vepi16_max(zero, in_black[i+3]);
    }
}

INLINE void quant_affine_relu(int16_t *weights, int32_t *biases, int16_t *inputs, float *outputs) {

    assert(L1SIZE % 16 == 0 && L2SIZE % 8 == 0);

    const int InChunks  = L1SIZE / vepi16_cnt;
    const int OutChunks = L2SIZE / 8;

    #if defined(USE_AVX2) || defined(USE_AVX)
    const vepi32 zero = vepi32_zero();
    #elif defined(USE_SSSE3)
    const vps32  zero = vps32_zero();
    #endif

    const vepi16 *inp = (vepi16*) inputs;
    const vepi32 *bia = (vepi32*) biases;
    const vepi16 *wgt = (vepi16*) weights;

    vps32 *out  = (vps32*) outputs;

    for (int i = 0; i < OutChunks; i++) {

        vepi32 acc0 = vepi16_madd(wgt[InChunks * (i * 8 + 0) + 0], inp[0]);
        vepi32 acc1 = vepi16_madd(wgt[InChunks * (i * 8 + 1) + 0], inp[0]);
        vepi32 acc2 = vepi16_madd(wgt[InChunks * (i * 8 + 2) + 0], inp[0]);
        vepi32 acc3 = vepi16_madd(wgt[InChunks * (i * 8 + 3) + 0], inp[0]);

        for (int j = 1; j < InChunks; j++) {
            acc0 = vepi32_add(acc0, vepi16_madd(wgt[InChunks * (i * 8 + 0) + j], inp[j]));
            acc1 = vepi32_add(acc1, vepi16_madd(wgt[InChunks * (i * 8 + 1) + j], inp[j]));
            acc2 = vepi32_add(acc2, vepi16_madd(wgt[InChunks * (i * 8 + 2) + j], inp[j]));
            acc3 = vepi32_add(acc3, vepi16_madd(wgt[InChunks * (i * 8 + 3) + j], inp[j]));
        }

        acc0 = vepi32_hadd(acc0, acc1);
        acc2 = vepi32_hadd(acc2, acc3);
        acc0 = vepi32_hadd(acc0, acc2);

        vepi32 acc4 = vepi16_madd(wgt[InChunks * (i * 8 + 4) + 0], inp[0]);
        vepi32 acc5 = vepi16_madd(wgt[InChunks * (i * 8 + 5) + 0], inp[0]);
        vepi32 acc6 = vepi16_madd(wgt[InChunks * (i * 8 + 6) + 0], inp[0]);
        vepi32 acc7 = vepi16_madd(wgt[InChunks * (i * 8 + 7) + 0], inp[0]);

        for (int j = 1; j < InChunks; j++) {
            acc4 = vepi32_add(acc4, vepi16_madd(wgt[InChunks * (i * 8 + 4) + j], inp[j]));
            acc5 = vepi32_add(acc5, vepi16_madd(wgt[InChunks * (i * 8 + 5) + j], inp[j]));
            acc6 = vepi32_add(acc6, vepi16_madd(wgt[InChunks * (i * 8 + 6) + j], inp[j]));
            acc7 = vepi32_add(acc7, vepi16_madd(wgt[InChunks * (i * 8 + 7) + j], inp[j]));
        }

        acc4 = vepi32_hadd(acc4, acc5);
        acc6 = vepi32_hadd(acc6, acc7);
        acc4 = vepi32_hadd(acc4, acc6);

        #if defined(USE_AVX2)

        __m128i sumabcd1 = _mm256_extracti128_si256(acc0, 0);
        __m128i sumabcd2 = _mm256_extracti128_si256(acc0, 1);
        __m128i sumefgh1 = _mm256_extracti128_si256(acc4, 0);
        __m128i sumefgh2 = _mm256_extracti128_si256(acc4, 1);

        sumabcd1 = _mm_add_epi32(sumabcd1, sumabcd2);
        sumefgh1 = _mm_add_epi32(sumefgh1, sumefgh2);

        acc0 = _mm256_inserti128_si256(_mm256_castsi128_si256(sumabcd1), sumefgh1, 1);
        out[i] = _mm256_cvtepi32_ps(vepi32_max(zero, vepi32_add(bia[i], acc0)));

        #elif defined(USE_AVX)

        __m128 ps0 = _mm_cvtepi32_ps(vepi32_max(zero, vepi32_add(bia[i * 2 + 0], acc0)));
        __m128 ps1 = _mm_cvtepi32_ps(vepi32_max(zero, vepi32_add(bia[i * 2 + 1], acc4)));

        out[i] = _mm256_insertf128_ps(out[i], ps0, 0);
        out[i] = _mm256_insertf128_ps(out[i], ps1, 1);

        #elif defined (USE_SSSE3)

        out[i * 2 + 0] = vps32_max(zero, _mm_cvtepi32_ps(vepi32_add(bia[i * 2 + 0], acc0)));
        out[i * 2 + 1] = vps32_max(zero, _mm_cvtepi32_ps(vepi32_add(bia[i * 2 + 1], acc4)));

        #endif
    }
}

INLINE void float_affine_relu(float *weights, float *biases, float *inputs, float *outputs) {

    assert(L2SIZE % 8 == 0 && L3SIZE % 8 == 0);

    const int InChunks  = L2SIZE / vps32_cnt;
    const int OutChunks = L3SIZE / 8;

    const vps32 zero = vps32_zero();

    const vps32 *inp = (vps32 *) inputs;
    const vps32 *bia = (vps32 *) biases;
    const vps32 *wgt = (vps32 *) weights;
    vps32 *const out = (vps32 *) outputs;

    for (int i = 0; i < OutChunks; i++) {

        vps32 acc0 = vps32_mul(wgt[InChunks * (i * 8 + 0) + 0], inp[0]);
        vps32 acc1 = vps32_mul(wgt[InChunks * (i * 8 + 1) + 0], inp[0]);
        vps32 acc2 = vps32_mul(wgt[InChunks * (i * 8 + 2) + 0], inp[0]);
        vps32 acc3 = vps32_mul(wgt[InChunks * (i * 8 + 3) + 0], inp[0]);
        vps32 acc4 = vps32_mul(wgt[InChunks * (i * 8 + 4) + 0], inp[0]);
        vps32 acc5 = vps32_mul(wgt[InChunks * (i * 8 + 5) + 0], inp[0]);
        vps32 acc6 = vps32_mul(wgt[InChunks * (i * 8 + 6) + 0], inp[0]);
        vps32 acc7 = vps32_mul(wgt[InChunks * (i * 8 + 7) + 0], inp[0]);

        for (int j = 1; j < InChunks; j++) {
            acc0 = vps32_fma(wgt[InChunks * (i * 8 + 0) + j], inp[j], acc0);
            acc1 = vps32_fma(wgt[InChunks * (i * 8 + 1) + j], inp[j], acc1);
            acc2 = vps32_fma(wgt[InChunks * (i * 8 + 2) + j], inp[j], acc2);
            acc3 = vps32_fma(wgt[InChunks * (i * 8 + 3) + j], inp[j], acc3);
            acc4 = vps32_fma(wgt[InChunks * (i * 8 + 4) + j], inp[j], acc4);
            acc5 = vps32_fma(wgt[InChunks * (i * 8 + 5) + j], inp[j], acc5);
            acc6 = vps32_fma(wgt[InChunks * (i * 8 + 6) + j], inp[j], acc6);
            acc7 = vps32_fma(wgt[InChunks * (i * 8 + 7) + j], inp[j], acc7);
        }

        acc0 = vps32_hadd(acc0, acc1);
        acc2 = vps32_hadd(acc2, acc3);
        acc4 = vps32_hadd(acc4, acc5);
        acc6 = vps32_hadd(acc6, acc7);

        acc0 = vps32_hadd(acc0, acc2);
        acc4 = vps32_hadd(acc4, acc6);

        #if defined(USE_AVX2) || defined(USE_AVX)

        __m128 sumabcd1 = _mm256_extractf128_ps(acc0, 0);
        __m128 sumabcd2 = _mm256_extractf128_ps(acc0, 1);
        __m128 sumefgh1 = _mm256_extractf128_ps(acc4, 0);
        __m128 sumefgh2 = _mm256_extractf128_ps(acc4, 1);

        sumabcd1 = _mm_add_ps(sumabcd1, sumabcd2);
        sumefgh1 = _mm_add_ps(sumefgh1, sumefgh2);

        acc0 = _mm256_insertf128_ps(_mm256_castps128_ps256(sumabcd1), sumefgh1, 1);
        out[i] = _mm256_max_ps(zero, _mm256_add_ps(bia[i], acc0));

        #elif defined(USE_SSSE3)

        out[i * 2 + 0] = vps32_max(zero, vps32_add(bia[i * 2 + 0], acc0));
        out[i * 2 + 1] = vps32_max(zero, vps32_add(bia[i * 2 + 1], acc4));

        #endif
    }
}

INLINE void output_transform(float *weights, float *biases, float *inputs, float *outputs) {

    assert(L3SIZE % 8 == 0);

    const int InChunks = L3SIZE / vps32_cnt;

    const vps32 *inp  = (vps32 *) inputs;
    const vps32 *wgt  = (vps32 *) weights;

    vps32 acc = vps32_mul(wgt[0], inp[0]);
    for (int i = 1; i < InChunks; i++)
        acc = vps32_fma(wgt[i], inp[i], acc);

    #if defined(USE_AVX) || defined(USE_AVX2)

    const __m128 hiQuad  = _mm256_extractf128_ps(acc, 1);
    const __m128 loQuad  = _mm256_castps256_ps128(acc);
    const __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);

    #elif defined(USE_SSSE3)

    const __m128 sumQuad = acc;

    #endif

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

    int eval;
    const uint64_t white = board->colours[WHITE];
    const uint64_t black = board->colours[BLACK];
    const uint64_t kings = board->pieces[KING];

    // For optimizations, auto-flag KvK as drawn
    if (kings == (white | black)) return 0;

    // Optimized computation of various input indices
    int wkingidx = 640 * relativeSquare(WHITE, getlsb(white & kings));
    int bkingidx = 640 * relativeSquare(BLACK, getlsb(black & kings));

    // Large enough to handle layer computations
    ALIGN64 int16_t out16[L1SIZE];
    ALIGN64 float outN1[L1SIZE];
    ALIGN64 float outN2[L1SIZE];

    NNUEAccumulator *accum = &thread->nnueStack[thread->height];

    if (!accum->accurate) {

        // Possible to recurse and incrementally update each
        if (nnue_can_update(accum, board))
            nnue_update_accumulator(accum, board, wkingidx, bkingidx);

        // History is missing, we must refresh completely
        else
            nnue_refresh_accumulators(accum, board, wkingidx, bkingidx);
    }

    // Feed-forward the entire evaluation function
    halfkp_relu(accum, out16, board->turn);
    quant_affine_relu(l1_weights, l1_biases, out16, outN1);
    float_affine_relu(l2_weights, l2_biases, outN1, outN2);
    output_transform(l3_weights, l3_biases, outN2, outN1);

    // Perform the dequantization step and multiply by 1.10
    eval = 110 * ((int)(outN1[0]) >> (2 * SHIFT)) / 100;
    return MAX(-1000, MIN(1000, eval));
}
