#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cii/assert.h>
#include <cii/except.h>
#include <cii/mem.h>

/* DEFINITIONS */

// useful macros
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

// define number types
typedef unsigned long long U64;           // define bitboard data type
#define C64(constantU64) constantU64##ULL // define shorthand for constants

typedef unsigned int U32;
#define C32(constantU32) constantU32##U

// useful bit patterns
const U64 universe = C64(0xffffffffffffffff); //
const U64 notAFile = C64(0xfefefefefefefefe); // ~0x0101010101010101
const U64 notHFile = C64(0x7f7f7f7f7f7f7f7f); // ~0x8080808080808080

// useful bit operations
#define bit_get(bitboard, square) (((bitboard) >> (square)) & C64(1))
#define bit_set(bitboard, square) ((bitboard) |= C64(1) << (square))
#define bit_pop(bitboard, square) ((bitboard) &= ~(C64(1) << (square)))

static inline int bit_count(U64 bitboard) {
  int count = 0;

  while (bitboard > 0) {
    count++;
    bitboard &= bitboard - 1;
  }

  return count;
}

static inline int bit_lsb_index(U64 bitboard) {
  if (!bitboard)
    return -1;

  return bit_count((bitboard & -bitboard) - 1);
}

// pseudo random numbers

U32 state = C32(1804289383);

U32 get_random_U32_number() {
  U32 number = state;

  number ^= number << 13;
  number ^= number >> 17;
  number ^= number << 5;

  return state = number;
}

U64 get_random_U64_number() {
  U64 n1, n2, n3, n4;

  n1 = (U64)(get_random_U32_number()) & C64(0xFFFF);
  n2 = (U64)(get_random_U32_number()) & C64(0xFFFF);
  n3 = (U64)(get_random_U32_number()) & C64(0xFFFF);
  n4 = (U64)(get_random_U32_number()) & C64(0xFFFF);

  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 generate_magic_number() {
  return get_random_U64_number() & get_random_U64_number() &
         get_random_U64_number();
}

// squares
// clang-format off
enum enumSquare {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8
};

const char *square_to_coordinates[]={
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};
// clang-format on

// board moving
typedef U64 (*direction_f)(U64);
U64 soutOne(U64 b) { return b >> 8; }
U64 nortOne(U64 b) { return b << 8; }
U64 eastOne(U64 b) { return (b & notHFile) << 1; }
U64 westOne(U64 b) { return (b & notAFile) >> 1; }
U64 soEaOne(U64 b) { return (b & notHFile) >> 7; }
U64 soWeOne(U64 b) { return (b & notAFile) >> 9; }
U64 noEaOne(U64 b) { return (b & notHFile) << 9; }
U64 noWeOne(U64 b) { return (b & notAFile) << 7; }

// board rotation
U64 rotateLeft(U64 x, int s) { return (x << s) | (x >> (64 - s)); }
U64 rotateRight(U64 x, int s) { return (x >> s) | (x << (64 - s)); }

enum enumColor { WHITE = 0, BLACK };
enum enumPiece { PAWN = 2, KNIGHT, BISHOP, ROOK, QUEEN, KIND };

typedef enum enumColor Color;
typedef enum enumPiece Piece;

// piece representation
typedef struct PieceType *PieceType;
struct PieceType {
  Piece piece;
  Color color;
};

int pieceCode(PieceType pt) { return pt->piece; }
int colorCode(PieceType pt) { return pt->color; }

// board representation
typedef struct CBoard *CBoard;
struct CBoard {
  U64 pieceBB[8];
};

U64 board_getPieceSet(CBoard self, PieceType pt) {
  return self->pieceBB[pieceCode(pt)] & self->pieceBB[colorCode(pt)];
}
U64 board_getWhitePawns(CBoard self) {
  return self->pieceBB[PAWN] & self->pieceBB[WHITE];
}
U64 board_getBlackPawns(CBoard self) {
  return self->pieceBB[PAWN] & self->pieceBB[BLACK];
}
U64 board_getPawns(CBoard self, Color ct) {
  return self->pieceBB[PAWN] & self->pieceBB[ct];
}

/* ... */

void bitboard_print(U64 bitboard) {
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = (7 - rank) * 8 + file;

      if (!file)
        printf(" %d  ", 8 - rank);

      printf("%d ", bit_get(bitboard, square) ? 1 : 0);
    }
    printf("\n");
  }

  printf("\n    A B C D E F G H\n\n");
  printf("    Bitboard: %llud\n\n", bitboard);
}

