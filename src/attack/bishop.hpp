#ifndef STELLAR_BISHOP_H
#define STELLAR_BISHOP_H

#include "utils.hpp"

namespace attack {
namespace bishop {

void init(void);
U64 attack(const Square square, U64 occupancy);

} // namespace bishop
} // namespace attack

#endif
