#ifndef STELLAR_ATTACK_PAWNB_H
#define STELLAR_ATTACK_PAWNB_H

#include "bit.hpp"
#include "bitboard.hpp"
#include "square.hpp"
#include "utils.hpp"

#include <array>

namespace attack {
namespace pawnb {

static constexpr U64 mask(const square::Square square) {
    U64 bitboard = C64(0);

    bit::set(bitboard, square);
    return bitboard::soWeOne(bitboard) | bitboard::soEaOne(bitboard);
}

typedef std::array<U64, 64> attack_array;
const attack_array attacks = []() -> attack_array {
    std::array<U64, 64> attacks;

    for (square::Square square = square::a1; square <= square::h8; ++square) {
        attacks[square] = mask(square);
    }

    return attacks;
}();

inline constexpr U64 attack(const square::Square square, U64 occupancy) { return attacks[square]; }

} // namespace pawnb
}; // namespace attack

#endif
