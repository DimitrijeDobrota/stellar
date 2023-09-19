#ifndef STELLAR_COLOR_H
#define STELLAR_COLOR_H

#include <string>

namespace color {

enum Color {
    WHITE = 0,
    BLACK
};

inline constexpr const Color other(const Color color) { return color == WHITE ? BLACK : WHITE; }
inline constexpr const std::string to_string(const Color color) {
    return std::string(color == WHITE ? "white" : "black");
}

} // namespace color

#endif
