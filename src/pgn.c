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

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attacks.h"
#include "bitboards.h"
#include "board.h"
#include "move.h"
#include "pgn.h"

/// Ethereal's NNUE Data Format

typedef struct HalfKPSample {
    uint64_t occupied;   // 8-byte occupancy bitboard ( No Kings )
    int16_t  eval;       // 2-byte int for the target evaluation
    uint8_t  result;     // 1-byte int for result. { L=0, D=1, W=2 }
    uint8_t  turn;       // 1-byte int for the side-to-move flag
    uint8_t  wking;      // 1-byte int for the White King Square
    uint8_t  bking;      // 1-byte int for the Black King Square
    uint8_t  packed[15]; // 1-byte int per two non-King pieces
} HalfKPSample;

static void pack_bitboard(uint8_t *packed, Board *board, uint64_t pieces) {

    #define encode_piece(p) (8 * pieceColour(p) + pieceType(p))
    #define pack_pieces(p1, p2) (((p1) << 4) | (p2))

    uint8_t types[32] = {0};
    int N = (1 + popcount(pieces)) / 2;

    for (int i = 0; pieces; i++) {
        int sq = poplsb(&pieces);
        types[i] = encode_piece(board->squares[sq]);
    }

    for (int i = 0; i < N; i++)
        packed[i] = pack_pieces(types[i*2], types[i*2+1]);

    #undef encode_piece
    #undef pack_pieces
}

static void build_halfkp_sample(Board *board, HalfKPSample *sample, unsigned result, int16_t eval) {

    const uint64_t white  = board->colours[WHITE];
    const uint64_t black  = board->colours[BLACK];
    const uint64_t pieces = (white | black);

    sample->occupied = pieces & ~board->pieces[KING];
    sample->eval     = board->turn == BLACK ? -eval : eval;
    sample->result   = board->turn == BLACK ? 2u - result : result;
    sample->turn     = board->turn;
    sample->wking    = getlsb(white & board->pieces[KING]);
    sample->bking    = getlsb(black & board->pieces[KING]);
    pack_bitboard(sample->packed, board, sample->occupied);
}


static bool san_is_file(char chr) {
    return 'a' <= chr && chr <= 'h';
}

static bool san_is_rank(char chr) {
    return '1' <= chr && chr <= '8';
}

static bool san_is_square(const char *SAN) {
    return san_is_file(SAN[0]) && san_is_rank(SAN[1]);
}


static bool san_has_promotion(const char *SAN) {
    for (const char *ptr = SAN; *ptr != '\0' && *ptr != ' '; ptr++)
        if (*ptr == '=') return true;
    return false;
}

static bool san_has_capture(const char *SAN) {
    for (const char *ptr = SAN; *ptr != '\0' && *ptr != ' '; ptr++)
        if (*ptr == 'x') return true;
    return false;
}

static int san_square(const char *str) {
    return 8 * (str[1] - '1') + (str[0] - 'a');
}

static int san_promotion_type(char chr) {

    switch(chr) {
        case 'N' : return KNIGHT_PROMO_MOVE;
        case 'B' : return BISHOP_PROMO_MOVE;
        case 'R' : return ROOK_PROMO_MOVE;
        default  : return QUEEN_PROMO_MOVE;
    }
}


static uint16_t san_pawn_push(Board *board, const char *SAN) {

    int to, from, type;

    if (!san_is_square(SAN))
        return NONE_MOVE;

    // Assume a single pawn push
    to   = san_square(SAN);
    from = board->turn == WHITE ? to - 8 : to + 8;

    // Promotion is entirely handled by a move flag
    type = san_has_promotion(SAN)
         ? san_promotion_type(SAN[3]) : NORMAL_MOVE;

    // Account for double pawn pushes
    if (board->squares[from] != makePiece(PAWN, board->turn))
        from = board->turn == WHITE ? from - 8 : from + 8;

    // We can assert legality later
    return MoveMake(from, to, type);
}

static uint16_t san_pawn_capture(Board *board, const char *SAN) {

    uint64_t pawns;
    int file, tosq, type;

    // Pawn Captures have a file and then an 'x'
    if (!san_is_file(SAN[0]) || !san_has_capture(SAN))
        return NONE_MOVE;

    // Their could be a rank given for the moving piece (???)
    file = SAN[0] - 'a';
    tosq = san_square(SAN + (SAN[1] != 'x') + 2);

    // If we capture "nothing", then we really En Passant
    if (board->squares[tosq] == EMPTY) {
        int rank = board->turn == WHITE ? 4 : 3;
        return MoveMake(8 * rank + file, board->epSquare, ENPASS_MOVE);
    }

    // Promotion is entirely handled by a move flag
    type = !san_has_promotion(SAN) ? NORMAL_MOVE
         :  san_promotion_type(SAN[(SAN[1] != 'x') + 5]);

    // Narrow down the position of the capturing Pawn
    pawns = Files[file]
          & board->pieces[PAWN]
          & board->colours[board->turn]
          & pawnAttacks(!board->turn, tosq);

    return MoveMake(getlsb(pawns), tosq, type);
}

