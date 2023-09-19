#ifndef STELLAR_SQUARE_H
#define STELLAR_SQUARE_H

#include "utils.hpp"
#include <string>

namespace square {

enum Square {
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

typedef Iterator<Square, Square::a1, Square::h8> Iter;

inline constexpr const Square mirror_array[]{
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

inline constexpr const char *coordinates_array[] = {
    // clang-format off
   "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
   "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
   "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
   "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
   "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
   "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
   "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
   "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", " "
    // clang-format on
};

inline constexpr const uint8_t file(const Square square) { return to_underlying(square) & 0x07; }
inline constexpr const uint8_t rank(const Square square) { return to_underlying(square) >> 3; }
inline constexpr const Square mirror(const Square square) { return mirror_array[square]; }

inline constexpr const std::string to_coordinates(const Square square) {
    return coordinates_array[to_underlying(square)];
}

inline const Square from_coordinates(const std::string &cord) {
    return static_cast<Square>((cord[1] - '1') * 8 + (cord[0] - 'a'));
}

} // namespace square

#endif
