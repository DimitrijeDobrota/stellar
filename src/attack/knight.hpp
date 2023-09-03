#ifndef STELLAR_ATTACK_KNIGHT_H
#define STELLAR_ATTACK_KNIGHT_H

#include "bit.hpp"
#include "bitboard.hpp"
#include "square.hpp"
#include "utils.hpp"

#include <array>

namespace attack {
namespace knight {

static constexpr U64 mask(const square::Square square) {
    U64 bitboard = C64(0), attacks = C64(0), tmp;

    bit::set(bitboard, to_underlying(square));
    tmp = bitboard::nortOne(bitboard::nortOne(bitboard));
    attacks |= bitboard::westOne(tmp) | bitboard::eastOne(tmp);
    tmp = bitboard::soutOne(bitboard::soutOne(bitboard));
    attacks |= bitboard::westOne(tmp) | bitboard::eastOne(tmp);
    tmp = bitboard::westOne(bitboard::westOne(bitboard));
    attacks |= bitboard::soutOne(tmp) | bitboard::nortOne(tmp);
    tmp = bitboard::eastOne(bitboard::eastOne(bitboard));
    attacks |= bitboard::soutOne(tmp) | bitboard::nortOne(tmp);

    return attacks;
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

} // namespace knight
} // namespace attack

#endif
