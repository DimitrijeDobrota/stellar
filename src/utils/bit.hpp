#ifndef STELLAR_BIT_H
#define STELLAR_BIT_H

#include "utils.hpp"
#include <bit>

namespace bit {

inline constexpr bool get(const U64 &bitboard, uint8_t square) { return (bitboard >> square) & C64(1); }
inline constexpr void set(U64 &bitboard, uint8_t square) { bitboard |= (C64(1) << square); }
inline constexpr void pop(U64 &bitboard, uint8_t square) { bitboard &= ~(C64(1) << square); }

inline constexpr uint8_t count(U64 bitboard) { return std::popcount(bitboard); }
inline constexpr uint8_t lsb_index(U64 bitboard) { return std::countr_zero(bitboard); }
inline constexpr U64 &lsb_pop(U64 &bitboard) { return bitboard = bitboard & (bitboard - 1); }

#define bitboard_for_each_bit(var, bb)                                                                       \
    for (var = bit::lsb_index(bb); bb; bit::lsb_pop(bb), var = bit::lsb_index(bb))

} // namespace bit

#endif
