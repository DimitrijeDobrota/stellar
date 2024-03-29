#ifndef STELLAR_ATTACK_H
#define STELLAR_ATTACK_H

#include "square.hpp"
#include "utils.hpp"

#include "bishop.hpp"
#include "king.hpp"
#include "knight.hpp"
#include "pawnb.hpp"
#include "pawnw.hpp"
#include "queen.hpp"
#include "rook.hpp"

namespace attack {

void init(void);
using attack_f = U64 (*)(const square::Square, U64);

} // namespace attack

#endif
