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
#include "slider.hpp"

namespace attack {

typedef U64 (*attack_f)(const square::Square square, U64 occupancy);

}

#endif
