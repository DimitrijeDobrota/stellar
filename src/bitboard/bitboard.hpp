#ifndef STELLAR_BITBOARD_H
#define STELLAR_BITBOARD_H

#include "square.hpp"
#include "utils.hpp"

namespace bitboard {

void print(U64 bitboard);

inline constexpr const U64 notAFile = C64(0xfefefefefefefefe);
inline constexpr const U64 notHFile = C64(0x7f7f7f7f7f7f7f7f);

typedef U64 (*direction_f)(U64);
inline constexpr U64 soutOne(U64 b) { return b >> 8; }
inline constexpr U64 nortOne(U64 b) { return b << 8; }
inline constexpr U64 eastOne(U64 b) { return (b & notHFile) << 1; }
inline constexpr U64 westOne(U64 b) { return (b & notAFile) >> 1; }
inline constexpr U64 soEaOne(U64 b) { return (b & notHFile) >> 7; }
inline constexpr U64 soWeOne(U64 b) { return (b & notAFile) >> 9; }
inline constexpr U64 noEaOne(U64 b) { return (b & notHFile) << 9; }
inline constexpr U64 noWeOne(U64 b) { return (b & notAFile) << 7; }

} // namespace bitboard

#endif
