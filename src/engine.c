#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "attack.h"
#include "utils.h"

#include <cii/assert.h>
#include <cii/except.h>
#include <cii/mem.h>

/* DEBUGGING */

// FEN debug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position                                                         \
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position                                                        \
  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position                                                        \
  "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position                                                           \
  "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

// piece representation
typedef struct Piece_T *Piece_T;
struct Piece_T {
  ePiece   piece;
  eColor   color;
  char     code;
  char     asci;
  char    *unicode;
  attack_f attacks;
};

ePiece Piece_piece(Piece_T pt) { return pt->piece; }
eColor Piece_color(Piece_T pt) { return pt->color; }
char   Piece_code(Piece_T pt) { return pt->code; }
char   Piece_asci(Piece_T pt) { return pt->asci; }
char  *Piece_unicode(Piece_T pt) { return pt->unicode; }

// Pieces table [color][piece]
// clang-format off
struct Piece_T Pieces[2][6] = {
    {
     {.color = WHITE, .code = 'P', .asci = 'P', .unicode = "♙ ",   .piece = PAWN,  .attacks = get_wpawn_attacks},
     {.color = WHITE, .code = 'N', .asci = 'N', .unicode = "♘ ", .piece = KNIGHT, .attacks = get_knight_attacks},
     {.color = WHITE, .code = 'B', .asci = 'B', .unicode = "♗ ", .piece = BISHOP, .attacks = get_bishop_attacks},
     {.color = WHITE, .code = 'R', .asci = 'R', .unicode = "♖ ",   .piece = ROOK,   .attacks = get_rook_attacks},
     {.color = WHITE, .code = 'Q', .asci = 'Q', .unicode = "♕ ",  .piece = QUEEN,  .attacks = get_queen_attacks},
     {.color = WHITE, .code = 'K', .asci = 'K', .unicode = "♔ ",   .piece = KING,   .attacks = get_king_attacks},
    },
    {
     {.color = BLACK, .code = 'p', .asci = 'p', .unicode = "♟ ",   .piece = PAWN,  .attacks = get_bpawn_attacks},
     {.color = BLACK, .code = 'n', .asci = 'n', .unicode = "♞ ", .piece = KNIGHT, .attacks = get_knight_attacks},
     {.color = BLACK, .code = 'b', .asci = 'b', .unicode = "♝ ", .piece = BISHOP, .attacks = get_bishop_attacks},
     {.color = BLACK, .code = 'r', .asci = 'r', .unicode = "♜ ",   .piece = ROOK,   .attacks = get_rook_attacks},
     {.color = BLACK, .code = 'q', .asci = 'q', .unicode = "♛ ",  .piece = QUEEN,  .attacks = get_queen_attacks},
     {.color = BLACK, .code = 'k', .asci = 'k', .unicode = "♚ ",   .piece = KING,   .attacks = get_king_attacks},
    },
};
// clang-format on
int Piece_index(Piece_T self) { return self->color * 8 + self->piece; }

Piece_T Piece_fromCode(char code) {
  int color = (isupper(code)) ? WHITE : BLACK;
  for (int i = 0; i < 6; i++)
    if (Pieces[color][i].code == code)
      return &Pieces[color][i];
  return NULL;
}

Piece_T Piece_fromInex(int index) { return &Pieces[index / 8][index % 8]; }

enum enumCastle { WK = 1, WQ = 2, BK = 4, BQ = 8 };
typedef enum enumCastle eCastle;

typedef U32 Move;
#define Move_encode(source, target, piece, promoted, capture, dbl, enpassant,  \
                    castling)                                                  \
  (source) | (target << 6) | (piece << 12) | (promoted << 16) |                \
      (capture << 20) | (dbl << 21) | (enpassant << 22) | (castling << 23)

