#ifndef STELLAR_ATTAKC_SLIDER_H
#define STELLAR_ATTAKC_SLIDER_H

#include "bit.hpp"
#include "bitboard.hpp"
#include "square.hpp"
#include "utils.hpp"

namespace attack {
namespace slider {

inline constexpr U64 occupancy(U64 index, uint8_t bits_in_mask, U64 attack_mask) {
    U64 occupancy = C64(0);

    for (uint8_t count = 0; count < bits_in_mask; count++) {
        uint8_t square = bit::lsb_index(attack_mask);
        bit::lsb_pop(attack_mask);

        if (bit::get(index, count)) bit::set(occupancy, square);
    }

    return occupancy;
}

inline constexpr U64 mask(const square::Square square, U64 block, const bitboard::direction_f dir[4],
                          const int len[4]) {
    U64 bitboard = C64(0), attacks = C64(0);
    bit::set(bitboard, to_underlying(square));
    for (int i = 0; i < 4; i++) {
        U64 tmp = bitboard;
        for (int j = 0; j < len[i]; j++) {
            attacks |= tmp = (dir[i])(tmp);
            if (tmp & block) break;
        }
    }
    return attacks;
}

} // namespace slider
} // namespace attack
#endif
