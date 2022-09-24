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

Piece_T Piece_fromCode(char code) {
  int color = (isupper(code)) ? WHITE : BLACK;
  for (int i = 0; i < 6; i++)
    if (Pieces[color][i].code == code)
      return &Pieces[color][i];
  return NULL;
}

enum enumCastle { WK = 1, WQ = 2, BK = 4, BQ = 8 };
typedef enum enumCastle eCastle;

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

int square_index_extractor(U64 btbrd) {
  static U64 bitboard;

  if (btbrd != no_sq)
    bitboard = btbrd;

  if (bitboard == C64(0))
    return no_sq;

  int index = bit_lsb_index(bitboard);
  bit_pop(bitboard, index);
  return index;
}

void CBoard_move_generate(CBoard_T self) {
  Square source, target;
  U64    bitboard, attack;

  U64 occupancy = self->colorBB[WHITE] | self->colorBB[BLACK];
  for (int color = 0; color < 2; color++) {
    // Generate quiet pawn moves
    {
      Piece_T Piece = &Pieces[color][PAWN];
      bitboard = self->pieceBB[PAWN] & self->colorBB[color];
      while (bitboard) {
        // push
        {
          int add = (color == WHITE) ? +8 : -8;
          target = source = bit_lsb_index(bitboard);
          target += add;
          if (target > a1 && target < h8 && !bit_get(occupancy, target)) {
            // promote
            if ((color == WHITE && source >= a7 && source <= h7) ||
                (color == BLACK && source >= a2 && source <= h2)) {
              // add move to move list
              printf("PROMOTION!!! ");
            } else {
              // one ahead
              // add move to move list
              printf("SINGLE PUSH!!! ");

              // two ahead
              if (((color == BLACK && source >= a7 && source <= h7) ||
                   (color == WHITE && source >= a2 && source <= h2)) &&
                  !bit_get(occupancy, target + add)) {
                // add to move list;
                printf("DOUBLE PUSH!!! ");
              }
              printf("%s pawn: %s; target: %s\n",
                     (color == WHITE) ? "white" : "black",
                     square_to_coordinates[source],
                     square_to_coordinates[target]);
            }
          }
        }
        // Capture
        {
          attack = Piece->attacks(source, occupancy) & self->colorBB[!color];
          while (attack) {
            target = bit_lsb_index(attack);
            bit_pop(attack, target);
            if ((color == WHITE && source >= a7 && source <= h7) ||
                (color == BLACK && source >= a2 && source <= h2)) {
              // add move to move list
              printf("Capture PROMOTION!!! ");
            } else {
              printf("%s pawn: %s; Capture: %s\n",
                     (color == WHITE) ? "white" : "black",
                     square_to_coordinates[source],
                     square_to_coordinates[target]);
            }
          }
        }
        // enpassant
        {
          if (self->enpassant != no_sq) {
            attack =
                Piece->attacks(source, occupancy) & (C64(1) << self->enpassant);
            if (attack) {
              target = bit_lsb_index(attack);
              printf("%s enpassand %s\n", square_to_coordinates[source],
                     square_to_coordinates[target]);
            }
          }
        }
        bit_pop(bitboard, source);
      }

      for (int piece = 1; piece < 6; piece++) {
        bitboard = self->pieceBB[piece] & self->colorBB[color];
        Piece_T Piece = &Pieces[color][piece];
        while (bitboard) {
          source = bit_lsb_index(bitboard);
          bit_pop(bitboard, source);
          attack = Piece->attacks(source, occupancy) & ~self->colorBB[color];
          while (attack) {
            target = bit_lsb_index(attack);
            if (bit_get(self->colorBB[!color], target))
              printf("%s from %s capture to %s\n", Piece->unicode,
                     square_to_coordinates[source],
                     square_to_coordinates[target]);
            else
              printf("%s from %s move %s\n", Piece->unicode,
                     square_to_coordinates[source],
                     square_to_coordinates[target]);
            bit_pop(attack, target);
          }
        }
      }
    }
  }

  // Castling
  {
    if (!CBoard_square_isAttack(self, e1, BLACK)) {
      if (self->castle & WK && !bit_get(occupancy, f1) &&
          !bit_get(occupancy, g1)) {
        printf("CASTLE WHITE KING SIDE\n");
      }
      if (self->castle & WQ && !bit_get(occupancy, b1) &&
          !bit_get(occupancy, c1) && !bit_get(occupancy, d1)) {
        printf("CASTLE WHITE QUEEN SIDE\n");
      }
    }

    if (!CBoard_square_isAttack(self, e1, WHITE)) {
      if (self->castle & BK && !bit_get(occupancy, f8) &&
          !bit_get(occupancy, g8)) {
        printf("CASTLE BLACK KING SIDE\n");
      }
      if (self->castle & BQ && !bit_get(occupancy, b8) &&
          !bit_get(occupancy, c8) && !bit_get(occupancy, d8)) {
        printf("CASTLE BLACK QUEEN SIDE\n");
      }
    }
  }
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

  CBoard_T board;
  board = CBoard_fromFEN(
      NULL,
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ");

  CBoard_print(board);
  CBoard_move_generate(board);

  FREE(board);
  return 0;
}
