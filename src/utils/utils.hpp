#ifndef STELLAR_UTILS_CPP_H
#define STELLAR_UTILS_CPP_H

#include <cstdint>
#include <exception>
#include <string>
#include <type_traits>

template <typename E> constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

template <typename C, C beginVal, C endVal> class Iterator {
    typedef typename std::underlying_type<C>::type val_t;
    int val;

  public:
    constexpr Iterator(const C &f) : val(static_cast<val_t>(f)) {}
    constexpr Iterator() : val(static_cast<val_t>(beginVal)) {}
    constexpr Iterator operator++() {
        ++val;
        return *this;
    }
    constexpr C operator*() { return static_cast<C>(val); }
    constexpr Iterator begin() { return *this; }
    constexpr Iterator end() {
        // static const Iterator endIter = ++Iterator(endVal);
        //  return endIter;
        return ++Iterator(endVal);
    }
    constexpr bool operator!=(const Iterator &i) { return val != i.val; }
};

#define C64(constantU64) constantU64##ULL
#define C32(constantU64) constantU64##UL
typedef uint64_t U64;
typedef uint32_t U32;

enum class Color : bool {
    WHITE = 0,
    BLACK
};

// clang-format off
enum class Square: uint8_t {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8, no_sq
};
// clang-format on

typedef Iterator<Square, Square::a1, Square::h8> SquareIter;

inline Square square_from_coordinates(const std::string &cord) {
    return static_cast<Square>((cord[1] - '1') * 8 + (cord[0] - 'a'));
}

// clang-format off
inline const char *square_to_coordinates(Square square) {
    static const char *map[]={
      "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
      "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
      "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
      "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
      "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
      "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
      "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
      "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", " "
    };
    return map[to_underlying(square)];
}
// clang-format on

// useful bit functions
constexpr bool bit_get(const U64 &bitboard, uint8_t square) { return (bitboard >> (square)) & C64(1); }

constexpr void bit_set(U64 &bitboard, uint8_t square) { bitboard |= (C64(1) << square); }

constexpr void bit_pop(U64 &bitboard, uint8_t square) { bitboard &= ~(C64(1) << (square)); }

constexpr uint8_t bit_count(U64 bitboard) {
#if __has_builtin(__builtin_popcountll)
    return __builtin_popcountll(bitboard);
#endif

    int count = 0;
    for (; bitboard > 0; bitboard &= bitboard - 1)
        count++;
    return count;
}

constexpr uint8_t bit_lsb_index(U64 bitboard) {
#if __has_builtin(__builtin_ffsll)
    return __builtin_ffsll(bitboard) - 1;
#endif

    if (!bitboard) return -1;
    return bit_count((bitboard & -bitboard) - 1);
}

#define bit_lsb_pop(bitboard) ((bitboard) &= (bitboard) & ((bitboard)-1))

#define bitboard_for_each_bit(var, bb)                                                                       \
    for (var = bit_lsb_index(bb); bb; bit_lsb_pop(bb), var = bit_lsb_index(bb))

// board moving
inline constexpr const U64 universe = C64(0xffffffffffffffff);
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

#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "

#endif
