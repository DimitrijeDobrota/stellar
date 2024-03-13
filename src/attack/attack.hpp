#ifndef STELLAR_ATTACK_H
#define STELLAR_ATTACK_H

#include "utils.hpp"

#include "bishop.hpp"
#include "king.hpp"
#include "knight.hpp"
#include "pawn.hpp"
#include "queen.hpp"
#include "rook.hpp"

#include "piece.hpp"

namespace attack {

void init(void);

inline constexpr const U64 attack_pawn(const Color color, const Square from) {
    return attack::pawn::attack(color, from);
}

inline constexpr const U64 attack(const piece::Type type, const Square from, const U64 occupancy) {
    switch (type) {
    case piece::QUEEN: return attack::queen::attack(from, occupancy);
    case piece::ROOK: return attack::rook::attack(from, occupancy);
    case piece::BISHOP: return attack::bishop::attack(from, occupancy);
    case piece::KING: return attack::king::attack(from);
    case piece::KNIGHT: return attack::knight::attack(from);
    default: return 0;
    }
}

} // namespace attack

#endif
