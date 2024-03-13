#ifndef STELLAR_UTILS_CPP_H
#define STELLAR_UTILS_CPP_H

#include <cstdint>

#define C64(constantU64) constantU64##ULL
#define C32(constantU64) constantU64##UL

typedef uint64_t U64;
typedef uint32_t U32;

#define ENABLE_INCR_OPERATORS_ON(T)                                                                          \
    inline T &operator++(T &d) { return d = T(int(d) + 1); }                                                 \
    inline T &operator--(T &d) { return d = T(int(d) - 1); }

#endif
