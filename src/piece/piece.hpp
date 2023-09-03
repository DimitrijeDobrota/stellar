#ifndef STELLAR_PIECE_H
#define STELLAR_PIECE_H

#include "attack.hpp"
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
typedef Iterator<Type, Type::PAWN, Type::KING> TypeIter;

struct Piece {
    const uint8_t index;
    const Type type;
    const color::Color color;
    const char code;
    const attack::attack_f attack;
};

inline constexpr const Piece table[2][6] = {
    // clang-format off
    {
        { .index = 0, .type = PAWN,   .color = color::WHITE, .code = 'P', .attack = attack::pawnw::attack  },
        { .index = 1, .type = KNIGHT, .color = color::WHITE, .code = 'N', .attack = attack::knight::attack },
        { .index = 2, .type = BISHOP, .color = color::WHITE, .code = 'B', .attack = attack::bishop::attack },
        { .index = 3, .type = ROOK,   .color = color::WHITE, .code = 'R', .attack = attack::rook::attack   },
        { .index = 4, .type = QUEEN,  .color = color::WHITE, .code = 'Q', .attack = attack::queen::attack  },
        { .index = 5, .type = KING,   .color = color::WHITE, .code = 'K', .attack = attack::king::attack   },
    }, {
        { .index = 6, .type = PAWN,   .color = color::BLACK, .code = 'p', .attack = attack::pawnb::attack  },
        { .index = 7, .type = KNIGHT, .color = color::BLACK, .code = 'n', .attack = attack::knight::attack },
        { .index = 8, .type = BISHOP, .color = color::BLACK, .code = 'b', .attack = attack::bishop::attack },
        { .index = 9, .type = ROOK,   .color = color::BLACK, .code = 'r', .attack = attack::rook::attack   },
        {.index = 10, .type = QUEEN,  .color = color::BLACK, .code = 'q', .attack = attack::queen::attack  },
        {.index = 11, .type = KING,   .color = color::BLACK, .code = 'k', .attack = attack::king::attack   },
    },
    // clang-format on
};

inline constexpr const Piece &get(const Type type, const color::Color color) {
    return table[static_cast<uint8_t>(color)][static_cast<uint8_t>(type)];
}

inline constexpr const U64 get_attack(const Type type, const color::Color color, const square::Square from,
                                      const U64 occupancy) {
    return get(type, color).attack(from, occupancy);
}

inline constexpr const char get_code(const Type type, const color::Color color = color::WHITE) {
    return get(type, color).code;
}

inline constexpr const U64 get_index(const Type type, const color::Color color) {
    return get(type, color).index;
}

inline constexpr const Piece &get_from_code(const char code) {
    color::Color color = isupper(code) ? color::WHITE : color::BLACK;

    for (Type type : TypeIter()) {
        const Piece &piece = get(type, color);
        if (piece.code == code) return piece;
    }

    throw std::exception();
}

inline constexpr const Piece &get_from_index(const uint8_t index) { return table[index / 6][index % 6]; }

} // namespace piece

#endif
