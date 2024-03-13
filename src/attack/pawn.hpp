#ifndef STELLAR_ATTACK_PAWN_H
#define STELLAR_ATTACK_PAWN_H

#include "bit.hpp"
#include "bitboard.hpp"
#include "utils.hpp"

#include <array>

namespace attack {
namespace pawn {

static constexpr U64 mask_white(const Square square) {
    U64 bitboard = C64(0);

    bit::set(bitboard, square);
    return bitboard::noWeOne(bitboard) | bitboard::noEaOne(bitboard);
}

static constexpr U64 mask_black(const Square square) {
    U64 bitboard = C64(0);

    bit::set(bitboard, square);
    return bitboard::soWeOne(bitboard) | bitboard::soEaOne(bitboard);
}

typedef std::array<std::array<U64, 64>, 2> attack_array;
const auto attacks = []() {
    attack_array attacks;

    for (Square square = Square::a1; square <= Square::h8; ++square) {
        attacks[WHITE][square] = mask_white(square);
        attacks[BLACK][square] = mask_black(square);
    }

    return attacks;
}();

inline constexpr U64 attack(Color color, Square square) { return attacks[color][square]; }

} // namespace pawn
} // namespace attack

#endif
