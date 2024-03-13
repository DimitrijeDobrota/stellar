#ifndef STELLAR_ATTACK_PAWN_H
#define STELLAR_ATTACK_PAWN_H

#include "bit.hpp"
#include "bitboard.hpp"
#include "color.hpp"
#include "square.hpp"
#include "utils.hpp"

#include <array>

namespace attack {
namespace pawn {

static constexpr U64 mask_white(const square::Square square) {
    U64 bitboard = C64(0);

    bit::set(bitboard, square);
    return bitboard::noWeOne(bitboard) | bitboard::noEaOne(bitboard);
}

static constexpr U64 mask_black(const square::Square square) {
    U64 bitboard = C64(0);

    bit::set(bitboard, square);
    return bitboard::soWeOne(bitboard) | bitboard::soEaOne(bitboard);
}

typedef std::array<std::array<U64, 64>, 2> attack_array;
const auto attacks = []() {
    attack_array attacks;

    for (square::Square square = square::a1; square <= square::h8; ++square) {
        attacks[color::WHITE][square] = mask_white(square);
        attacks[color::BLACK][square] = mask_black(square);
    }

    return attacks;
}();

inline constexpr U64 attack(color::Color color, square::Square square) {
    return attacks[color][square];
}

} // namespace pawn
} // namespace attack

#endif
