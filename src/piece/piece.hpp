#ifndef STELLAR_PIECE_H
#define STELLAR_PIECE_H

#include "color.hpp"
#include "utils.hpp"

#include <cctype>

namespace piece {

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

struct Piece {
    const uint8_t index;
    const Type type;
    const color::Color color;
    const char code;
};

inline constexpr const Piece table[2][6] = {
    // clang-format off
    {
        { .index = 0, .type = PAWN,   .color = color::WHITE, .code = 'P' },
        { .index = 1, .type = KNIGHT, .color = color::WHITE, .code = 'N' },
        { .index = 2, .type = BISHOP, .color = color::WHITE, .code = 'B' },
        { .index = 3, .type = ROOK,   .color = color::WHITE, .code = 'R' },
        { .index = 4, .type = QUEEN,  .color = color::WHITE, .code = 'Q' },
        { .index = 5, .type = KING,   .color = color::WHITE, .code = 'K' },
    }, {
        { .index = 6, .type = PAWN,   .color = color::BLACK, .code = 'p' },
        { .index = 7, .type = KNIGHT, .color = color::BLACK, .code = 'n' },
        { .index = 8, .type = BISHOP, .color = color::BLACK, .code = 'b' },
        { .index = 9, .type = ROOK,   .color = color::BLACK, .code = 'r' },
        {.index = 10, .type = QUEEN,  .color = color::BLACK, .code = 'q' },
        {.index = 11, .type = KING,   .color = color::BLACK, .code = 'k' },
    },
    // clang-format on
};

inline constexpr const Piece &get(const Type type, const color::Color color) { return table[color][type]; }

inline constexpr const char get_code(const Type type, const color::Color color = color::BLACK) {
    return get(type, color).code;
}

inline constexpr const U64 get_index(const Type type, const color::Color color) {
    return get(type, color).index;
}

inline constexpr const Piece &get_from_code(const char code) {
    color::Color color = isupper(code) ? color::WHITE : color::BLACK;

    for (Type type = PAWN; type <= KING; ++type) {
        const Piece &piece = get(type, color);
        if (piece.code == code) return piece;
    }

    throw std::exception();
}

inline constexpr const Piece &get_from_index(const uint8_t index) { return table[index / 6][index % 6]; }

} // namespace piece

#endif
