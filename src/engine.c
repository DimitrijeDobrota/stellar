#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cii/assert.h>
#include <cii/except.h>
#include <cii/mem.h>

/* DEFINITIONS */

typedef unsigned long long U64;           // define bitboard data type
#define C64(constantU64) constantU64##ULL // define shorthand for constants

// usefull bit patterns
const U64 universe = C64(0xffffffffffffffff); //
const U64 notAFile = C64(0xfefefefefefefefe); // ~0x0101010101010101
const U64 notHFile = C64(0x7f7f7f7f7f7f7f7f); // ~0x8080808080808080

// useful bit operations
#define bit_get(bitboard, square) ((bitboard >> square) & C64(1))
#define bit_set(bitboard, square) (bitboard |= C64(1) << square)
#define bit_pop(bitboard, square) (bitboard &= ~(C64(1) << square))

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

char squares[][3]={
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

// pawn attack table [side][square]
U64 pawn_attacks[2][64];

// generate pawn attack
U64 mask_pawn_attacks(int side, int square) {
  U64 bitboard = C64(0), attacks;

  bit_set(bitboard, square);
  if (side == WHITE)
    return noWeOne(bitboard) | noEaOne(bitboard);
  else
    return soWeOne(bitboard) | soEaOne(bitboard);
}

void init_leapers_attacks(void) {
  for (int square = 0; square < 64; square++) {
    pawn_attacks[WHITE][square] = mask_pawn_attacks(WHITE, square);
    pawn_attacks[BLACK][square] = mask_pawn_attacks(BLACK, square);
    bitboard_print(pawn_attacks[WHITE][square]);
  }
}

int main(void) {
  init_leapers_attacks();
  return 0;
}
