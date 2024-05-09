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
#include "../evaluate.h"
#include "../thread.h"

#include "../incbin/incbin.h"

#define SHIFT_L0 6
#define SHIFT_L1 5

#ifdef EVALFILE
const char *NNUEDefault = EVALFILE;
INCBIN(IncWeights, EVALFILE);
#endif

ALIGN64 int16_t in_weights[INSIZE * KPSIZE ];
ALIGN64 int8_t  l1_weights[L1SIZE * L2SIZE ];
ALIGN64 float   l2_weights[L2SIZE * L3SIZE ];
ALIGN64 float   l3_weights[L3SIZE * OUTSIZE];

ALIGN64 int16_t in_biases[KPSIZE ];
ALIGN64 int32_t l1_biases[L2SIZE ];
ALIGN64 float   l2_biases[L3SIZE ];
ALIGN64 float   l3_biases[OUTSIZE];

static int NNUE_LOADED = 0;

static void scale_weights() {

    // Delayed dequantization of the results of L1 forces an upshift in
    // biases of L2 and L3 to compensate. This saves SRAI calls, as well as
    // increases the precision of each layer, with no clear downsides.

    for (int i = 0; i < L3SIZE; i++)
        l2_biases[i] *= (1 << SHIFT_L1);

    for (int i = 0; i < OUTSIZE; i++)
        l3_biases[i] *= (1 << SHIFT_L1);
}

static void quant_transpose(int8_t *matrix, int rows, int cols) {

    // Typical Matrix Transposition using int8_t. Ethereal's trainer
    // stores weights in a way to allow faster updates, not computes

    int8_t *cpy = malloc(sizeof(int8_t) * rows * cols);

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            cpy[j * rows + i] = matrix[i * cols + j];

    memcpy(matrix, cpy, sizeof(int8_t) * rows * cols);
    free(cpy);
}

static void float_transpose(float *matrix, int rows, int cols) {

    // Typical Matrix Transposition using floats. Ethereal's trainer
    // stores weights in a way to allow faster updates, not computes

    float *cpy = malloc(sizeof(float) * rows * cols);

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            cpy[j * rows + i] = matrix[i * cols + j];

    memcpy(matrix, cpy, sizeof(float) * rows * cols);
    free(cpy);
}

static void shuffle_input_layer() {

    #if defined(USE_AVX2)

    __m256i *wgt = (__m256i *) in_weights;
    __m256i *bia = (__m256i *) in_biases;

    // Interleave adjacent 256-bit chunks of 2-byte values. During
    // halfkp_relu() adjacent chunks are split, with the A-half of
    // chunk 1 swapping with A-half of chunk 2. This is done to both
    // the weights and the biases, to avoid unshuffling them later.

    for (int i = 0; i < KPSIZE / vepi16_cnt; i += 2) {

        __m128i half1 = _mm256_extracti128_si256(bia[i+0], 1);
        __m128i half2 = _mm256_extracti128_si256(bia[i+1], 0);

        bia[i+0] = _mm256_inserti128_si256(bia[i+0], half2, 1);
        bia[i+1] = _mm256_inserti128_si256(bia[i+1], half1, 0);
    }

    for (int i = 0; i < INSIZE * KPSIZE / vepi16_cnt; i += 2) {

        __m128i half1 = _mm256_extracti128_si256(wgt[i+0], 1);
        __m128i half2 = _mm256_extracti128_si256(wgt[i+1], 0);

        wgt[i+0] = _mm256_inserti128_si256(wgt[i+0], half2, 1);
        wgt[i+1] = _mm256_inserti128_si256(wgt[i+1], half1, 0);
    }

    #endif
}

static void abort_nnue(const char *reason) {
    printf("info string %s\n", reason);
    fflush(stdout); exit(EXIT_FAILURE);
}

INLINE vepi8 vepi16_relu_packu(vepi16 in0, vepi16 in1) {
    vepi16 shiftA = vepi16_srai(in0, SHIFT_L0);
    vepi16 shiftB = vepi16_srai(in1, SHIFT_L0);
    return vepi16_packu(shiftA, shiftB);
}

INLINE void relu_maddubs_x4(vepi32 *acc, const vepi16 *inp, const vepi8 *wgt, int i, int j, int k) {

    static const int InChunks = L1SIZE / vepi8_cnt;

    vepi16 sum0 = vepi16_maubs(vepi16_relu_packu(inp[0], inp[1]), wgt[InChunks * (i * 8 + k) + j + 0]);
    vepi16 sum1 = vepi16_maubs(vepi16_relu_packu(inp[2], inp[3]), wgt[InChunks * (i * 8 + k) + j + 1]);
    vepi16 sum2 = vepi16_maubs(vepi16_relu_packu(inp[4], inp[5]), wgt[InChunks * (i * 8 + k) + j + 2]);
    vepi16 sum3 = vepi16_maubs(vepi16_relu_packu(inp[6], inp[7]), wgt[InChunks * (i * 8 + k) + j + 3]);

    vepi16 sumX = vepi16_add(sum0, vepi16_add(sum1, vepi16_add(sum2, sum3)));
    *acc = vepi32_add(*acc, vepi16_madd(vepi16_one, sumX));
}

