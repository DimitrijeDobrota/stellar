#ifndef STELLAR_BISHOP_H
#define STELLAR_BISHOP_H

#include "square.hpp"
#include "utils.hpp"

namespace attack {
namespace bishop {

void init(void);
U64 attack(const square::Square square, U64 occupancy);

}
} // namespace attack

#endif