#define Move_source(move)    (((move)&C32(0x00003f)))
#define Move_target(move)    (((move)&C32(0x000fc0)) >> 6)
#define Move_piece(move)     (((move)&C32(0x00f000)) >> 12)
#define Move_promote(move)   (((move)&C32(0x0f0000)) >> 16)
#define Move_capture(move)   (((move)&C32(0x100000)) >> 20)
#define Move_double(move)    (((move)&C32(0x200000)))
#define Move_enpassant(move) (((move)&C32(0x400000)))
#define Move_castle(move)    (((move)&C32(0x800000)))

void Move_print(Move move) {
  int promote = Move_promote(move);
  printf("%5s %5s %5s %5c %4d %4d %4d %4d\n",
         square_to_coordinates[Move_source(move)],
         square_to_coordinates[Move_target(move)],
         Piece_fromInex(Move_piece(move))->unicode,
         promote ? Piece_fromInex(promote)->asci : 'X',
         Move_capture(move) ? 1 : 0, Move_double(move) ? 1 : 0,
         Move_enpassant(move) ? 1 : 0, Move_castle(move) ? 1 : 0);
}

typedef struct MoveList_T *MoveList_T;
struct MoveList_T {
  Move moves[256];
  int  count;
};

MoveList_T MoveList_new(void) {
  MoveList_T p;
  NEW0(p);
  return p;
}

void MoveList_add(MoveList_T self, Move move) {
  self->moves[self->count++] = move;
}

void MoveList_print(MoveList_T self) {
  printf(" From    To  Pi  Prmt  Cap  Dbl  Enp  Cst\n");
  for (int i = 0; i < self->count; i++)
    Move_print(self->moves[i]);
  printf("Total: %d\n", self->count);
}

void MoveList_free(MoveList_T *p) { FREE(*p); }

// board representation
typedef struct CBoard_T *CBoard_T;
struct CBoard_T {
  U64     colorBB[2];
  U64     pieceBB[6];
  eColor  side;
  Square  enpassant;
  eCastle castle;
};

U64 CBoard_getPieceSet(CBoard_T self, Piece_T piece) {
  return self->pieceBB[Piece_color(piece)] & self->pieceBB[Piece_color(piece)];
}
U64 CBoard_getWhitePawns(CBoard_T self) {
  return self->pieceBB[PAWN] & self->pieceBB[WHITE];
}
U64 CBoard_getBlackPawns(CBoard_T self) {
  return self->pieceBB[PAWN] & self->pieceBB[BLACK];
}
U64 CBoard_getPawns(CBoard_T self, eColor color) {
  return self->pieceBB[PAWN] & self->pieceBB[color];
}

/* ... */

CBoard_T CBoard_fromFEN(CBoard_T board, char *fen) {
  if (!board)
    NEW(board);

  memset(board, C64(0), sizeof(*board));

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
    board->enpassant = (*(fen + 1) - '1') * 8 + (*fen - 'a');
  }

  return board;
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

#define pawn_promotion(color, source)                                          \
  ((color == WHITE && source >= a7 && source <= h7) ||                         \
   (color == BLACK && source >= a2 && source <= h2))

#define pawn_start(color, source)                                              \
  ((color == BLACK && source >= a7 && source <= h7) ||                         \
   (color == WHITE && source >= a2 && source <= h2))

#define pawn_promote(source, target, index, capture)                           \
  for (int i = 1; i < 5; i++) {                                                \
    move = Move_encode(source, target, index, Piece_index(&Pieces[color][i]),  \
                       capture, 0, 0, 0);                                      \
    MoveList_add(moves, move);                                                 \
  }

