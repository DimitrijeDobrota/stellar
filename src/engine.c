#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CBoard.h"
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
#define Move_double(move)    (((move)&C32(0x200000)) >> 21)
#define Move_enpassant(move) (((move)&C32(0x400000)) >> 22)
#define Move_castle(move)    (((move)&C32(0x800000)) >> 23)

void Move_print(Move move) {
  int promote = Move_promote(move);
  printf("%5s %5s %5s %5c %4d %4d %4d %4d\n",
         square_to_coordinates[Move_source(move)],
         square_to_coordinates[Move_target(move)],
         Piece_unicode(Piece_fromIndex(Move_piece(move))),
         promote ? Piece_asci(Piece_fromIndex(promote)) : 'X',
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

#define pawn_canPromote(color, source)                                         \
  ((color == WHITE && source >= a7 && source <= h7) ||                         \
   (color == BLACK && source >= a2 && source <= h2))

#define pawn_onStart(color, source)                                            \
  ((color == BLACK && source >= a7 && source <= h7) ||                         \
   (color == WHITE && source >= a2 && source <= h2))

#define pawn_promote(source, target, index, capture)                           \
  for (int i = 1; i < 5; i++) {                                                \
    move = Move_encode(source, target, index,                                  \
                       Piece_index(Piece_get(i, color)), capture, 0, 0, 0);    \
    MoveList_add(moves, move);                                                 \
  }

MoveList_T generate_moves(CBoard_T cboard, MoveList_T moves) {
  Move   move;
  Square src, tgt;
  U64 occupancy = CBoard_colorBB(cboard, WHITE) | CBoard_colorBB(cboard, BLACK);
  eColor color = CBoard_side(cboard);

  if (!moves)
    moves = MoveList_new();

  { // pawn moves
    Piece_T Piece = Piece_get(PAWN, color);
    int     index = Piece_index(Piece);
    U64     bitboard = CBoard_getPieceSet(cboard, Piece);
    bitboard_for_each_bit(src, bitboard) {
      { // quiet
        int add = (color == WHITE) ? +8 : -8;
        tgt = src + add;
        if (tgt > a1 && tgt < h8 && !bit_get(occupancy, tgt)) {
          if (pawn_canPromote(color, src)) {
            pawn_promote(src, tgt, index, 0);
          } else {
            MoveList_add(moves, Move_encode(src, tgt, index, 0, 0, 0, 0, 0));

            // two ahead
            if (pawn_onStart(color, src) && !bit_get(occupancy, tgt += add))
              MoveList_add(moves, Move_encode(src, tgt, index, 0, 0, 1, 0, 0));
          }
        }
      }
      { // capture
        U64 attack = Piece_attacks(Piece)(src, occupancy) &
                     CBoard_colorBB(cboard, !color);
        bitboard_for_each_bit(tgt, attack) {
          if (pawn_canPromote(color, src)) {
            pawn_promote(src, tgt, index, 1);
          } else {
            MoveList_add(moves, Move_encode(src, tgt, index, 0, 1, 0, 0, 0));
          }
        }
      }

      { // en passant
        U64 attack = Piece_attacks(Piece)(src, occupancy) &
                     (C64(1) << CBoard_enpassant(cboard));
        if (CBoard_enpassant(cboard) != no_sq && attack)
          MoveList_add(moves, Move_encode(src, bit_lsb_index(attack), index, 0,
                                          1, 0, 1, 0));
      }
    }
  }

  // All piece move
  for (int piece = 1; piece < 6; piece++) {
    Piece_T Piece = Piece_get(piece, color);
    U64     bitboard = CBoard_getPieceSet(cboard, Piece);
    bitboard_for_each_bit(src, bitboard) {
      U64 attack =
          Piece_attacks(Piece)(src, occupancy) & ~CBoard_colorBB(cboard, color);
      bitboard_for_each_bit(tgt, attack) {
        int take = bit_get(CBoard_colorBB(cboard, !color), tgt);
        MoveList_add(
            moves, Move_encode(src, tgt, Piece_index(Piece), 0, take, 0, 0, 0));
      }
    }
  }

  // Castling
  {
    if (color == WHITE) {
      int index = Piece_index(Piece_get(KING, WHITE));
      if (CBoard_castle(cboard) & WK) {
        if (!bit_get(occupancy, f1) && !bit_get(occupancy, g1))
          if (!CBoard_square_isAttack(cboard, e1, BLACK) &&
              !CBoard_square_isAttack(cboard, f1, BLACK))
            MoveList_add(moves, Move_encode(e1, g1, index, 0, 0, 0, 0, 1));
      }
      if (CBoard_castle(cboard) & WQ) {
        if (!bit_get(occupancy, d1) && !bit_get(occupancy, c1) &&
            !bit_get(occupancy, b1))
          if (!CBoard_square_isAttack(cboard, e1, BLACK) &&
              !CBoard_square_isAttack(cboard, d1, BLACK))
            MoveList_add(moves, Move_encode(e1, c1, index, 0, 0, 0, 0, 1));
      }
    } else {
      int index = Piece_index(Piece_get(KING, BLACK));
      if (CBoard_castle(cboard) & BK) {
        if (!bit_get(occupancy, f8) && !bit_get(occupancy, g8))
          if (!CBoard_square_isAttack(cboard, e8, WHITE) &&
              !CBoard_square_isAttack(cboard, f8, WHITE))
            MoveList_add(moves, Move_encode(e8, g8, index, 0, 0, 0, 0, 1));
      }
      if (CBoard_castle(cboard) & BQ) {
        if (!bit_get(occupancy, d8) && !bit_get(occupancy, c8) &&
            !bit_get(occupancy, b8))
          if (!CBoard_square_isAttack(cboard, e8, WHITE) &&
              !CBoard_square_isAttack(cboard, d8, WHITE))
            MoveList_add(moves, Move_encode(e8, c8, index, 0, 0, 0, 0, 1));
      }
    }
  }

  return moves;
}

// clang-format off
const int castling_rights[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7,  15, 15, 15, 3,  15, 15, 11,
};
// clang-format on

int make_move(CBoard_T self, Move move, int flag) {
  if (flag == 0) {

    Square  source = Move_source(move);
    Square  target = Move_target(move);
    Piece_T Piece = Piece_fromIndex(Move_piece(move));

    CBoard_piece_move(self, Piece, source, target);

    if (Move_capture(move)) {
      CBoard_colorBB_pop(self, !Piece_color(Piece), target);
      ePiece i;
      for (i = 0; i < 6; i++) {
        if (i != Piece_piece(Piece) && CBoard_pieceBB_get(self, i, target)) {
          CBoard_pieceBB_pop(self, i, target);
          break;
        }
      }
    }

    if (Move_promote(move)) {
      Piece_T Promote = Piece_fromIndex(Move_promote(move));
      CBoard_pieceBB_pop(self, Piece_piece(Piece), target);
      CBoard_pieceBB_set(self, Piece_piece(Promote), target);
    }

    if (Move_enpassant(move)) {
      if (Piece_color(Piece) == WHITE)
        CBoard_piece_pop(self, Piece_get(PAWN, !Piece_color(Piece)),
                         target - 8);
      else
        CBoard_piece_pop(self, Piece_get(PAWN, !Piece_color(Piece)),
                         target + 8);
    }

    if (Move_double(move))
      CBoard_enpassant_set(self,
                           target + (Piece_color(Piece) == WHITE ? -8 : +8));
    else
      CBoard_enpassant_set(self, no_sq);

    if (Move_castle(move)) {
      if (CBoard_side(self) == WHITE) {
        Piece_T Rook = Piece_get(ROOK, WHITE);
        if (target == g1)
          CBoard_piece_move(self, Rook, h1, f1);
        else
          CBoard_piece_move(self, Rook, a1, d1);
        CBoard_castle_pop(self, WK);
        CBoard_castle_pop(self, WQ);
      } else {
        Piece_T Rook = Piece_get(ROOK, BLACK);
        if (target == g8)
          CBoard_piece_move(self, Rook, h8, f8);
        else
          CBoard_piece_move(self, Rook, a8, d8);
        CBoard_castle_pop(self, BK);
        CBoard_castle_pop(self, BQ);
      }
    }

    CBoard_castle_and(self, castling_rights[source]);
    CBoard_castle_and(self, castling_rights[target]);

    if (!CBoard_isCheck(self)) {
      CBoard_side_switch(self);
      return 1;
    } else
      return 0;
  } else {
    if (Move_capture(move)) {
      make_move(self, move, 0);
      return 1;
    } else
      return 0;
  }

  return 0;
}

long              nodes = 0;
struct MoveList_T moveList[10];

void perft_driver(CBoard_T self, int depth) {
  if (depth == 0) {
    nodes++;
    return;
  }

  CBoard_T   backup = CBoard_new();
  MoveList_T moves;

  moves = generate_moves(self, &moveList[depth]);

  for (int i = 0; i < moves->count; i++, CBoard_copy(backup, self)) {
    CBoard_copy(self, backup);
    if (!make_move(self, moves->moves[i], 0))
      continue;

    perft_driver(self, depth - 1);
  }
  moveList[depth].count = 0;
}

void perft_test(CBoard_T self, int depth) {
  CBoard_T backup = CBoard_new();

  printf("\n     Performance test\n\n");

  MoveList_T moves;
  moves = generate_moves(self, &moveList[depth]);
  long start = get_time_ms();

  for (int i = 0; i < moves->count; i++, CBoard_copy(backup, self)) {
    CBoard_copy(self, backup);
    if (!make_move(self, moves->moves[i], 0))
      continue;
    long cummulative_nodes = nodes;
    perft_driver(self, depth - 1);
    long old_nodes = nodes - cummulative_nodes;
    printf("%s%s: %ld\n", square_to_coordinates[Move_source(moves->moves[i])],
           square_to_coordinates[Move_target(moves->moves[i])], old_nodes);
  }

  moveList[depth].count = 0;

  // print results
  printf("\nNodes searched: %ld\n\n", nodes);
  return;
  printf("\n    Depth: %d\n", depth);
  printf("    Nodes: %ld\n", nodes);
  printf("     Time: %ld\n\n", get_time_ms() - start);
}

void init_all() {
  init_leapers_attacks();
  init_sliders_attacks();
}

int main(void) {
  init_all();

  CBoard_T board = CBoard_fromFEN(NULL, tricky_position);
  perft_test(board, 4);

  return 0;
}
