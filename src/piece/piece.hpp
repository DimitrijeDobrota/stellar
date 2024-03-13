#ifndef STELLAR_PIECE_H
#define STELLAR_PIECE_H

#include "utils.hpp"

#include <cctype>
#include <exception>

namespace piece {

struct Piece {
    const uint8_t index;
    const Type type;
    const Color color;
    const char code;
};

inline constexpr const Piece table[2][6] = {
    // clang-format off
    {
        { .index = 0, .type = PAWN,   .color = WHITE, .code = 'P' },
        { .index = 1, .type = KNIGHT, .color = WHITE, .code = 'N' },
        { .index = 2, .type = BISHOP, .color = WHITE, .code = 'B' },
        { .index = 3, .type = ROOK,   .color = WHITE, .code = 'R' },
        { .index = 4, .type = QUEEN,  .color = WHITE, .code = 'Q' },
        { .index = 5, .type = KING,   .color = WHITE, .code = 'K' },
    }, {
        { .index = 6, .type = PAWN,   .color = BLACK, .code = 'p' },
        { .index = 7, .type = KNIGHT, .color = BLACK, .code = 'n' },
        { .index = 8, .type = BISHOP, .color = BLACK, .code = 'b' },
        { .index = 9, .type = ROOK,   .color = BLACK, .code = 'r' },
        {.index = 10, .type = QUEEN,  .color = BLACK, .code = 'q' },
        {.index = 11, .type = KING,   .color = BLACK, .code = 'k' },
    },
    // clang-format on
};

inline constexpr const Piece &get(const Type type, const Color color) { return table[color][type]; }

inline constexpr const char get_code(const Type type, const Color color = BLACK) {
    return get(type, color).code;
}

inline constexpr const U64 get_index(const Type type, const Color color) { return get(type, color).index; }

inline constexpr const Piece &get_from_code(const char code) {
    Color color = isupper(code) ? WHITE : BLACK;

    for (Type type = PAWN; type <= KING; ++type) {
        const Piece &piece = get(type, color);
        if (piece.code == code) return piece;
    }

    throw std::exception();
}

inline constexpr const Piece &get_from_index(const uint8_t index) { return table[index / 6][index % 6]; }

} // namespace piece

#endif
