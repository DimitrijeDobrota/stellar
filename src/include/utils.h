#ifndef STELLAR_UTILS_H
#define STELLAR_UTILS_H

// useful macros
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

// define number types
typedef unsigned long long U64;           // define bitboard data type
#define C64(constantU64) constantU64##ULL // define shorthand for constants

typedef unsigned int U32;
#define C32(constantU32) constantU32##U

// useful bit patterns
extern const U64 universe;
extern const U64 notAFile;
extern const U64 notHFile;

// useful bit functions
#define bit_get(bitboard, square) (((bitboard) >> (square)) & C64(1))
#define bit_set(bitboard, square) ((bitboard) |= C64(1) << (square))
#define bit_pop(bitboard, square) ((bitboard) &= ~(C64(1) << (square)))
int bit_count(U64 bitboard);
int bit_lsb_index(U64 bitboard);

#define bitboard_for_each_bit(var, bb)                                         \
    for (var = bit_lsb_index(bb); bb; bit_pop(bb, var), var = bit_lsb_index(bb))

void bitboard_print(U64 bitboard);

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
  a8, b8, c8, d8, e8, f8, g8, h8, no_sq
};
// clang-format on
typedef enum enumSquare Square;

extern const char *square_to_coordinates[];
Square coordinates_to_square(const char *cord);

// board moving
typedef U64 (*direction_f)(U64);
U64 soutOne(U64 b);
U64 nortOne(U64 b);
U64 eastOne(U64 b);
U64 westOne(U64 b);
U64 soEaOne(U64 b);
U64 soWeOne(U64 b);
U64 noEaOne(U64 b);
U64 noWeOne(U64 b);

// board rotation
U64 rotateLeft(U64 x, int s);
U64 rotateRight(U64 x, int s);

int get_time_ms(void);

typedef U64 (*attack_f)(Square square, U64 occupancy);

#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position                                                         \
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "

#endif
