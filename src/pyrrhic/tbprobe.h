/*
 * Copyright (c) 2013-2020 Ronald de Man
 * Copyright (c) 2015 Basil, all rights reserved,
 * Modifications Copyright (c) 2016-2019 by Jon Dart
 * Modifications Copyright (c) 2020-2020 by Andrew Grant
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TBPROBE_H
#define TBPROBE_H

#include <stdint.h>
#include <stdbool.h>

#include "tbconfig.h"

/****************************************************************************/
/* MAIN API                                                                 */
/****************************************************************************/

#define TB_MAX_MOVES                256
#define TB_MAX_CAPTURES             64
#define TB_MAX_PLY                  256

#define TB_LOSS                     0       /* LOSS */
#define TB_BLESSED_LOSS             1       /* LOSS but 50-move draw */
#define TB_DRAW                     2       /* DRAW */
#define TB_CURSED_WIN               3       /* WIN but 50-move draw  */
#define TB_WIN                      4       /* WIN  */

#define TB_RESULT_WDL_MASK          0x0000000F
#define TB_RESULT_TO_MASK           0x000003F0
#define TB_RESULT_FROM_MASK         0x0000FC00
#define TB_RESULT_PROMOTES_MASK     0x00070000
#define TB_RESULT_EP_MASK           0x00080000
#define TB_RESULT_DTZ_MASK          0xFFF00000
#define TB_RESULT_WDL_SHIFT         0
#define TB_RESULT_TO_SHIFT          4
#define TB_RESULT_FROM_SHIFT        10
#define TB_RESULT_PROMOTES_SHIFT    16
#define TB_RESULT_EP_SHIFT          19
#define TB_RESULT_DTZ_SHIFT         20

#define TB_GET_WDL(_res)                        \
    (((_res) & TB_RESULT_WDL_MASK) >> TB_RESULT_WDL_SHIFT)
#define TB_GET_TO(_res)                         \
    (((_res) & TB_RESULT_TO_MASK) >> TB_RESULT_TO_SHIFT)
#define TB_GET_FROM(_res)                       \
    (((_res) & TB_RESULT_FROM_MASK) >> TB_RESULT_FROM_SHIFT)
#define TB_GET_PROMOTES(_res)                   \
    (((_res) & TB_RESULT_PROMOTES_MASK) >> TB_RESULT_PROMOTES_SHIFT)
#define TB_GET_EP(_res)                         \
    (((_res) & TB_RESULT_EP_MASK) >> TB_RESULT_EP_SHIFT)
#define TB_GET_DTZ(_res)                        \
    (((_res) & TB_RESULT_DTZ_MASK) >> TB_RESULT_DTZ_SHIFT)

#define TB_SET_WDL(_res, _wdl)                  \
    (((_res) & ~TB_RESULT_WDL_MASK) |           \
     (((_wdl) << TB_RESULT_WDL_SHIFT) & TB_RESULT_WDL_MASK))
#define TB_SET_TO(_res, _to)                    \
    (((_res) & ~TB_RESULT_TO_MASK) |            \
     (((_to) << TB_RESULT_TO_SHIFT) & TB_RESULT_TO_MASK))
#define TB_SET_FROM(_res, _from)                \
    (((_res) & ~TB_RESULT_FROM_MASK) |          \
     (((_from) << TB_RESULT_FROM_SHIFT) & TB_RESULT_FROM_MASK))
#define TB_SET_PROMOTES(_res, _promotes)        \
    (((_res) & ~TB_RESULT_PROMOTES_MASK) |      \
     (((_promotes) << TB_RESULT_PROMOTES_SHIFT) & TB_RESULT_PROMOTES_MASK))
#define TB_SET_EP(_res, _ep)                    \
    (((_res) & ~TB_RESULT_EP_MASK) |            \
     (((_ep) << TB_RESULT_EP_SHIFT) & TB_RESULT_EP_MASK))
#define TB_SET_DTZ(_res, _dtz)                  \
    (((_res) & ~TB_RESULT_DTZ_MASK) |           \
     (((_dtz) << TB_RESULT_DTZ_SHIFT) & TB_RESULT_DTZ_MASK))

#define TB_RESULT_CHECKMATE         TB_SET_WDL(0, TB_WIN)
#define TB_RESULT_STALEMATE         TB_SET_WDL(0, TB_DRAW)
#define TB_RESULT_FAILED            0xFFFFFFFF

/*
 * The tablebase can be probed for any position where #pieces <= TB_LARGEST.
 */
extern int TB_LARGEST;

/*
 * Initialize the tablebase.
 *
 * PARAMETERS:
 * - path:
 *   The tablebase PATH string.
 *
 * RETURN:
 * - true=succes, false=failed.  The TB_LARGEST global will also be
 *   initialized.  If no tablebase files are found, then `true' is returned
 *   and TB_LARGEST is set to zero.
 */
bool tb_init(const char *_path);

/*
 * Free any resources allocated by tb_init
 */
void tb_free(void);

