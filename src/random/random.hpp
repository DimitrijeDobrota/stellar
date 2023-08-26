#ifndef STELLAR_RANDOM_H
#define STELLAR_RANDOM_H

#include "utils.hpp"

class Random {
  public:
    constexpr Random(void) = default;
    constexpr Random(U64 seed) : state(seed) {}

    constexpr U64 operator()(void) { return get_U64(); }

    constexpr void reset(void) { state = seed; }

    constexpr U32 get_U32(void) {
        U32 number = state;

        number ^= number << 13;
        number ^= number >> 17;
        number ^= number << 5;

        return state = number;
    }

    constexpr U64 get_U64(void) {
        U64 n1, n2, n3, n4;

        n1 = (U64)(get_U32()) & C64(0xFFFF);
        n2 = (U64)(get_U32()) & C64(0xFFFF);
        n3 = (U64)(get_U32()) & C64(0xFFFF);
        n4 = (U64)(get_U32()) & C64(0xFFFF);

        return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
    }

  private:
    static inline constexpr const U32 seed = C32(1804289383);
    U32 state = seed;
};

#endif
