#ifndef STELLAR_ATTACK_ROOK_H
#define STELLAR_ATTACK_ROOK_H

#include "square.hpp"
#include "utils.hpp"

namespace attack {
namespace rook {

void init(void);
U64 attack(const square::Square square, U64 occupancy);

} // namespace rook
} // namespace attack

#endif