INLINE void halfkp_relu_quant_affine_relu(int8_t *weights, int32_t *biases, int16_t *us_accum, int16_t *opp_accum, float *outputs) {

    assert(L1SIZE % 64 == 0 && L2SIZE % 8 == 0);
    assert(L1SIZE == KPSIZE * 2);

    const int InChunks  = KPSIZE / vepi8_cnt;
    const int OutChunks = L2SIZE / 8;

    #if defined(USE_AVX2) || defined(USE_AVX)
    const vepi32 zero = vepi32_zero();
    #elif defined(USE_SSSE3)
    const vps32  zero = vps32_zero();
    #endif

    const vepi8  *us  = (vepi8  *) us_accum;
    const vepi8  *opp = (vepi8  *) opp_accum;
    const vepi8  *wgt = (vepi8  *) weights;
    const vepi32 *bia = (vepi32 *) biases;
    vps32 *const out  = (vps32  *) outputs;

    for (int i = 0; i < OutChunks; i++) {

        vepi32 acc0 = vepi32_zero();
        vepi32 acc1 = vepi32_zero();
        vepi32 acc2 = vepi32_zero();
        vepi32 acc3 = vepi32_zero();
        vepi32 acc4 = vepi32_zero();
        vepi32 acc5 = vepi32_zero();
        vepi32 acc6 = vepi32_zero();
        vepi32 acc7 = vepi32_zero();

        for (int j = 0; j < InChunks; j += 4) {
            relu_maddubs_x4(&acc0, &us [j * 2], wgt, i, j, 0);
            relu_maddubs_x4(&acc1, &us [j * 2], wgt, i, j, 1);
            relu_maddubs_x4(&acc2, &us [j * 2], wgt, i, j, 2);
            relu_maddubs_x4(&acc3, &us [j * 2], wgt, i, j, 3);
            relu_maddubs_x4(&acc4, &us [j * 2], wgt, i, j, 4);
            relu_maddubs_x4(&acc5, &us [j * 2], wgt, i, j, 5);
            relu_maddubs_x4(&acc6, &us [j * 2], wgt, i, j, 6);
            relu_maddubs_x4(&acc7, &us [j * 2], wgt, i, j, 7);

            relu_maddubs_x4(&acc0, &opp[j * 2], wgt + InChunks, i, j, 0);
            relu_maddubs_x4(&acc1, &opp[j * 2], wgt + InChunks, i, j, 1);
            relu_maddubs_x4(&acc2, &opp[j * 2], wgt + InChunks, i, j, 2);
            relu_maddubs_x4(&acc3, &opp[j * 2], wgt + InChunks, i, j, 3);
            relu_maddubs_x4(&acc4, &opp[j * 2], wgt + InChunks, i, j, 4);
            relu_maddubs_x4(&acc5, &opp[j * 2], wgt + InChunks, i, j, 5);
            relu_maddubs_x4(&acc6, &opp[j * 2], wgt + InChunks, i, j, 6);
            relu_maddubs_x4(&acc7, &opp[j * 2], wgt + InChunks, i, j, 7);
        }

        acc0 = vepi32_hadd(acc0, acc1);
        acc2 = vepi32_hadd(acc2, acc3);
        acc0 = vepi32_hadd(acc0, acc2);
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
        acc0 = _mm256_add_epi32(acc0, bia[i]);
        acc0 = _mm256_max_epi32(acc0, zero);
        out[i] = _mm256_cvtepi32_ps(acc0);

        #elif defined (USE_AVX)

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
        abort_nnue("Unable to read NNUE File");

    if (   fread(l1_biases, sizeof(int32_t), L2SIZE, fin) != (size_t) L2SIZE
        || fread(l1_weights, sizeof(int8_t), L1SIZE * L2SIZE, fin) != (size_t) L1SIZE * L2SIZE)
        abort_nnue("Unable to read NNUE File");

    if (   fread(l2_biases, sizeof(float), L3SIZE, fin) != (size_t) L3SIZE
        || fread(l2_weights, sizeof(float), L2SIZE * L3SIZE, fin) != (size_t) L2SIZE * L3SIZE)
        abort_nnue("Unable to read NNUE File");

    if (   fread(l3_biases, sizeof(float), OUTSIZE, fin) != (size_t) OUTSIZE
        || fread(l3_weights, sizeof(float), L3SIZE * OUTSIZE, fin) != (size_t) L3SIZE * OUTSIZE)
        abort_nnue("Unable to read NNUE File");

    scale_weights();
    shuffle_input_layer();
    quant_transpose(l1_weights, L1SIZE, L2SIZE);
    float_transpose(l2_weights, L2SIZE, L3SIZE);
    fclose(fin);

    NNUE_LOADED = 1;
}

void nnue_incbin_init() {

    // Inits from an NNUE file compiled into the binary. Assume the compiled
    // data is correct for the given NNUE config. Afterwords, scale some
    // weights for speed optimizations, and transpose the weights in L1 and L2

    #ifdef EVALFILE

    int8_t *data8; int16_t *data16; int32_t *data32; float *dataf;

    // Input layer uses 16-bit Biases and Weights

    data16 = (int16_t*) gIncWeightsData;
    for (int i = 0; i < KPSIZE; i++)
        in_biases[i] = *(data16++);

    for (int i = 0; i < INSIZE * KPSIZE; i++)
        in_weights[i] = *(data16++);

    // Layer one uses 32-bit Biases and 8-bit Weights

    data32 = (int32_t*) data16;
    for (int i = 0; i < L2SIZE; i++)
        l1_biases[i] = *(data32++);

    data8 = (int8_t*) data32;
    for (int i = 0; i < L1SIZE * L2SIZE; i++)
        l1_weights[i] = *(data8++);

    // Layer two and uses Floating Point Biases and Weights

    dataf = (float*) data8;
    for (int i = 0; i < L3SIZE; i++)
        l2_biases[i] = *(dataf++);

    for (int i = 0; i < L2SIZE * L3SIZE; i++)
        l2_weights[i] = *(dataf++);

    // Layer three and uses Floating Point Biases and Weights

    for (int i = 0; i < OUTSIZE; i++)
        l3_biases[i] = *(dataf++);

    for (int i = 0; i < L3SIZE * OUTSIZE; i++)
        l3_weights[i] = *(dataf++);

    scale_weights();
    shuffle_input_layer();
    quant_transpose(l1_weights, L1SIZE, L2SIZE);
    float_transpose(l2_weights, L2SIZE, L3SIZE);

    NNUE_LOADED = 1;

    #endif
}

int nnue_evaluate(Thread *thread, Board *board) {

    int mg_eval, eg_eval;
    const uint64_t white = board->colours[WHITE];
    const uint64_t black = board->colours[BLACK];
    const uint64_t kings = board->pieces[KING];

    if (!NNUE_LOADED)
        abort_nnue("NNUE File was not provided");

    // For optimizations, auto-flag KvK as drawn
    if (kings == (white | black)) return 0;

    // Optimized computation of various input indices
    int wrelksq = relativeSquare(WHITE, getlsb(white & kings));
    int brelksq = relativeSquare(BLACK, getlsb(black & kings));

    NNUEAccumulator *accum = thread->nnue->current;

    ALIGN64 float   outN1[L1SIZE];
    ALIGN64 float   outN2[L1SIZE];

    if (!accum->accurate[WHITE]) {

        // Possible to recurse and incrementally update each
        if (nnue_can_update(accum, board, WHITE))
            nnue_update_accumulator(accum, board, WHITE, wrelksq);

        // History is missing, we must refresh completely
        else
            nnue_refresh_accumulator(thread->nnue, accum, board, WHITE, wrelksq);
    }

    if (!accum->accurate[BLACK]) {

        // Possible to recurse and incrementally update each
        if (nnue_can_update(accum, board, BLACK))
            nnue_update_accumulator(accum, board, BLACK, brelksq);

        // History is missing, we must refresh completely
        else
            nnue_refresh_accumulator(thread->nnue, accum, board, BLACK, brelksq);
    }

    // Feed-forward the entire evaluation function
    halfkp_relu_quant_affine_relu(l1_weights, l1_biases, accum->values[board->turn], accum->values[!board->turn], outN1);
    float_affine_relu(l2_weights, l2_biases, outN1, outN2);
    output_transform (l3_weights, l3_biases, outN2, outN1);

    // Perform the dequantization step and upscale the Midgame
    mg_eval = 140 * ((int)(outN1[0]) >> SHIFT_L1) / 100;
    eg_eval = 100 * ((int)(outN1[0]) >> SHIFT_L1) / 100;

    // Cap the NNUE evaluation within [-2000, 2000]
    mg_eval = MAX(-2000, MIN(2000, mg_eval));
    eg_eval = MAX(-2000, MIN(2000, eg_eval));
    return MakeScore(mg_eval, eg_eval);
}
