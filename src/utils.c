#include <stdio.h>
#ifdef WIN64
#include <widnows.h>
#else
#include <sys/time.h>
#endif

#include "utils.h"

const U64 universe = C64(0xffffffffffffffff); //
const U64 notAFile = C64(0xfefefefefefefefe); // ~0x0101010101010101
const U64 notHFile = C64(0x7f7f7f7f7f7f7f7f); // ~0x8080808080808080

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

// clang-format off
const char *square_to_coordinates[]={
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", " "
};
// clang-format on

int bit_count(U64 bitboard) {
  int count = 0;

  while (bitboard > 0) {
    count++;
    bitboard &= bitboard - 1;
  }

  return count;
}

int bit_lsb_index(U64 bitboard) {
  if (!bitboard)
    return -1;

  return bit_count((bitboard & -bitboard) - 1);
}

void bitboard_print(U64 bitboard) {
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      Square square = (7 - rank) * 8 + file;

      if (!file)
        printf(" %d  ", 8 - rank);

      printf("%d ", bit_get(bitboard, square) ? 1 : 0);
    }
    printf("\n");
  }

  printf("\n    A B C D E F G H\n\n");
  printf("    Bitboard: %llud\n\n", bitboard);
}

int get_time_ms(void) {
#ifdef WIN64
  return GetTickCount();
#else
  struct timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec * 1000 + time.tv_usec / 1000;
#endif
}
