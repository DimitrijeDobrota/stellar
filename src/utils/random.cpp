#include "random.hpp"

U32 state = C32(1804289383);

void random_state_reset() {
    state = C32(1804289383);
}

U32 random_get_U32() {
    U32 number = state;

    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    return state = number;
}

U64 random_get_U64() {
    U64 n1, n2, n3, n4;

    n1 = (U64)(random_get_U32()) & C64(0xFFFF);
    n2 = (U64)(random_get_U32()) & C64(0xFFFF);
    n3 = (U64)(random_get_U32()) & C64(0xFFFF);
    n4 = (U64)(random_get_U32()) & C64(0xFFFF);

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}
