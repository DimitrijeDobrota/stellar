#ifndef STELLAR_ATTACK_PAWNW_H
#define STELLAR_ATTACK_PAWNW_H

#include "bit.hpp"
#include "bitboard.hpp"
#include "square.hpp"
#include "utils.hpp"

#include <array>

namespace attack {
namespace pawnw {

static constexpr U64 mask(const square::Square square) {
    U64 bitboard = C64(0);

    bit::set(bitboard, to_underlying(square));
    return bitboard::noWeOne(bitboard) | bitboard::noEaOne(bitboard);
}

typedef std::array<U64, 64> attack_array;
const attack_array attacks = []() -> attack_array {
    std::array<U64, 64> attacks;
    for (uint8_t square = 0; square < 64; square++)
        attacks[square] = mask(static_cast<square::Square>(square));
    return attacks;
}();

inline constexpr U64 attack(const square::Square square, U64 occupancy) {
    return attacks[to_underlying(square)];
}

} // namespace pawnw
} // namespace attack

#endif