/*
 * Probe the Win-Draw-Loss (WDL) table.
 *
 * PARAMETERS:
 * - white, black, kings, queens, rooks, bishops, knights, pawns:
 *   The current position (bitboards).
 * - ep:
 *   The en passant square (if exists).  Set to zero if there is no en passant
 *   square.
 * - turn:
 *   true=white, false=black
 *
 * RETURN:
 * - One of {TB_LOSS, TB_BLESSED_LOSS, TB_DRAW, TB_CURSED_WIN, TB_WIN}.
 *   Otherwise returns TB_RESULT_FAILED if the probe failed.
 *
 * NOTES:
 * - Engines should use this function during search.
 * - This function is thread safe assuming TB_NO_THREADS is disabled.
 */
unsigned tb_probe_wdl(
    uint64_t white,     uint64_t black,
    uint64_t kings,     uint64_t queens,
    uint64_t rooks,     uint64_t bishops,
    uint64_t knights,   uint64_t pawns,
    unsigned ep,        bool     turn);

/*
 * Probe the Distance-To-Zero (DTZ) table.
 *
 * PARAMETERS:
 * - white, black, kings, queens, rooks, bishops, knights, pawns:
 *   The current position (bitboards).
 * - rule50:
 *   The 50-move half-move clock.
 * - castling:
 *   Castling rights.  Set to zero if no castling is possible.
 * - ep:
 *   The en passant square (if exists).  Set to zero if there is no en passant
 *   square.
 * - turn:
 *   true=white, false=black
 * - results (OPTIONAL):
 *   Alternative results, one for each possible legal move.  The passed array
 *   must be TB_MAX_MOVES in size.
 *   If alternative results are not desired then set results=NULL.
 *
 * RETURN:
 * - A TB_RESULT value comprising:
 *   1) The WDL value (TB_GET_WDL)
 *   2) The suggested move (TB_GET_FROM, TB_GET_TO, TB_GET_PROMOTES, TB_GET_EP)
 *   3) The DTZ value (TB_GET_DTZ)
 *   The suggested move is guaranteed to preserved the WDL value.
 *
 *   Otherwise:
 *   1) TB_RESULT_STALEMATE is returned if the position is in stalemate.
 *   2) TB_RESULT_CHECKMATE is returned if the position is in checkmate.
 *   3) TB_RESULT_FAILED is returned if the probe failed.
 *
 *   If results!=NULL, then a TB_RESULT for each legal move will be generated
 *   and stored in the results array.  The results array will be terminated
 *   by TB_RESULT_FAILED.
 *
 * NOTES:
 * - Engines can use this function to probe at the root.  This function should
 *   not be used during search.
 * - DTZ tablebases can suggest unnatural moves, especially for losing
 *   positions.  Engines may prefer to traditional search combined with WDL
 *   move filtering using the alternative results array.
 * - This function is NOT thread safe.  For engines this function should only
 *   be called once at the root per search.
 */
unsigned tb_probe_root(
    uint64_t white,    uint64_t black,
    uint64_t kings,    uint64_t queens,
    uint64_t rooks,    uint64_t bishops,
    uint64_t knights,  uint64_t pawns,
    unsigned rule50,   unsigned ep,
    bool     turn,     unsigned *results);


typedef uint16_t PyrrhicMove;

struct TbRootMove {
  PyrrhicMove move;
  PyrrhicMove pv[TB_MAX_PLY];
  unsigned pvSize;
  int32_t tbScore, tbRank;
};

struct TbRootMoves {
  unsigned size;
  struct TbRootMove moves[TB_MAX_MOVES];
};

/*
 * Use the DTZ tables to rank and score all root moves.
 * INPUT: as for tb_probe_root
 * OUTPUT: TbRootMoves structure is filled in. This contains
 * an array of TbRootMove structures.
 * Each structure instance contains a rank, a score, and a
 * predicted principal variation.
 * RETURN VALUE:
 *   non-zero if ok, 0 means not all probes were successful
 *
 */
int tb_probe_root_dtz(
    uint64_t white,    uint64_t black,
    uint64_t kings,    uint64_t queens,
    uint64_t rooks,    uint64_t bishops,
    uint64_t knights,  uint64_t pawns,
    unsigned rule50,   unsigned ep,
    bool     turn,     bool hasRepeated,
    bool useRule50,    struct TbRootMoves *results);

/*
// Use the WDL tables to rank and score all root moves.
// This is a fallback for the case that some or all DTZ tables are missing.
 * INPUT: as for tb_probe_root
 * OUTPUT: TbRootMoves structure is filled in. This contains
 * an array of TbRootMove structures.
 * Each structure instance contains a rank, a score, and a
 * predicted principal variation.
 * RETURN VALUE:
 *   non-zero if ok, 0 means not all probes were successful
 *
 */
int tb_probe_root_wdl(
    uint64_t white,    uint64_t black,
    uint64_t kings,    uint64_t queens,
    uint64_t rooks,    uint64_t bishops,
    uint64_t knights,  uint64_t pawns,
    unsigned rule50,   unsigned ep,
    bool     turn,     bool useRule50,
    struct TbRootMoves *_results);

#endif
