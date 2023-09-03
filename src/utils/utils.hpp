#ifndef STELLAR_UTILS_CPP_H
#define STELLAR_UTILS_CPP_H

#include <cstdint>

#define C64(constantU64) constantU64##ULL
#define C32(constantU64) constantU64##UL

typedef uint64_t U64;
typedef uint32_t U32;

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

#endif