/* ATTACKS */

// clang-format off
const int bishop_relavant_bits[] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6,
};

const int rook_relavant_bits[] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12,
};
// clang-format on

// pawn attack table [side][square]
U64 pawn_attacks[2][64];

// knight attack table [square]
U64 knight_attacks[64];

// king attack table [square]
U64 king_attacks[64];

// generate pawn attack
U64 mask_pawn_attacks(int side, int square) {
  U64 bitboard = C64(0), attacks;

  bit_set(bitboard, square);
  if (side == WHITE)
    return noWeOne(bitboard) | noEaOne(bitboard);
  else
    return soWeOne(bitboard) | soEaOne(bitboard);
}

U64 mask_knight_attacks(int square) {
  U64 bitboard = C64(0), attacks = C64(0), tmp;

  bit_set(bitboard, square);
  tmp = nortOne(nortOne(bitboard));
  attacks |= westOne(tmp) | eastOne(tmp);
  tmp = soutOne(soutOne(bitboard));
  attacks |= westOne(tmp) | eastOne(tmp);
  tmp = westOne(westOne(bitboard));
  attacks |= soutOne(tmp) | nortOne(tmp);
  tmp = eastOne(eastOne(bitboard));
  attacks |= soutOne(tmp) | nortOne(tmp);

  return attacks;
}

U64 mask_king_attacks(int square) {
  U64 bitboard = C64(0), attacks = C64(0);

  bit_set(bitboard, square);
  attacks |= westOne(bitboard) | eastOne(bitboard);
  attacks |= soutOne(bitboard) | nortOne(bitboard);
  attacks |= soutOne(bitboard) | nortOne(bitboard);
  attacks |= soEaOne(bitboard) | noEaOne(bitboard);
  attacks |= soWeOne(bitboard) | noWeOne(bitboard);

  return attacks;
}

U64 mask_slide_attacks(int square, U64 block, direction_f dir[4], int len[4]) {
  U64 bitboard = C64(0), attacks = C64(0), tmp;
  int i, j;

  bit_set(bitboard, square);
  for (i = 0; i < 4; i++) {
    for (j = 0, tmp = bitboard; j < len[i]; j++) {
      attacks |= tmp = (dir[i])(tmp);
      if (tmp & block)
        break;
    }
  }
  return attacks;
}

direction_f bishop_direction[4] = {noEaOne, noWeOne, soEaOne, soWeOne};
direction_f rook_direction[4] = {westOne, soutOne, eastOne, nortOne};

U64 mask_bishop_attacks(int square) {
  int tr = square / 8, tf = square % 8;
  int len[4] = {MIN(7 - tf, 7 - tr) - 1, MIN(tf, 7 - tr) - 1,
                MIN(7 - tf, tr) - 1, MIN(tf, tr) - 1};
  return mask_slide_attacks(square, C64(0), bishop_direction, len);
}

U64 mask_rook_attacks(int square) {
  int tr = square / 8, tf = square % 8;
  int len[4] = {tf - 1, tr - 1, 6 - tf, 6 - tr};

  return mask_slide_attacks(square, C64(0), rook_direction, len);
}

U64 bishop_attacks_on_the_fly(int square, U64 block) {
  int tr = square / 8, tf = square % 8;
  int len[4] = {MIN(7 - tf, 7 - tr), MIN(tf, 7 - tr), MIN(7 - tf, tr),
                MIN(tf, tr)};

  return mask_slide_attacks(square, block, bishop_direction, len);
}

U64 rook_attacks_on_the_fly(int square, U64 block) {
  int tr = square / 8, tf = square % 8;
  int len[4] = {tf, tr, 7 - tf, 7 - tr};

  return mask_slide_attacks(square, block, rook_direction, len);
}

void init_leapers_attacks(void) {
  for (int square = 0; square < 64; square++) {
    pawn_attacks[WHITE][square] = mask_pawn_attacks(WHITE, square);
    pawn_attacks[BLACK][square] = mask_pawn_attacks(BLACK, square);
    knight_attacks[square] = mask_knight_attacks(square);
    king_attacks[square] = mask_king_attacks(square);
  }
}

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
  U64 occupancy = C64(0);

  for (int count = 0; count < bits_in_mask; count++) {
    int square = bit_lsb_index(attack_mask);
    bit_pop(attack_mask, square);

    if (index & (1 << count))
      bit_set(occupancy, square);
  }

  return occupancy;
}

int main(void) {
  init_leapers_attacks();

  bitboard_print(generate_magic_number());
  return 0;
}