MoveList_T CBoard_move_generate(CBoard_T self) {
  MoveList_T moves;
  Move       move;
  Square     src, tgt;
  U64        occupancy = self->colorBB[WHITE] | self->colorBB[BLACK];
  eColor     color = self->side;

  moves = MoveList_new();

  { // pawn moves
    Piece_T Piece = &Pieces[color][PAWN];
    int     index = Piece_index(Piece);
    U64     bitboard = self->pieceBB[PAWN] & self->colorBB[color];
    bitboard_for_each_bit(src, bitboard) {
      { // quiet
        int add = (color == WHITE) ? +8 : -8;
        tgt = src + add;
        if (tgt > a1 && tgt < h8 && !bit_get(occupancy, tgt)) {
          if (pawn_promotion(color, src)) {
            pawn_promote(src, tgt, index, 0);
          } else {
            MoveList_add(moves, Move_encode(src, tgt, index, 0, 0, 0, 0, 0));

            // two ahead
            if (pawn_start(color, src) && !bit_get(occupancy, tgt += add))
              MoveList_add(moves, Move_encode(src, tgt, index, 0, 0, 1, 0, 0));
          }
        }
      }
      { // capture
        U64 attack = Piece->attacks(src, occupancy) & self->colorBB[!color];
        bitboard_for_each_bit(tgt, attack) {
          if (pawn_promotion(color, src)) {
            pawn_promote(src, tgt, index, 1);
          } else {
            MoveList_add(moves, Move_encode(src, tgt, index, 0, 1, 0, 0, 0));
          }
        }
      }

      { // en passant
        U64 attack = Piece->attacks(src, occupancy);
        if (self->enpassant != no_sq && attack & (C64(1) << self->enpassant))
          MoveList_add(moves, Move_encode(src, bit_lsb_index(attack), index, 0,
                                          1, 0, 1, 0));
      }
    }
  }

  // All piece move
  for (int piece = 1; piece < 6; piece++) {
    Piece_T Piece = &Pieces[color][piece];
    U64     bitboard = self->pieceBB[piece] & self->colorBB[color];
    bitboard_for_each_bit(src, bitboard) {
      U64 attack = Piece->attacks(src, occupancy) & ~self->colorBB[color];
      bitboard_for_each_bit(tgt, attack) {
        int take = bit_get(self->colorBB[!color], tgt);
        MoveList_add(
            moves, Move_encode(src, tgt, Piece_index(Piece), 0, take, 0, 0, 0));
      }
    }
  }

  // Castling
  {
    if (color == WHITE && !CBoard_square_isAttack(self, e1, BLACK)) {
      int index = Piece_index(&Pieces[WHITE][KING]);
      if (self->castle & WK && !bit_get(occupancy, f1) &&
          !bit_get(occupancy, g1)) {
        MoveList_add(moves, Move_encode(e1, g1, index, 0, 0, 0, 0, 1));
      }
      if (self->castle & WQ && !bit_get(occupancy, b1) &&
          !bit_get(occupancy, c1) && !bit_get(occupancy, d1)) {
        MoveList_add(moves, Move_encode(e1, c1, index, 0, 0, 0, 0, 1));
      }
    }

    if (color == BLACK && !CBoard_square_isAttack(self, e8, WHITE)) {
      int index = Piece_index(&Pieces[BLACK][KING]);
      if (self->castle & BK && !bit_get(occupancy, f8) &&
          !bit_get(occupancy, g8)) {
        MoveList_add(moves, Move_encode(e8, g8, index, 0, 0, 0, 0, 1));
      }
      if (self->castle & BQ && !bit_get(occupancy, b8) &&
          !bit_get(occupancy, c8) && !bit_get(occupancy, d8)) {
        MoveList_add(moves, Move_encode(e8, c8, index, 0, 0, 0, 0, 1));
      }
    }
  }

  return moves;
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

int CBoard_move_make(CBoard_T self, Move move, int flag) {
  if (flag == 0) {

    Square  source = Move_source(move);
    Square  target = Move_target(move);
    Piece_T Piece = Piece_fromInex(Move_piece(move));

    if (!Move_castle(move))
      CBoard_piece_move(self, Piece, source, target);

    if (Move_capture(move)) {
      bit_pop(self->colorBB[!Piece->color], target);
      for (ePiece i = 0; i < 6; i++)
        if (i != Piece->piece && bit_get(self->pieceBB[i], target)) {
          bit_pop(self->pieceBB[i], target);
          break;
        }
    }

    if (Move_promote(move)) {
      Piece_T Promote = Piece_fromInex(Move_promote(move));
      bit_pop(self->pieceBB[Piece->piece], target);
      bit_set(self->pieceBB[Promote->piece], target);
    }

    if (Move_enpassant(move)) {
      target += (Piece->color == WHITE) ? -8 : +8;
      CBoard_piece_pop(self, &Pieces[!Piece->color][PAWN], source);
    }

    if (Move_double(move))
      self->enpassant = target + (Piece->color == WHITE ? -8 : +8);
    else
      self->enpassant = no_sq;

    if (Move_castle(move)) {
      if (self->side == WHITE) {
        Piece_T Rook = &Pieces[WHITE][ROOK];
        if (target == g1) {
          if (CBoard_square_isAttack(self, f1, BLACK) ||
              CBoard_square_isAttack(self, g1, BLACK))
            return 0;
          CBoard_piece_move(self, Rook, h1, f1);
          CBoard_piece_move(self, Piece, source, target);
          bit_pop(self->castle, 0);
          bit_pop(self->castle, 1);
        } else {
          if (CBoard_square_isAttack(self, c1, BLACK) ||
              CBoard_square_isAttack(self, d1, BLACK))
            return 0;
          CBoard_piece_move(self, Rook, a1, d1);
          CBoard_piece_move(self, Piece, source, target);
          bit_pop(self->castle, 0);
          bit_pop(self->castle, 1);
        }
      } else {
        Piece_T Rook = &Pieces[BLACK][ROOK];
        if (target == g8) {
          if (CBoard_square_isAttack(self, f8, WHITE) ||
              CBoard_square_isAttack(self, g8, WHITE))
            return 0;
          CBoard_piece_move(self, Rook, h8, f8);
          CBoard_piece_move(self, Piece, source, target);
          bit_pop(self->castle, 2);
          bit_pop(self->castle, 3);
        } else {
          if (CBoard_square_isAttack(self, c8, WHITE) ||
              CBoard_square_isAttack(self, d8, WHITE))
            return 0;
          CBoard_piece_move(self, Rook, a8, d8);
          CBoard_piece_move(self, Piece, source, target);
          bit_pop(self->castle, 2);
          bit_pop(self->castle, 3);
        }
      }
    } else {
      int add = (self->side == WHITE) ? 0 : 2;
      switch (Piece->piece) {
      case ROOK: bit_pop(self->castle, (source == h1 ? 0 : 1) + add); break;
      case KING:
        bit_pop(self->castle, 0 + add);
        bit_pop(self->castle, 1 + add);
        break;
      default: break;
      }
    }

    if (!CBoard_square_isAttack(
            self,
            bit_lsb_index(self->pieceBB[KING] & self->colorBB[self->side]),
            !self->side)) {
      self->side = !self->side;
      return 1;
    } else
      return 0;

  } else {
    if (Move_capture(move)) {
      CBoard_move_make(self, move, 0);
      return 1;
    } else
      return 0;
  }

  return 0;
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

void CBoard_print_attacked(CBoard_T self, eColor side) {
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      Square square = (7 - rank) * 8 + file;

      if (!file)
        printf(" %d  ", 8 - rank);

      printf("%d ", CBoard_square_isAttack(self, square, side));
    }
    printf("\n");
  }
  printf("    A B C D E F G H\n");
  printf("\n");
}

void init_all() {
  init_leapers_attacks();
  init_sliders_attacks();
}

int main(void) {
  init_all();

  CBoard_T   board;
  MoveList_T moves;

  board = CBoard_fromFEN(NULL, "r3k2r/pP1pqpb1/bn2pnp1/2pPN3/1p2P3/"
                               "2N2Q1p/PPPBBPpP/R3K2R w KQkq c6 0 1 ");
  moves = CBoard_move_generate(board);

  int start = get_time_ms();
  for (int i = 0; i < moves->count; i++) {
    struct CBoard_T backup = *board;
    if (!CBoard_move_make(board, moves->moves[i], 0)) {
      *board = backup;
      continue;
    }
    CBoard_print(board);

    *board = backup;
    /* CBoard_print(board); */
    /* getc(stdin); */
  }
  printf("Executed in: %dms\n", get_time_ms() - start);

  return 0;
}
