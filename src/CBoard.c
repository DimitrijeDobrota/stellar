#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <cul/assert.h>
#include <cul/mem.h>

#include "CBoard.h"
#include "attack.h"
#include "utils.h"

U64 CBoard_pieceBB_get(CBoard_T self, ePiece piece, Square target);

// PIECE
struct Piece_T {
  ePiece   piece;
  eColor   color;
  char     code;
  char     asci;
  char    *unicode;
  attack_f attacks;
};

// clang-format off
struct Piece_T Pieces[2][6] = {
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

attack_f Piece_attacks(Piece_T self) { return self->attacks; }
char     Piece_asci(Piece_T self) { return self->asci; }
char     Piece_code(Piece_T self) { return self->code; }
char    *Piece_unicode(Piece_T self) { return self->unicode; }
eColor   Piece_color(Piece_T self) { return self->color; }
ePiece   Piece_piece(Piece_T self) { return self->piece; }
int      Piece_index(Piece_T self) { return self->color * 8 + self->piece; }

Piece_T Piece_fromCode(char code) {
  int color = (isupper(code)) ? WHITE : BLACK;
  for (int i = 0; i < 6; i++)
    if (Pieces[color][i].code == code)
      return &Pieces[color][i];
  return NULL;
}

ePiece Piece_piece_fromCode(int index) {
  return Pieces[WHITE][index % 8].piece;
}

Piece_T Piece_fromIndex(int index) { return &Pieces[index / 8][index % 8]; }
Piece_T Piece_get(ePiece piece, eColor color) { return &Pieces[color][piece]; }

// CBOARD
struct CBoard_T {
  U64     colorBB[2];
  U64     pieceBB[6];
  eColor  side;
  Square  enpassant;
  eCastle castle;
};

CBoard_T CBoard_new(void) {
  CBoard_T p;
  NEW0(p);
  return p;
}

void CBoard_free(CBoard_T *p) { FREE(*p); }

void CBoard_copy(CBoard_T self, CBoard_T dest) { *dest = *self; }

Square  CBoard_enpassant(CBoard_T self) { return self->enpassant; }
eCastle CBoard_castle(CBoard_T self) { return self->castle; }
eColor  CBoard_side(CBoard_T self) { return self->side; }
U64 CBoard_colorBB(CBoard_T self, eColor color) { return self->colorBB[color]; }
U64 CBoard_pieceBB(CBoard_T self, ePiece piece) { return self->pieceBB[piece]; }
U64 CBoard_occupancy(CBoard_T self) {
  return self->colorBB[WHITE] | self->colorBB[BLACK];
}

U64 CBoard_pieceBB_get(CBoard_T self, ePiece piece, Square target) {
  return bit_get(self->pieceBB[piece], target);
}

U64 CBoard_pieceSet(CBoard_T self, Piece_T piece) {
  return self->pieceBB[Piece_piece(piece)] & self->colorBB[Piece_color(piece)];
}

void CBoard_enpassant_set(CBoard_T self, Square target) {
  self->enpassant = target;
}

void CBoard_colorBB_pop(CBoard_T self, eColor color, Square target) {
  bit_pop(self->colorBB[color], target);
}
void CBoard_colorBB_set(CBoard_T self, eColor color, Square target) {
  bit_set(self->colorBB[color], target);
}
U64 CBoard_colorBB_get(CBoard_T self, eColor color, Square target) {
  return bit_get(self->colorBB[color], target);
}

int CBoard_piece_get(CBoard_T self, Square square) {
  for (int i = 0; i < 6; i++)
    if (bit_get(self->pieceBB[i], square))
      return i;
  return -1;
}

void CBoard_piece_pop(CBoard_T self, Piece_T Piece, Square square) {
  bit_pop(self->pieceBB[Piece->piece], square);
  bit_pop(self->colorBB[Piece->color], square);
}

void CBoard_piece_set(CBoard_T self, Piece_T Piece, Square square) {
  bit_set(self->pieceBB[Piece->piece], square);
  bit_set(self->colorBB[Piece->color], square);
}

void CBoard_piece_move(CBoard_T self, Piece_T Piece, Square source,
                       Square target) {
  CBoard_piece_pop(self, Piece, source);
  CBoard_piece_set(self, Piece, target);
}

U64 CBoard_piece_attacks(CBoard_T self, Piece_T Piece, Square src) {
  return Piece_attacks(Piece)(src, CBoard_occupancy(self));
}

void CBoard_piece_capture(CBoard_T self, Piece_T Piece, Piece_T Taken,
                          Square source, Square target) {
  CBoard_piece_pop(self, Piece, source);
  if (Taken)
    CBoard_piece_pop(self, Taken, target);
  CBoard_piece_set(self, Piece, target);
}

void CBoard_castle_pop(CBoard_T self, eCastle castle) {
  bit_pop(self->castle, bit_lsb_index(castle));
}

void CBoard_castle_and(CBoard_T self, int exp) { self->castle &= exp; }
void CBoard_side_switch(CBoard_T self) { self->side = !self->side; }

int CBoard_isCheck(CBoard_T self) {
  U64 king = self->pieceBB[KING] & self->colorBB[self->side];
  return CBoard_square_isAttack(self, bit_lsb_index(king), !self->side);
}
int CBoard_square_isOccupied(CBoard_T self, Square square) {
  return bit_get(CBoard_occupancy(self), square);
}

int CBoard_square_isAttack(CBoard_T self, Square square, eColor side) {
  U64 occupancy = self->colorBB[WHITE] | self->colorBB[BLACK];

  for (int i = 0; i < 6; i++) {
    if (Pieces[!side][i].attacks(square, occupancy) & self->pieceBB[i] &
        self->colorBB[side])
      return 1;
  }

  return 0;
}

Piece_T CBoard_square_piece(CBoard_T self, Square square, eColor color) {
  for (ePiece i = 0; i < 6; i++)
    if (CBoard_pieceBB_get(self, i, square))
      return Piece_get(i, color);
  return NULL;
}

void CBoard_print(CBoard_T self) {
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      Square  square = (7 - rank) * 8 + file;
      Piece_T piece = NULL;

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

      if (!file)
        printf(" %d  ", 8 - rank);

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

CBoard_T CBoard_fromFEN(CBoard_T board, char *fen) {
  if (!board)
    NEW(board);

  memset(board, 0, sizeof(*board));

  board->side = -1;
  board->enpassant = no_sq;
  board->castle = 0;

  int file = 0, rank = 7;
  for (Piece_T piece; *fen != ' '; fen++) {
    Square square = rank * 8 + file;
    if (isalpha(*fen)) {
      if (!(piece = Piece_fromCode(*fen)))
        assert(0);
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
    case 'K': board->castle |= WK; break;
    case 'Q': board->castle |= WQ; break;
    case 'k': board->castle |= BK; break;
    case 'q': board->castle |= BQ; break;
    case '-': break;
    default: assert(0);
    }
  }

  fen++;
  if (*fen != '-') {
    board->enpassant = coordinates_to_square(fen);
  }

  return board;
}
