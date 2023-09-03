#ifndef STELLAR_COLOR_H
#define STELLAR_COLOR_H

namespace color {

enum Color {
    WHITE = 0,
    BLACK
};

inline constexpr const Color other(const Color color) { return color == WHITE ? BLACK : WHITE; }

} // namespace color

#endif
