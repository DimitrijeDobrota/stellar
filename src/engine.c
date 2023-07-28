#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CBoard.h"
#include "attack.h"
#include "utils.h"

#include <cul/assert.h>
#include <cul/mem.h>

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

Move Move_encode(Square src, Square tgt, Piece_T Piece, Piece_T Capture,
                 Piece_T Promote, int dbl, int enpassant, int castle) {
  Move move = C32(0);
  move |= (src) | (tgt << 6) | (dbl << 27) | (enpassant << 28) | (castle << 29);
  move |= (Piece_index(Piece) << 12);
  if (Capture != NULL) {
    move |= (Piece_index(Capture) << 17);
    move |= (1 << 30);
  }

  if (Promote != NULL) {
    move |= (Piece_index(Promote) << 22);
    move |= (1 << 31);
  }

  return move;
}

#define Move_source(move)    (((move)&C32(0x0000003f)))
#define Move_target(move)    (((move)&C32(0x00000fc0)) >> 6)
#define Move_double(move)    (((move)&C32(0x08000000)) >> 27)
#define Move_enpassant(move) (((move)&C32(0x10000000)) >> 28)
#define Move_castle(move)    (((move)&C32(0x20000000)) >> 29)
#define Move_capture(move)   (((move)&C32(0x40000000)) >> 30)
#define Move_promote(move)   (((move)&C32(0x80000000)) >> 31)

#define Move_piece(move) (Piece_fromIndex(((move)&C32(0x0001F000)) >> 12))
#define Move_piece_capture(move)                                               \
  (assert(Move_capture(move)), Piece_fromIndex(((move)&C32(0x003E0000)) >> 17))
#define Move_piece_promote(move)                                               \
  (assert(Move_promote(move)), Piece_fromIndex(((move)&C32(0x07C00000)) >> 22))

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

struct Score_T {
  int value;
  int position[64];
  int capture[6];
};

struct Score_T Scores[] = {
[PAWN] = {
.value = 100,
.position = {
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0, -10, -10,   0,   0,   0,
             0,   0,   0,   5,   5,   0,   0,   0,
             5,   5,  10,  20,  20,   5,   5,   5,
            10,  10,  10,  20,  20,  10,  10,  10,
            20,  20,  20,  30,  30,  30,  20,  20,
            30,  30,  30,  40,  40,  30,  30,  30,
            90,  90,  90,  90,  90,  90,  90,  90, },
.capture = {   [PAWN] = 105, [KNIGHT] = 205,
             [BISHOP] = 305,   [ROOK] = 405,
              [QUEEN] = 505,   [KING] = 605} },
[KNIGHT] = {
.value = 300,
.position = {
            -5,  -10 , 0,   0,   0,   0, -10,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   5,  20,  10,  10,  20,   5,  -5,
            -5,  10,  20,  30,  30,  20,  10,  -5,
            -5,  10,  20,  30,  30,  20,  10,  -5,
            -5,   5,  20,  20,  20,  20,   5,  -5,
            -5,   0,   0,  10,  10,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5, },
.capture = {   [PAWN] = 104, [KNIGHT] = 204,
             [BISHOP] = 304,   [ROOK] = 404,
              [QUEEN] = 504,   [KING] = 604} },
[BISHOP] = {
.value = 350,
.position = {
             0,   0, -10,   0,   0, -10,   0,   0,
             0,  30,   0,   0,   0,   0,  30,   0,
             0,  10,   0,   0,   0,   0,  10,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,   0,  10,  10,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0, },
.capture = {   [PAWN] = 103, [KNIGHT] = 203,
             [BISHOP] = 303,   [ROOK] = 403,
              [QUEEN] = 503,   [KING] = 603} },
[ROOK] = {
.value = 500,
.position = {
             0,   0,   0,  20,  20,   0,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
            50,  50,  50,  50,  50,  50,  50,  50,
            50,  50,  50,  50,  50,  50,  50,  50, },
.capture = {   [PAWN] = 102, [KNIGHT] = 202,
             [BISHOP] = 302,   [ROOK] = 402,
              [QUEEN] = 502,   [KING] = 602} },
[QUEEN] = {
.value = 1000,
.position = {
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0, },
.capture = {   [PAWN] = 101, [KNIGHT] = 201,
             [BISHOP] = 301,   [ROOK] = 401,
              [QUEEN] = 501,   [KING] = 601} },
[KING] = {
.value = 10000,
.position = {
             0,   0,   5,   0, -15,   0,  10,   0,
             0,   5,   5,  -5,  -5,   0,   5,   0,
             0,   0,   5,  10,  10,   5,   0,   0,
             0,   5,  10,  20,  20,  10,   5,   0,
             0,   5,  10,  20,  20,  10,   5,   0,
             0,   5,   5,  10,  10,   5,   5,   0,
             0,   0,   5,   5,   5,   5,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0, },
.capture = {   [PAWN] = 100, [KNIGHT] = 200,
             [BISHOP] = 300,   [ROOK] = 400,
              [QUEEN] = 500,   [KING] = 600} },
};

