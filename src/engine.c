#include <ctype.h>
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
  eColor color = CBoard_side(cboard);

  if (!moves)
    moves = MoveList_new();

  { // pawn moves
    Piece_T Piece = Piece_get(PAWN, color);
    int     index = Piece_index(Piece);
    U64     bitboard = CBoard_pieceSet(cboard, Piece);
    bitboard_for_each_bit(src, bitboard) {
      { // quiet
        int add = (color == WHITE) ? +8 : -8;
        tgt = src + add;
        if (tgt > a1 && tgt < h8 && !CBoard_square_isOccupied(cboard, tgt)) {
          if (pawn_canPromote(color, src)) {
            pawn_promote(src, tgt, index, 0);
          } else {
            MoveList_add(moves, Move_encode(src, tgt, index, 0, 0, 0, 0, 0));

            // two ahead
            if (pawn_onStart(color, src) &&
                !CBoard_square_isOccupied(cboard, tgt += add))
              MoveList_add(moves, Move_encode(src, tgt, index, 0, 0, 1, 0, 0));
          }
        }
      }
      { // capture
        U64 attack = CBoard_piece_attacks(cboard, Piece, src) &
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
        if (CBoard_enpassant(cboard) != no_sq &&
            CBoard_piece_attacks(cboard, Piece, src) &
                (C64(1) << CBoard_enpassant(cboard)))
          MoveList_add(moves, Move_encode(src, CBoard_enpassant(cboard), index,
                                          0, 1, 0, 1, 0));
      }
    }
  }

  // All piece move
  for (int piece = 1; piece < 6; piece++) {
    Piece_T Piece = Piece_get(piece, color);
    U64     bitboard = CBoard_pieceSet(cboard, Piece);
    bitboard_for_each_bit(src, bitboard) {
      U64 attack = CBoard_piece_attacks(cboard, Piece, src) &
                   ~CBoard_colorBB(cboard, color);
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
        if (!CBoard_square_isOccupied(cboard, f1) &&
            !CBoard_square_isOccupied(cboard, g1) &&
            !CBoard_square_isAttack(cboard, e1, BLACK) &&
            !CBoard_square_isAttack(cboard, f1, BLACK))
          MoveList_add(moves, Move_encode(e1, g1, index, 0, 0, 0, 0, 1));
      }
      if (CBoard_castle(cboard) & WQ) {
        if (!CBoard_square_isOccupied(cboard, d1) &&
            !CBoard_square_isOccupied(cboard, c1) &&
            !CBoard_square_isOccupied(cboard, b1) &&
            !CBoard_square_isAttack(cboard, e1, BLACK) &&
            !CBoard_square_isAttack(cboard, d1, BLACK))
          MoveList_add(moves, Move_encode(e1, c1, index, 0, 0, 0, 0, 1));
      }
    } else {
      int index = Piece_index(Piece_get(KING, BLACK));
      if (CBoard_castle(cboard) & BK) {
        if (!CBoard_square_isOccupied(cboard, f8) &&
            !CBoard_square_isOccupied(cboard, g8) &&
            !CBoard_square_isAttack(cboard, e8, WHITE) &&
            !CBoard_square_isAttack(cboard, f8, WHITE))
          MoveList_add(moves, Move_encode(e8, g8, index, 0, 0, 0, 0, 1));
      }
      if (CBoard_castle(cboard) & BQ) {
        if (!CBoard_square_isOccupied(cboard, d8) &&
            !CBoard_square_isOccupied(cboard, c8) &&
            !CBoard_square_isOccupied(cboard, b8) &&
            !CBoard_square_isAttack(cboard, e8, WHITE) &&
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

int make_move(CBoard_T cboard, Move move, int flag) {
  if (flag == 0) {

    Square  source = Move_source(move);
    Square  target = Move_target(move);
    Piece_T Piece = Piece_fromIndex(Move_piece(move));
    eColor  color = CBoard_side(cboard);

    if (!Move_capture(move))
      CBoard_piece_move(cboard, Piece, source, target);
    else
      CBoard_piece_capture(cboard, Piece, source, target);

    if (Move_promote(move)) {
      Piece_T Promote = Piece_fromIndex(Move_promote(move));
      CBoard_piece_pop(cboard, Piece, target);
      CBoard_piece_set(cboard, Promote, target);
    }

    {
      int ntarget = target + (color == WHITE ? -8 : +8);
      if (Move_enpassant(move))
        CBoard_piece_pop(cboard, Piece_get(PAWN, !color), ntarget);

      CBoard_enpassant_set(cboard, Move_double(move) ? ntarget : no_sq);
    }

    if (Move_castle(move)) {
      Piece_T Rook = Piece_get(ROOK, CBoard_side(cboard));
      switch (target) {
      case g1: CBoard_piece_move(cboard, Rook, h1, f1); break;
      case c1: CBoard_piece_move(cboard, Rook, a1, d1); break;
      case g8: CBoard_piece_move(cboard, Rook, h8, f8); break;
      case c8: CBoard_piece_move(cboard, Rook, a8, d8); break;
      default: break;
      }
    }

    CBoard_castle_and(cboard, castling_rights[source]);
    CBoard_castle_and(cboard, castling_rights[target]);

    if (!CBoard_isCheck(cboard)) {
      CBoard_side_switch(cboard);
      return 1;
    } else
      return 0;
  } else {
    if (Move_capture(move))
      return make_move(cboard, move, 0);
    else
      return 0;
  }
}

long              nodes = 0;
struct MoveList_T moveList[10];

void perft_driver(CBoard_T self, int depth) {
  if (depth == 0) {
    nodes++;
    return;
  }

  MoveList_T moves = generate_moves(self, &moveList[depth]);
  CBoard_T   backup = CBoard_new();

  for (int i = 0; i < moves->count; i++, CBoard_copy(backup, self)) {
    CBoard_copy(self, backup);
    if (!make_move(self, moves->moves[i], 0))
      continue;

    perft_driver(self, depth - 1);
  }
  moveList[depth].count = 0;
}

void perft_test(CBoard_T self, int depth) {
  MoveList_T moves = generate_moves(self, &moveList[depth]);
  CBoard_T   backup = CBoard_new();
  long       start = get_time_ms();

  printf("\n     Performance test\n\n");
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

/* UCI */

struct Score_T {
  int value;
  int score[64];
};

// clang-format off
struct Score_T Scores[] = {
[PAWN] = { 
.value = 100,
.score = {
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0, -10, -10,   0,   0,   0,
             0,   0,   0,   5,   5,   0,   0,   0,
             5,   5,  10,  20,  20,   5,   5,   5,
            10,  10,  10,  20,  20,  10,  10,  10,
            20,  20,  20,  30,  30,  30,  20,  20,
            30,  30,  30,  40,  40,  30,  30,  30,
            90,  90,  90,  90,  90,  90,  90,  90,
         }},
[KNIGHT] = {
.value = 300,
.score = {
            -5,  -10 , 0,   0,   0,   0, -10,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   5,  20,  10,  10,  20,   5,  -5,
            -5,  10,  20,  30,  30,  20,  10,  -5,
            -5,  10,  20,  30,  30,  20,  10,  -5,
            -5,   5,  20,  20,  20,  20,   5,  -5,
            -5,   0,   0,  10,  10,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
         }},
[BISHOP] = {
.value = 350,
.score = {
             0,   0, -10,   0,   0, -10,   0,   0,
             0,  30,   0,   0,   0,   0,  30,   0,
             0,  10,   0,   0,   0,   0,  10,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,   0,  10,  10,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
         }},
[ROOK] = {
.value = 500,
.score = {
             0,   0,   0,  20,  20,   0,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
            50,  50,  50,  50,  50,  50,  50,  50,
            50,  50,  50,  50,  50,  50,  50,  50,
         }},
[QUEEN] = {
.value = 1000,
.score = {
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
         }},
[KING] = {
.value = 10000,
.score = {
             0,   0,   5,   0, -15,   0,  10,   0,
             0,   5,   5,  -5,  -5,   0,   5,   0,
             0,   0,   5,  10,  10,   5,   0,   0,
             0,   5,  10,  20,  20,  10,   5,   0,
             0,   5,  10,  20,  20,  10,   5,   0,
             0,   5,   5,  10,  10,   5,   5,   0,
             0,   0,   5,   5,   5,   5,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
         }},
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

int Score_score(ePiece piece, eColor color, Square square) {
  if (color == BLACK)
    square = mirror_score[square];
  return Scores[piece].score[square];
}

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
        score += Score_score(i, side, square);
      } else {
        score -= Score_value(i);
        score -= Score_score(i, !side, square);
      }
    }
  }

  return side == WHITE ? score : -score;
}

void search_position(CBoard_T cboard, int depth) {
  (void)cboard;
  (void)depth;
  printf("bestmove d7d5\n");
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

Move parse_move(CBoard_T self, char *move_string) {
  Move       result = 0;
  MoveList_T moves;
  Square     source, target;

  source = coordinates_to_square(move_string);
  target = coordinates_to_square(move_string + 2);

  moves = generate_moves(self, NULL);
  for (int i = 0; i < moves->count; i++) {
    Move move = moves->moves[i];
    if (Move_source(move) == source && Move_target(move) == target) {
      if (move_string[4]) {
        Piece_T promoted = Piece_fromIndex(Move_promote(move));
        if (tolower(Piece_code(promoted)) != move_string[4])
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
          make_move(board, move, 0);
        } else {
          printf("Invalid move %s!\n", token);
        }
      }
      CBoard_print(board);
      return board;
    }

    if (strcmp(token, "go") == 0) {
      token = Instruction_token_next(self);
      if (token && strcmp(token, "depth") == 0) {
        token = Instruction_token_next(self);
        search_position(board, atoi(token));
      } else {
        search_position(board, 6);
        printf("Unknown argument after go\n");
      }
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

int main(void) {
  init_all();
  int debug = 1;
  if (debug) {
    printf("debugging!\n");
    CBoard_T board = NULL;
    board = CBoard_fromFEN(board, start_position);
    make_move(board, parse_move(board, "e2e4"), 0);
    make_move(board, parse_move(board, "b7b6"), 0);
    make_move(board, parse_move(board, "d2d4"), 0);
    make_move(board, parse_move(board, "c8b7"), 0);
    CBoard_print(board);
    printf("Evaluation: %d\n", evaluate(board));
  } else
    uci_loop();
  return 0;
}
