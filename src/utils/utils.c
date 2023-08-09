#include <stdio.h>
#include <sys/time.h>

#include "utils.h"

const U64 universe = C64(0xffffffffffffffff);
const U64 notAFile = C64(0xfefefefefefefefe);
const U64 notHFile = C64(0x7f7f7f7f7f7f7f7f);

inline uint8_t bit_count(U64 bitboard) {
#if __has_builtin(__builtin_popcountll)
    return __builtin_popcountll(bitboard);
#endif

    int count = 0;
    for (; bitboard > 0; bitboard &= bitboard - 1)
        count++;
    return count;
}

inline uint8_t bit_lsb_index(U64 bitboard) {
#if __has_builtin(__builtin_ffsll)
    return __builtin_ffsll(bitboard) - 1;
#endif

    if (!bitboard) return -1;
    return bit_count((bitboard & -bitboard) - 1);
}

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

eSquare coordinates_to_square(const char *cord) {
    return (cord[1] - '1') * 8 + (cord[0] - 'a');
}

int get_time_ms(void) {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}