const int mirror_score[128] =
{
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1, no_sq,
};
// clang-format on

int Score_value(ePiece piece) { return Scores[piece].value; }

int Score_position(ePiece piece, eColor color, Square square) {
  if (color == BLACK)
    square = mirror_score[square];
  return Scores[piece].position[square];
}

int Score_capture(ePiece src, ePiece tgt) { return Scores[src].capture[tgt]; }

#define MAX_PLY 64

typedef struct Stats_T *Stats_T;
struct Stats_T {
  long nodes;
  int  ply;
  int  pv_length[MAX_PLY];
  Move pv_table[MAX_PLY][MAX_PLY];
  Move killer_moves[2][MAX_PLY];
  Move history_moves[16][64];
};

Stats_T Stats_new(void) {
  Stats_T p;
  NEW0(p);
  return p;
}

void Stats_free(Stats_T *p) { FREE(*p); }

int Move_score(Stats_T stats, Move move) {
  if (Move_capture(move)) {
    return Score_capture(Piece_piece(Move_piece(move)),
                         Piece_piece(Move_piece_capture(move)));
  } else {
    if (stats->killer_moves[0][stats->ply] == move)
      return 9000;
    else if (stats->killer_moves[1][stats->ply] == move)
      return 8000;
    else
      return stats
          ->history_moves[Piece_index(Move_piece(move))][Move_target(move)];
  }

  return 0;
}

