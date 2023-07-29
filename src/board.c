#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <cul/assert.h>
#include <cul/mem.h>

#include "attack.h"
#include "board.h"
#include "utils.h"

U64 board_pieceBB_get(Board self, ePiece piece, Square target);

// PIECE
struct Piece {
    ePiece piece;
    eColor color;
    char code;
    char asci;
    char *unicode;
    attack_f attacks;
};

// clang-format off
struct Piece Pieces[2][6] = {
    {
  [PAWN] = {.color = WHITE, .code = 'P', .asci = 'P', .unicode = "♙ ",   .piece = PAWN,  .attacks = get_wpawn_attacks},
[KNIGHT] = {.color = WHITE, .code = 'N', .asci = 'N', .unicode = "♘ ", .piece = KNIGHT, .attacks = get_knight_attacks},
[BISHOP] = {.color = WHITE, .code = 'B', .asci = 'B', .unicode = "♗ ", .piece = BISHOP, .attacks = get_bishop_attacks},
  [ROOK] = {.color = WHITE, .code = 'R', .asci = 'R', .unicode = "♖ ",   .piece = ROOK,   .attacks = get_rook_attacks},
 [QUEEN] = {.color = WHITE, .code = 'Q', .asci = 'Q', .unicode = "♕ ",  .piece = QUEEN,  .attacks = get_queen_attacks},
  [KING] = {.color = WHITE, .code = 'K', .asci = 'K', .unicode = "♔ ",   .piece = KING,   .attacks = get_king_attacks},
    },
    {
  [PAWN] = {.color = BLACK, .code = 'p', .asci = 'p', .unicode = "♟ ",   .piece = PAWN,  .attacks = get_bpawn_attacks},
[KNIGHT] = {.color = BLACK, .code = 'n', .asci = 'n', .unicode = "♞ ", .piece = KNIGHT, .attacks = get_knight_attacks},
[BISHOP] = {.color = BLACK, .code = 'b', .asci = 'b', .unicode = "♝ ", .piece = BISHOP, .attacks = get_bishop_attacks},
  [ROOK] = {.color = BLACK, .code = 'r', .asci = 'r', .unicode = "♜ ",   .piece = ROOK,   .attacks = get_rook_attacks},
 [QUEEN] = {.color = BLACK, .code = 'q', .asci = 'q', .unicode = "♛ ",  .piece = QUEEN,  .attacks = get_queen_attacks},
  [KING] = {.color = BLACK, .code = 'k', .asci = 'k', .unicode = "♚ ",   .piece = KING,   .attacks = get_king_attacks},
    },
};
// clang-format on

attack_f Piece_attacks(Piece self) { return self->attacks; }
char Piece_asci(Piece self) { return self->asci; }
char Piece_code(Piece self) { return self->code; }
char *Piece_unicode(Piece self) { return self->unicode; }
eColor Piece_color(Piece self) { return self->color; }
ePiece Piece_piece(Piece self) { return self->piece; }
int Piece_index(Piece self) { return self->color * 8 + self->piece; }

Piece Piece_fromCode(char code) {
    int color = (isupper(code)) ? WHITE : BLACK;
    for (int i = 0; i < 6; i++)
        if (Pieces[color][i].code == code) return &Pieces[color][i];
    return NULL;
}

ePiece Piece_piece_fromCode(int index) {
    return Pieces[WHITE][index % 8].piece;
}

Piece Piece_fromIndex(int index) { return &Pieces[index / 8][index % 8]; }
Piece Piece_get(ePiece piece, eColor color) { return &Pieces[color][piece]; }

// CBOARD
struct Board {
    U64 colorBB[2];
    U64 pieceBB[6];
    eColor side;
    Square enpassant;
    eCastle castle;
};

Board board_new(void) {
    Board p;
    NEW0(p);
    return p;
}

void board_free(Board *p) { FREE(*p); }

void board_copy(Board self, Board dest) { *dest = *self; }

Square board_enpassant(Board self) { return self->enpassant; }
eCastle board_castle(Board self) { return self->castle; }
eColor board_side(Board self) { return self->side; }
U64 board_colorBB(Board self, eColor color) { return self->colorBB[color]; }
U64 board_pieceBB(Board self, ePiece piece) { return self->pieceBB[piece]; }
U64 board_occupancy(Board self) {
    return self->colorBB[WHITE] | self->colorBB[BLACK];
}

U64 board_pieceBB_get(Board self, ePiece piece, Square target) {
    return bit_get(self->pieceBB[piece], target);
}

U64 board_pieceSet(Board self, Piece piece) {
    return self->pieceBB[Piece_piece(piece)] &
           self->colorBB[Piece_color(piece)];
}

void board_enpassant_set(Board self, Square target) {
    self->enpassant = target;
}

void board_colorBB_pop(Board self, eColor color, Square target) {
    bit_pop(self->colorBB[color], target);
}
void board_colorBB_set(Board self, eColor color, Square target) {
    bit_set(self->colorBB[color], target);
}
U64 board_colorBB_get(Board self, eColor color, Square target) {
    return bit_get(self->colorBB[color], target);
}