static uint16_t san_castle_move(Board *board, const char *SAN) {

    // Trivially check and build Queen Side Castles
    if (!strncmp(SAN, "O-O-O", 5)) {
        uint64_t friendly = board->colours[board->turn];
        int king = getlsb(friendly & board->pieces[KING]);
        int rook = getlsb(friendly & board->castleRooks);
        return MoveMake(king, rook, CASTLE_MOVE);
    }

    // Trivially check and build King Side Castles
    if (!strncmp(SAN, "O-O", 3)) {
        uint64_t friendly = board->colours[board->turn];
        int king = getlsb(friendly & board->pieces[KING]);
        int rook = getmsb(friendly & board->castleRooks);
        return MoveMake(king, rook, CASTLE_MOVE);
    }

    return NONE_MOVE;
}

static uint16_t san_piece_move(Board *board, const char *SAN) {

    int piece, tosq = -1;
    bool has_file, has_rank, has_capt;
    uint64_t options, occupied;

    // Decode the moving piece, which should be given
    switch (SAN[0]) {
        case 'K' : piece = KING;   break;
        case 'Q' : piece = QUEEN;  break;
        case 'R' : piece = ROOK;   break;
        case 'B' : piece = BISHOP; break;
        case 'N' : piece = KNIGHT; break;
        default  : return NONE_MOVE;
    }

    // Parse the SAN for various features. Captures are indicted by an 'x' inside
    // the SAN string. Checking of there is a File given requires you to identify
    // both a file, as well as another square, which is implied by the existence
    // of a capture. Likewise for Rank detection. The tosquare follows any 'x'

    has_capt = san_has_capture(SAN);

    has_file =  san_is_file(SAN[1])
            && (has_capt || (san_is_square(SAN + 2) || san_is_square(SAN + 3)));

    has_rank = has_capt ? san_is_rank(SAN[1]) || san_is_square(SAN + 1)
             :   (san_is_rank(SAN[1]) && san_is_square(SAN + 2))
              || (san_is_square(SAN + 1) && san_is_square(SAN + 3));

    tosq = san_square(SAN + has_file + has_rank + has_capt + 1);

    // From the to-sq, find any of our pieces which can attack. We ignore
    // pins, or otherwise illegal moves until later disambiguation

    occupied = board->colours[WHITE] | board->colours[BLACK];

    options = piece == KING   ?   kingAttacks(tosq)
            : piece == QUEEN  ?  queenAttacks(tosq, occupied)
            : piece == ROOK   ?   rookAttacks(tosq, occupied)
            : piece == BISHOP ? bishopAttacks(tosq, occupied)
            : piece == KNIGHT ? knightAttacks(tosq) : 0ull;

    options &= board->colours[board->turn]& board->pieces[piece];

    // Narrow down our options using the file disambiguation
    if (has_file)
        options &= Files[SAN[1] - 'a'];

    // Narrow down our options using the rank disambiguation
    if (has_rank)
        options &= Ranks[SAN[1 + has_file] - '1'];

    // If we only have one option, we can delay the legality check
    if (onlyOne(options))
        return MoveMake(getlsb(options), tosq, NORMAL_MOVE);

    // If we have multiple options due to pins, we must verify now
    while (options) {
        uint16_t move = MoveMake(poplsb(&options), tosq, NORMAL_MOVE);
        if (moveIsLegal(board, move)) return move;
    }

    // This should never happen, based on the call order of parse_san()
    return NONE_MOVE;
}

static uint16_t parse_san(Board *board, const char *SAN) {

    uint16_t move = NONE_MOVE;

    // Keep trying to parse the move until success or out of attempts
    if (move == NONE_MOVE) move = san_pawn_push(board, SAN);
    if (move == NONE_MOVE) move = san_pawn_capture(board, SAN);
    if (move == NONE_MOVE) move = san_castle_move(board, SAN);
    if (move == NONE_MOVE) move = san_piece_move(board, SAN);

    // This should not be needed, but lets verify to be safe
    return !moveIsLegal(board, move) ? NONE_MOVE : move;
}


