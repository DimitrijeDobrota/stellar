#ifndef STELLAR_ATTACK_ROOK_H
#define STELLAR_ATTACK_ROOK_H

#include "utils.hpp"

namespace attack {
namespace rook {

void init(void);
U64 attack(const Square square, U64 occupancy);

} // namespace rook
} // namespace attack

#endif
