#ifndef STELLAR_ATTACK_QUEEN_H
#define STELLAR_ATTACK_QUEEN_H

#include "bishop.hpp"
#include "rook.hpp"

namespace attack {
namespace queen {

inline constexpr U64 attack(const square::Square square, U64 occupancy) {
    return rook::attack(square, occupancy) | bishop::attack(square, occupancy);
}

} // namespace queen
} // namespace attack

#endif
