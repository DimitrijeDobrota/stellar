#ifndef STELLAR_BIT_H
#define STELLAR_BIT_H

#include "utils.hpp"

namespace bit {

inline constexpr bool get(const U64 &bitboard, uint8_t square) { return (bitboard >> (square)) & C64(1); }
inline constexpr void set(U64 &bitboard, uint8_t square) { bitboard |= (C64(1) << square); }
inline constexpr void pop(U64 &bitboard, uint8_t square) { bitboard &= ~(C64(1) << (square)); }

inline constexpr uint8_t count(U64 bitboard) {
#if __has_builtin(__builtin_popcountll)
    return __builtin_popcountll(bitboard);
#else
    int count = 0;
    for (; bitboard > 0; bitboard &= bitboard - 1)
        count++;
    return count;
#endif
}

inline constexpr uint8_t lsb_index(U64 bitboard) {
#if __has_builtin(__builtin_ffsll)
    return __builtin_ffsll(bitboard) - 1;
#else
    if (!bitboard) return -1;
    return bit_count((bitboard & -bitboard) - 1);
#endif
}

inline constexpr U64 &lsb_pop(U64 &bitboard) { return bitboard &= bitboard & (bitboard - 1); }

#define bitboard_for_each_bit(var, bb)                                                                       \
    for (var = bit::lsb_index(bb); bb; bit::lsb_pop(bb), var = bit::lsb_index(bb))

} // namespace bit

#endif
