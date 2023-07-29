#include <ctype.h>
#include <stdlib.h>

#include "attack.h"
#include "board.h"

struct Piece {
    attack_f attacks;
    char *unicode;
    ePiece piece;
    eColor color;
    char code;
    char asci;
};

// clang-format off
const struct Piece Pieces[2][6] = {
[WHITE] = {
    [KNIGHT] = {.color = WHITE, .code = 'N', .asci = 'N', .unicode = "♘ ", .piece = KNIGHT, .attacks = get_knight_attacks},
    [BISHOP] = {.color = WHITE, .code = 'B', .asci = 'B', .unicode = "♗ ", .piece = BISHOP, .attacks = get_bishop_attacks},
     [QUEEN] = {.color = WHITE, .code = 'Q', .asci = 'Q', .unicode = "♕ ",  .piece = QUEEN,  .attacks = get_queen_attacks},
      [KING] = {.color = WHITE, .code = 'K', .asci = 'K', .unicode = "♔ ",   .piece = KING,   .attacks = get_king_attacks},
      [PAWN] = {.color = WHITE, .code = 'P', .asci = 'P', .unicode = "♙ ",   .piece = PAWN,  .attacks = get_wpawn_attacks},
      [ROOK] = {.color = WHITE, .code = 'R', .asci = 'R', .unicode = "♖ ",   .piece = ROOK,   .attacks = get_rook_attacks},
    },
[BLACK] = {
    [KNIGHT] = {.color = BLACK, .code = 'n', .asci = 'n', .unicode = "♞ ", .piece = KNIGHT, .attacks = get_knight_attacks},
    [BISHOP] = {.color = BLACK, .code = 'b', .asci = 'b', .unicode = "♝ ", .piece = BISHOP, .attacks = get_bishop_attacks},
     [QUEEN] = {.color = BLACK, .code = 'q', .asci = 'q', .unicode = "♛ ",  .piece = QUEEN,  .attacks = get_queen_attacks},
      [KING] = {.color = BLACK, .code = 'k', .asci = 'k', .unicode = "♚ ",   .piece = KING,   .attacks = get_king_attacks},
      [PAWN] = {.color = BLACK, .code = 'p', .asci = 'p', .unicode = "♟ ",   .piece = PAWN,  .attacks = get_bpawn_attacks},
      [ROOK] = {.color = BLACK, .code = 'r', .asci = 'r', .unicode = "♜ ",   .piece = ROOK,   .attacks = get_rook_attacks},
    },
};
// clang-format on

attack_f piece_attacks(Piece self) { return self->attacks; }
char piece_asci(Piece self) { return self->asci; }
char piece_code(Piece self) { return self->code; }
char *piece_unicode(Piece self) { return self->unicode; }
eColor piece_color(Piece self) { return self->color; }
ePiece piece_piece(Piece self) { return self->piece; }
int piece_index(Piece self) { return self->color * 8 + self->piece; }

Piece piece_get(ePiece piece, eColor color) { return &Pieces[color][piece]; }
Piece piece_from_index(int index) { return &Pieces[index / 8][index % 8]; }
Piece piece_from_code(char code) {
    int color = (isupper(code)) ? WHITE : BLACK;
    for (int i = 0; i < 6; i++)
        if (Pieces[color][i].code == code) return &Pieces[color][i];
    return NULL;
}