int board_piece_get(Board self, Square square) {
    for (int i = 0; i < 6; i++)
        if (bit_get(self->pieceBB[i], square)) return i;
    return -1;
}

void board_piece_pop(Board self, Piece Piece, Square square) {
    bit_pop(self->pieceBB[Piece->piece], square);
    bit_pop(self->colorBB[Piece->color], square);
}

void board_piece_set(Board self, Piece Piece, Square square) {
    bit_set(self->pieceBB[Piece->piece], square);
    bit_set(self->colorBB[Piece->color], square);
}

void board_piece_move(Board self, Piece Piece, Square source, Square target) {
    board_piece_pop(self, Piece, source);
    board_piece_set(self, Piece, target);
}

U64 board_piece_attacks(Board self, Piece Piece, Square src) {
    return Piece_attacks(Piece)(src, board_occupancy(self));
}

void board_piece_capture(Board self, Piece piece, Piece taken, Square source,
                         Square target) {
    board_piece_pop(self, piece, source);
    if (taken) board_piece_pop(self, taken, target);
    board_piece_set(self, piece, target);
}

void board_castle_pop(Board self, eCastle castle) {
    bit_pop(self->castle, bit_lsb_index(castle));
}

void board_castle_and(Board self, int exp) { self->castle &= exp; }
void board_side_switch(Board self) { self->side = !self->side; }

int board_isCheck(Board self) {
    U64 king = self->pieceBB[KING] & self->colorBB[self->side];
    return board_square_isAttack(self, bit_lsb_index(king), !self->side);
}
int board_square_isOccupied(Board self, Square square) {
    return bit_get(board_occupancy(self), square);
}

int board_square_isAttack(Board self, Square square, eColor side) {
    U64 occupancy = self->colorBB[WHITE] | self->colorBB[BLACK];

    for (int i = 0; i < 6; i++) {
        if (Pieces[!side][i].attacks(square, occupancy) & self->pieceBB[i] &
            self->colorBB[side])
            return 1;
    }

    return 0;
}

Piece board_square_piece(Board self, Square square, eColor color) {
    for (ePiece i = 0; i < 6; i++)
        if (board_pieceBB_get(self, i, square)) return Piece_get(i, color);
    return NULL;
}

void board_print(Board self) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            Square square = (7 - rank) * 8 + file;
            Piece piece = NULL;

            int color = -1;
            if (bit_get(self->colorBB[WHITE], square))
                color = WHITE;
            else if (bit_get(self->colorBB[BLACK], square))
                color = BLACK;

            if (color != -1) {
                for (int piece_index = 0; piece_index < 6; piece_index++) {
                    if (bit_get(self->pieceBB[piece_index], square)) {
                        piece = &Pieces[color][piece_index];
                        break;
                    }
                }
            }

            if (!file) printf(" %d  ", 8 - rank);

            printf("%s", (piece) ? Piece_unicode(piece) : ". ");
        }
        printf("\n");
    }
    printf("    A B C D E F G H\n");
    printf("     Side: %s\n", (self->side == WHITE) ? "white" : "black");
    printf("Enpassant: %s\n", square_to_coordinates[self->enpassant]);
    printf(" Castling: %c%c%c%c\n", (self->castle & WK) ? 'K' : '-',
           (self->castle & WQ) ? 'Q' : '-', (self->castle & BK) ? 'k' : '-',
           (self->castle & BQ) ? 'q' : '-');
    printf("\n");
}

Board board_fromFEN(Board board, char *fen) {
    if (!board) NEW(board);

    memset(board, 0, sizeof(*board));

    board->side = -1;
    board->enpassant = no_sq;
    board->castle = 0;

    int file = 0, rank = 7;
    for (Piece piece; *fen != ' '; fen++) {
        Square square = rank * 8 + file;
        if (isalpha(*fen)) {
            if (!(piece = Piece_fromCode(*fen))) assert(0);
            bit_set(board->colorBB[piece->color], square);
            bit_set(board->pieceBB[piece->piece], square);
            file++;
        } else if (isdigit(*fen)) {
            file += *fen - '0';
        } else if (*fen == '/') {
            file = 0;
            rank--;
        } else
            assert(0);
    }

    fen++;
    if (*fen == 'w')
        board->side = WHITE;
    else if (*fen == 'b')
        board->side = BLACK;
    else
        assert(0);

    for (fen += 2; *fen != ' '; fen++) {
        switch (*fen) {
        case 'K':
            board->castle |= WK;
            break;
        case 'Q':
            board->castle |= WQ;
            break;
        case 'k':
            board->castle |= BK;
            break;
        case 'q':
            board->castle |= BQ;
            break;
        case '-':
            break;
        default:
            assert(0);
        }
    }

    fen++;
    if (*fen != '-') {
        board->enpassant = coordinates_to_square(fen);
    }

    return board;
}
