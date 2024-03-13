#ifndef STELLAR_UTILS_HPP
#define STELLAR_UTILS_HPP

#include <cstdint>

#define C64(constantU64) constantU64##ULL
#define C32(constantU64) constantU64##UL

typedef uint64_t U64;
typedef uint32_t U32;

#define ENABLE_INCR_OPERATORS_ON(T)                                                                          \
    inline T &operator++(T &d) { return d = T(int(d) + 1); }                                                 \
    inline T &operator--(T &d) { return d = T(int(d) - 1); }

/* Color */

enum Color {
    WHITE,
    BLACK,
    COLOR_NB = 2
};

inline constexpr const Color other(const Color color) { return color == WHITE ? BLACK : WHITE; }

/* Square */

enum Square : int {
    // clang-format off
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, no_sq
    // clang-format on
};

ENABLE_INCR_OPERATORS_ON(Square)

constexpr const Square mirror_array[]{
    // clang-format off
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
    // clang-format on
};

constexpr uint8_t get_file(const Square square) { return square & 0x07; }
constexpr uint8_t get_rank(const Square square) { return square >> 3; }
constexpr Square get_mirror(const Square square) { return mirror_array[square]; }

/* piece */

enum Type {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE = 7,
};

ENABLE_INCR_OPERATORS_ON(Type)

#endif