static int pgn_read_until_move(char *buffer, int index) {
    for (; !isalpha(buffer[index]) && buffer[index] != '\0'; index++);
    return index;
}

static int pgn_read_until_space(char *buffer, int index) {
    for (; buffer[index] != ' ' && buffer[index] != '\0'; index++);
    return index;
}


static bool pgn_read_headers(FILE *pgn, PGNData *data) {

    if (fgets(data->buffer, 65536, pgn) == NULL)
        return false;

    if (strstr(data->buffer, "[White \"Ethereal") == data->buffer)
        data->is_white = true;

    else if (strstr(data->buffer, "[Black \"Ethereal") == data->buffer)
        data->is_black = true;

    else if (strstr(data->buffer, "[Result \"0-1\"]") == data->buffer)
        data->result = PGN_LOSS;

    else if (strstr(data->buffer, "[Result \"1/2-1/2\"]") == data->buffer)
        data->result = PGN_DRAW;

    else if (strstr(data->buffer, "[Result \"1-0\"]") == data->buffer)
        data->result = PGN_WIN;

    else if (strstr(data->buffer, "[Result \"*\"") == data->buffer)
        data->result = PGN_UNKNOWN_RESULT;

    else if (strstr(data->buffer, "[FEN \"") == data->buffer) {
        *strstr(data->buffer, "\"]") = '\0';
        data->startpos = strdup(data->buffer + strlen("[FEN \""));
    }

    return data->buffer[0] == '[';
}

static void pgn_read_moves(FILE *pgn, FILE *bindata, PGNData *data, HalfKPSample *samples, Board *board) {

    Undo undo;
    double feval;
    uint16_t move;
    int eval, placed = 0, index = 0;

    if (fgets(data->buffer, 65536, pgn) == NULL)
        return;

    while (1) {

        // Read and Apply the next move if there is one
        index = pgn_read_until_move(data->buffer, index);
        if (data->buffer[index] == '\0') break;
        move = parse_san(board, data->buffer + index);

        // Assume that each move has an associated score
        index = pgn_read_until_space(data->buffer, index);

        // Scan for an eval and ignore Mate scores
        if (sscanf(data->buffer + index + 1, "%lf", &feval) == 1)
            eval = round(100.0 * feval);
        else eval = MATE;

        // Use White's POV for all evaluations
        if (board->turn == BLACK) eval = -eval;

        // Use the sample if it is quiet and within [-2000, 2000] cp
        if (    abs(eval) <= 2000
            && !board->kingAttackers
            && !moveIsTactical(board, move)
            && (board->turn == WHITE ? data->is_white : data->is_black))
            build_halfkp_sample(board, &samples[placed++], data->result, eval);

        // Skip head to the end of this comment to prepare for the next Move
        index = pgn_read_until_space(data->buffer, index+1); data->plies++;
        applyMove(board, move, &undo);
    }

    if (data->result != PGN_UNKNOWN_RESULT)
        fwrite(samples, sizeof(HalfKPSample), placed, bindata);
}

static bool process_next_pgn(FILE *pgn, FILE *bindata, PGNData *data, HalfKPSample *samples, Board *board) {

    // Make sure to cleanup previous PGNs
    if (data->startpos != NULL)
        free(data->startpos);

    // Clear the current PGN to the blank state
    data->startpos = NULL;
    data->is_white = false;
    data->is_black = false;
    data->result   = PGN_NO_RESULT;
    data->plies    = 0;

    // Read Result & Fen and skip to Moves
    while (pgn_read_headers(pgn, data));

    // Process until we don't get a Result header
    if (data->result == PGN_NO_RESULT)
        return false;

    // Init the board, let Ethereal determine FRC
    boardFromFEN(board, data->startpos, 0);

    // Use all positions if neither matched Ethereal
    if (!data->is_white && !data->is_black)
        data->is_white = data->is_black = true;

    // Read Result & Fen and skip to Moves
    pgn_read_moves(pgn, bindata, data, samples, board);

    // Skip the trailing Newline of each PGN
    if (fgets(data->buffer, 65536, pgn) == NULL)
        return false;

    return true;
}


void process_pgn(const char *fin, const char *fout) {

    FILE *pgn = fopen(fin, "r");
    FILE *bindata = fopen(fout, "wb");
    PGNData *data = calloc(1, sizeof(PGNData));
    HalfKPSample *samples = calloc(1024, sizeof(HalfKPSample));

    Board board;
    while (process_next_pgn(pgn, bindata, data, samples, &board));
    fclose(pgn); fclose(bindata); free(data); free(samples);
}