void Move_print(Move move) {
  printf("%5s %5s  %2s  %2s   %2s %4d %4d %4d %4d %4d\n",
         square_to_coordinates[Move_source(move)],
         square_to_coordinates[Move_target(move)],
         Piece_unicode(Move_piece(move)),
         Move_capture(move) ? Piece_unicode(Move_piece_capture(move)) : "X ",
         Move_promote(move) ? Piece_unicode(Move_piece_promote(move)) : "X ",
         Move_double(move) ? 1 : 0, Move_enpassant(move) ? 1 : 0,
         Move_castle(move) ? 1 : 0, Move_capture(move) ? 1 : 0,
         Move_promote(move) ? 1 : 0);
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

void MoveList_free(MoveList_T *p) { FREE(*p); }

Move MoveList_move(MoveList_T self, int index) { return self->moves[index]; }
int  MoveList_size(MoveList_T self) { return self->count; }
void MoveList_reset(MoveList_T self) { self->count = 0; }

void MoveList_add(MoveList_T self, Move move) {
  self->moves[self->count++] = move;
}

void MoveList_print(MoveList_T self) {
  printf(" From    To  Pi  Cap  Prmt  Dbl  Enp  Cst  C   P\n");
  for (int i = 0; i < self->count; i++)
    Move_print(self->moves[i]);
  printf("Total: %d\n", self->count);
}

void MoveList_sort(Stats_T stats, MoveList_T list) {
  int score[list->count];
  for (int i = 0; i < list->count; i++)
    score[i] = Move_score(stats, list->moves[i]);

  for (int i = 0; i < list->count; i++)
    for (int j = i + 1; j < list->count; j++)
      if (score[i] < score[j]) {
        Move t = list->moves[i];
        list->moves[i] = list->moves[j];
        list->moves[j] = t;

        int s = score[i];
        score[i] = score[j];
        score[j] = s;
      }
}

#define pawn_canPromote(color, source)                                         \
  ((color == WHITE && source >= a7 && source <= h7) ||                         \
   (color == BLACK && source >= a2 && source <= h2))

#define pawn_onStart(color, source)                                            \
  ((color == BLACK && source >= a7 && source <= h7) ||                         \
   (color == WHITE && source >= a2 && source <= h2))

#define pawn_promote(source, target, Piece, Capture)                           \
  for (int i = 1; i < 5; i++) {                                                \
    move = Move_encode(source, target, Piece, Capture, Piece_get(i, color), 0, \
                       0, 0);                                                  \
    MoveList_add(moves, move);                                                 \
  }

MoveList_T MoveList_generate(MoveList_T moves, CBoard_T board) {
  Move   move;
  Square src, tgt;
  eColor color = CBoard_side(board);

  if (!moves)
    moves = MoveList_new();

  { // pawn moves
    Piece_T Piece = Piece_get(PAWN, color);
    U64     bitboard = CBoard_pieceSet(board, Piece);
    bitboard_for_each_bit(src, bitboard) {
      { // quiet
        int add = (color == WHITE) ? +8 : -8;
        tgt = src + add;
        if (tgt > a1 && tgt < h8 && !CBoard_square_isOccupied(board, tgt)) {
          if (pawn_canPromote(color, src)) {
            pawn_promote(src, tgt, Piece, 0);
          } else {
            MoveList_add(moves, Move_encode(src, tgt, Piece, 0, 0, 0, 0, 0));

            // two ahead
            if (pawn_onStart(color, src) &&
                !CBoard_square_isOccupied(board, tgt += add))
              MoveList_add(moves, Move_encode(src, tgt, Piece, 0, 0, 1, 0, 0));
          }
        }
      }
      { // capture
        U64 attack = CBoard_piece_attacks(board, Piece, src) &
                     CBoard_colorBB(board, !color);
        bitboard_for_each_bit(tgt, attack) {
          if (pawn_canPromote(color, src)) {
            pawn_promote(src, tgt, Piece,
                         CBoard_square_piece(board, tgt, !color));
          } else {
            MoveList_add(moves,
                         Move_encode(src, tgt, Piece,
                                     CBoard_square_piece(board, tgt, !color), 0,
                                     0, 0, 0));
          }
        }
      }

      { // en passant
        if (CBoard_enpassant(board) != no_sq &&
            CBoard_piece_attacks(board, Piece, src) &
                (C64(1) << CBoard_enpassant(board)))
          MoveList_add(moves,
                       Move_encode(src, CBoard_enpassant(board), Piece,
                                   CBoard_square_piece(board, tgt, !color), 0,
                                   0, 1, 0));
      }
    }
  }

  // All piece move
  for (int piece = 1; piece < 6; piece++) {
    Piece_T Piece = Piece_get(piece, color);
    U64     bitboard = CBoard_pieceSet(board, Piece);
    bitboard_for_each_bit(src, bitboard) {
      U64 attack = CBoard_piece_attacks(board, Piece, src) &
                   ~CBoard_colorBB(board, color);
      bitboard_for_each_bit(tgt, attack) {
        /* int take = bit_get(CBoard_colorBB(board, !color), tgt); */
        MoveList_add(moves, Move_encode(src, tgt, Piece,
                                        CBoard_square_piece(board, tgt, !color),
                                        0, 0, 0, 0));
      }
    }
  }

  // Castling
  {
    if (color == WHITE) {
      Piece_T Piece = Piece_get(KING, WHITE);
      if (CBoard_castle(board) & WK) {
        if (!CBoard_square_isOccupied(board, f1) &&
            !CBoard_square_isOccupied(board, g1) &&
            !CBoard_square_isAttack(board, e1, BLACK) &&
            !CBoard_square_isAttack(board, f1, BLACK))
          MoveList_add(moves, Move_encode(e1, g1, Piece, 0, 0, 0, 0, 1));
      }
      if (CBoard_castle(board) & WQ) {
        if (!CBoard_square_isOccupied(board, d1) &&
            !CBoard_square_isOccupied(board, c1) &&
            !CBoard_square_isOccupied(board, b1) &&
            !CBoard_square_isAttack(board, e1, BLACK) &&
            !CBoard_square_isAttack(board, d1, BLACK))
          MoveList_add(moves, Move_encode(e1, c1, Piece, 0, 0, 0, 0, 1));
      }
    } else {
      Piece_T Piece = Piece_get(KING, BLACK);
      if (CBoard_castle(board) & BK) {
        if (!CBoard_square_isOccupied(board, f8) &&
            !CBoard_square_isOccupied(board, g8) &&
            !CBoard_square_isAttack(board, e8, WHITE) &&
            !CBoard_square_isAttack(board, f8, WHITE))
          MoveList_add(moves, Move_encode(e8, g8, Piece, 0, 0, 0, 0, 1));
      }
      if (CBoard_castle(board) & BQ) {
        if (!CBoard_square_isOccupied(board, d8) &&
            !CBoard_square_isOccupied(board, c8) &&
            !CBoard_square_isOccupied(board, b8) &&
            !CBoard_square_isAttack(board, e8, WHITE) &&
            !CBoard_square_isAttack(board, d8, WHITE))
          MoveList_add(moves, Move_encode(e8, c8, Piece, 0, 0, 0, 0, 1));
      }
    }
  }

  return moves;
}

int Move_make(Move move, CBoard_T board, int flag) {
  if (flag == 0) {

    Square  source = Move_source(move);
    Square  target = Move_target(move);
    Piece_T Piece = Move_piece(move);
    eColor  color = CBoard_side(board);

    if (!Move_capture(move))
      CBoard_piece_move(board, Piece, source, target);
    else
      CBoard_piece_capture(board, Piece, Move_piece_capture(move), source,
                           target);

    if (Move_promote(move)) {
      CBoard_piece_pop(board, Piece, target);
      CBoard_piece_set(board, Move_piece_promote(move), target);
    }

    {
      int ntarget = target + (color == WHITE ? -8 : +8);
      if (Move_enpassant(move))
        CBoard_piece_pop(board, Piece_get(PAWN, !color), ntarget);

      CBoard_enpassant_set(board, Move_double(move) ? ntarget : no_sq);
    }

    if (Move_castle(move)) {
      Piece_T Rook = Piece_get(ROOK, CBoard_side(board));
      switch (target) {
      case g1: CBoard_piece_move(board, Rook, h1, f1); break;
      case c1: CBoard_piece_move(board, Rook, a1, d1); break;
      case g8: CBoard_piece_move(board, Rook, h8, f8); break;
      case c8: CBoard_piece_move(board, Rook, a8, d8); break;
      default: break;
      }
    }

    CBoard_castle_and(board, castling_rights[source]);
    CBoard_castle_and(board, castling_rights[target]);

    if (!CBoard_isCheck(board)) {
      CBoard_side_switch(board);
      return 1;
    } else
      return 0;
  } else {
    if (Move_capture(move))
      return Move_make(move, board, 0);
    else
      return 0;
  }
}

/* SEARCHING */

int evaluate(CBoard_T board) {
  Square square;
  eColor side = CBoard_side(board);
  U64    occupancy = CBoard_colorBB(board, side);

  int score = 0;
  for (int i = 0; i < 6; i++) {
    U64 bitboard = CBoard_pieceBB(board, i);
    bitboard_for_each_bit(square, bitboard) {
      if (bit_get(occupancy, square)) {
        score += Score_value(i);
        score += Score_position(i, side, square);
      } else {
        score -= Score_value(i);
        score -= Score_position(i, !side, square);
      }
    }
  }

  return score;
}

int quiescence(Stats_T stats, CBoard_T board, int alpha, int beta) {
  MoveList_T moves;
  CBoard_T   copy;

  int eval = evaluate(board);
  stats->nodes++;

  if (eval >= beta) {
    return beta;
  }

  if (eval > alpha) {
    alpha = eval;
  }

  copy = CBoard_new();
  moves = MoveList_generate(NULL, board);
  MoveList_sort(stats, moves);

  for (int i = 0; i < MoveList_size(moves); i++) {
    CBoard_copy(board, copy);

    if (Move_make(MoveList_move(moves, i), copy, 1) == 0)
      continue;

    stats->ply++;
    int score = -quiescence(stats, copy, -beta, -alpha);
    stats->ply--;

    if (score >= beta) {
      MoveList_free(&moves);
      CBoard_free(&copy);
      return beta;
    }

    if (score > alpha) {
      alpha = score;
    }
  }

  MoveList_free(&moves);
  CBoard_free(&copy);
  return alpha;
}

int negamax(Stats_T stats, CBoard_T board, int alpha, int beta, int depth) {
  MoveList_T list;
  CBoard_T   copy;
  int        isCheck = 0;
  int        ply = stats->ply;

  stats->pv_length[ply] = ply;

  if (depth == 0)
    return quiescence(stats, board, alpha, beta);

  if (ply > MAX_PLY - 1)
    return evaluate(board);

  stats->nodes++;

  copy = CBoard_new();
  list = MoveList_generate(NULL, board);
  isCheck = CBoard_isCheck(board);

  if (isCheck)
    depth++;

  int legal_moves = 0;
  MoveList_sort(stats, list);
  for (int i = 0; i < MoveList_size(list); i++) {
    Move move = MoveList_move(list, i);

    CBoard_copy(board, copy);
    if (Move_make(move, copy, 0) == 0) {
      continue;
    }

    stats->ply++;
    int score = -negamax(stats, copy, -beta, -alpha, depth - 1);
    stats->ply--;
    legal_moves++;

    if (score >= beta) {
      if (!Move_capture(move)) {
        stats->killer_moves[1][ply] = stats->killer_moves[0][ply];
        stats->killer_moves[0][ply] = move;
      }
      MoveList_free(&list);
      CBoard_free(&copy);
      return beta;
    }

    if (score > alpha) {
      if (!Move_capture(move))
        stats
            ->history_moves[Piece_index(Move_piece(move))][Move_target(move)] +=
            depth;
      alpha = score;

      stats->pv_table[ply][ply] = move;
      for (int i = stats->ply + 1; i < stats->pv_length[ply + 1]; i++)
        stats->pv_table[ply][i] = stats->pv_table[ply + 1][i];
      stats->pv_length[ply] = stats->pv_length[ply + 1];
    }
  }

  if (legal_moves == 0) {
    if (isCheck)
      return -49000 + stats->ply;
    else
      return 0;
  }

  MoveList_free(&list);
  CBoard_free(&copy);
  return alpha;
}

/* UCI */

void Move_print_UCI(Move move) {
  printf("%s%s", square_to_coordinates[Move_source(move)],
         square_to_coordinates[Move_target(move)]);
  if (Move_promote(move))
    printf(" %c", Piece_asci(Move_piece_promote(move)));
}

void search_position(CBoard_T board, int depth) {
  Stats_T stats = Stats_new();

  for (int crnt = 1; crnt <= depth; crnt++) {
    int score = negamax(stats, board, -50000, 50000, crnt);

    printf("info score cp %d depth %d nodes %ld pv ", score, crnt,
           stats->nodes);

    for (int i = 0; i < stats->pv_length[0]; i++) {
      Move_print_UCI(stats->pv_table[0][i]);
      printf(" ");
    }
    printf("\n");
  }

  printf("bestmove ");
  Move_print_UCI(stats->pv_table[0][0]);
  printf("\n");

  Stats_free(&stats);
}

void print_info(void) {
  printf("id name chessTrainer\n");
  printf("id author Dimitrije Dobrota\n");
  printf("uciok\n");
}

typedef struct Instruction_T *Instruction_T;
struct Instruction_T {
  char *command;
  char *token;
  char *crnt;
};

char *Instruction_token_next(Instruction_T self);

Instruction_T Instruction_new(char *command) {
  Instruction_T p;
  NEW0(p);
  p->command = ALLOC(strlen(command) + 1);
  p->token = ALLOC(strlen(command) + 1);
  strcpy(p->command, command);
  p->crnt = command;
  Instruction_token_next(p);
  return p;
}

void Instruction_free(Instruction_T *p) {
  FREE((*p)->command);
  FREE((*p)->token);
  FREE(*p);
}

char *Instruction_token(Instruction_T self) { return self->token; }
char *Instruction_token_n(Instruction_T self, int n) {
  while (isspace(*self->crnt) && *self->crnt != '\0')
    self->crnt++;

  if (*self->crnt == '\0') {
    *self->token = '\0';
    return NULL;
  }

  char *p = self->token;
  while (n--) {
    while (!isspace(*self->crnt) && *self->crnt != '\0' && *self->crnt != ';')
      *p++ = *self->crnt++;
    if (*self->crnt == '\0') {
      p++;
      break;
    }
    self->crnt++;
    *p++ = ' ';
  }
  *--p = '\0';

  return self->token;
}

char *Instruction_token_next(Instruction_T self) {
  return Instruction_token_n(self, 1);
}

Move parse_move(CBoard_T board, char *move_string) {
  Move       result = 0;
  MoveList_T moves;
  Square     source, target;

  source = coordinates_to_square(move_string);
  target = coordinates_to_square(move_string + 2);

  moves = MoveList_generate(NULL, board);
  for (int i = 0; i < moves->count; i++) {
    Move move = moves->moves[i];
    if (Move_source(move) == source && Move_target(move) == target) {
      if (move_string[4]) {
        if (tolower(Piece_code(Move_piece_promote(move))) != move_string[4])
          continue;
      }
      result = move;
      break;
    }
  }

  MoveList_free(&moves);
  return result;
}

CBoard_T Instruction_parse(Instruction_T self, CBoard_T board) {
  char *token = Instruction_token(self);

  if (!board)
    board = CBoard_new();

  do {
    if (strcmp(token, "ucinewgame") == 0) {
      board = CBoard_fromFEN(board, start_position);
      continue;
    }

    if (strcmp(token, "quit") == 0)
      return NULL;

    if (strcmp(token, "position") == 0) {
      token = Instruction_token_next(self);
      if (token && strcmp(token, "startpos") == 0) {
        board = CBoard_fromFEN(board, start_position);
      } else if (token && strcmp(token, "fen") == 0) {
        token = Instruction_token_n(self, 6);
        board = CBoard_fromFEN(board, token);
      } else {
        printf("Unknown argument after position\n");
      }
      CBoard_print(board);
      continue;
    }

    if (strcmp(token, "moves") == 0) {
      while ((token = Instruction_token_next(self))) {
        Move move = parse_move(board, token);
        if (move) {
          Move_make(move, board, 0);
        } else {
          printf("Invalid move %s!\n", token);
        }
      }
      CBoard_print(board);
      return board;
    }

    if (strcmp(token, "go") == 0) {
      token = Instruction_token_next(self);
      int depth = 6;
      if (token && strcmp(token, "depth") == 0) {
        token = Instruction_token_next(self);
        depth = atoi(token);
      } else {
        printf("Unknown argument after go\n");
      }
      search_position(board, depth);
      continue;
    }

    if (strcmp(token, "isready") == 0) {
      printf("readyok\n");
      continue;
    }

    if (strcmp(token, "uci") == 0) {
      print_info();
      continue;
    }

  } while ((token = Instruction_token_next(self)));

  return board;
}

void uci_loop(void) {
  CBoard_T      board = NULL;
  Instruction_T instruction;
  char          input[2000];

  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  print_info();
  while (1) {
    memset(input, 0, sizeof(input));
    fflush(stdout);
    if (!fgets(input, sizeof(input), stdin))
      continue;

    instruction = Instruction_new(input);
    if (!(board = Instruction_parse(instruction, board)))
      break;
    Instruction_free(&instruction);
  }

  Instruction_free(&instruction);
  CBoard_free(&board);
}

/* PERFT */

struct MoveList_T moveList[10];
long              nodes;

void perft_driver(CBoard_T board, struct MoveList_T *moveList, int depth, unsigned long long *nodes) {
  if (depth == 0) {
    (*nodes)++;
    return;
  }

  MoveList_T list = MoveList_generate(&moveList[depth], board);
  CBoard_T   copy = CBoard_new();

  for (int i = 0; i < MoveList_size(list); i++) {
    CBoard_copy(board, copy);
    if (!Move_make(MoveList_move(list, i), copy, 0))
      continue;
    perft_driver(copy, moveList, depth - 1, nodes);
  }

  MoveList_reset(list);
  CBoard_free(&copy);
}

void perft_test(CBoard_T board, int depth) {
  MoveList_T list = MoveList_generate(&moveList[depth], board);
  CBoard_T   copy = CBoard_new();
  long       start = get_time_ms();

  printf("\n     Performance test\n\n");

  nodes = 0;
  for (int i = 0; i < MoveList_size(list); i++) {
    CBoard_copy(board, copy);
    Move move = MoveList_move(list, i);
    if (!Move_make(MoveList_move(list, i), copy, 0)) continue;
    unsigned long long node = 0;
    perft_driver(copy, moveList, depth - 1, &node);
    printf("%s%s: %llu\n", square_to_coordinates[Move_source(move)],
           square_to_coordinates[Move_target(move)], node);
    nodes+=node;
  }
  MoveList_reset(list);
  CBoard_free(&copy);

  printf("\nNodes searched: %ld\n\n", nodes);
  printf("\n    Depth: %d\n", depth);
  printf("    Nodes: %ld\n", nodes);
  printf("     Time: %ld\n\n", get_time_ms() - start);
}

 #include <semaphore.h>
 #include <pthread.h>

typedef struct perf_shared perf_shared;
struct perf_shared {
  CBoard_T board;
  MoveList_T list;
  int depth;
  sem_t* mutex;
  int* index;
  sem_t* finish;
  unsigned long long *total;
};

void* perft_thread(void *arg) {
  perf_shared *shared = (perf_shared *)arg;
  CBoard_T board = CBoard_new();
  unsigned long long node_count = 0;

  struct MoveList_T moveList[10];

  while(1) {
    sem_wait(shared->mutex);
    *shared->total += node_count;
    if(*shared->index >= MoveList_size(shared->list)) {
        sem_post(shared->mutex);
        break;
    }
    Move move = MoveList_move(shared->list, (*shared->index)++);
    sem_post(shared->mutex);

    CBoard_copy(shared->board, board);
    if(!Move_make(move, board, 0)) continue;

    node_count = 0;
    perft_driver(board, moveList, shared->depth, &node_count);
    printf("%s%s: %llu\n", square_to_coordinates[Move_source(move)],
           square_to_coordinates[Move_target(move)], node_count);
  }
  CBoard_free(&board);
  sem_post(shared->finish);
  return NULL;
}

void perft_test_threaded(CBoard_T board, int depth) {
  MoveList_T list = MoveList_generate(NULL, board);
  int size = 8;

  unsigned long long total = 0;
  perf_shared shared[size];
  pthread_t threads[size];
  sem_t mutex, finish;


  int index = 0;
  sem_init(&mutex, 0, 1);
  sem_init(&finish, 0, 0);
  for(int i=0; i<size; i++) {
    shared[i].board = board;
    shared[i].list = list;
    shared[i].depth = depth - 1;
    shared[i].mutex = &mutex;
    shared[i].index = &index;
    shared[i].finish = &finish;
    shared[i].total = &total;
    pthread_create(threads + i, NULL, perft_thread, (void *)(shared + i));
  }

  for(int i=0; i<size; i++) sem_wait(&finish);
  MoveList_free(&list);

  printf("Nodes processed: %llu\n", total);
}

/* MAIN */

void init_all() {
  init_leapers_attacks();
  init_sliders_attacks();
}


int main(void) {
  init_all();

  CBoard_T      board = NULL;
  board = CBoard_fromFEN(board, start_position);
  CBoard_print(board);
  //perft_test(board, 7);
  perft_test_threaded(board, 7);
  CBoard_free(&board);
  return 0;
}
