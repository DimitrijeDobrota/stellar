#include "attacks.hpp"
#include "internal.hpp"
#include "magic.hpp"
#include "utils_cpp.hpp"

#define UNUSED(x) (void)(x)

U64 attacks_wpawn_get(Square square, U64 occupancy) {
    UNUSED(occupancy);
    return pawn_attacks[to_underlying(Color::WHITE)][to_underlying(square)];
}

U64 attacks_bpawn_get(Square square, U64 occupancy) {
    UNUSED(occupancy);
    return pawn_attacks[to_underlying(Color::BLACK)][to_underlying(square)];
}

U64 attacks_knight_get(Square square, U64 occupancy) {
    UNUSED(occupancy);
    return knight_attacks[to_underlying(square)];
}

U64 attacks_king_get(Square square, U64 occupancy) {
    UNUSED(occupancy);
    return king_attacks[to_underlying(square)];
}

U64 attacks_bishop_get(Square square, U64 occupancy) {
    int square_i = to_underlying(square);
    occupancy &= bishop_masks[square_i];
    occupancy = hash(occupancy, bishop_magic_numbers[square_i],
                     bishop_relevant_bits[square_i]);
    return bishop_attacks[square_i][occupancy];
}

U64 attacks_rook_get(Square square, U64 occupancy) {
    int square_i = to_underlying(square);
    occupancy &= rook_masks[square_i];
    occupancy = hash(occupancy, rook_magic_numbers[square_i],
                     rook_relevant_bits[square_i]);
    return rook_attacks[square_i][occupancy];
}

U64 attacks_queen_get(Square square, U64 occupancy) {
    return (attacks_bishop_get(square, occupancy) |
            attacks_rook_get(square, occupancy));
}

void attacks_init_leapers(void) {
    for (Square square : SquareIter()) {
        uint8_t square_i;
        pawn_attacks[to_underlying(Color::WHITE)][square_i] =
            pawn_mask(Color::WHITE, square);
        pawn_attacks[to_underlying(Color::BLACK)][square_i] =
            pawn_mask(Color::BLACK, square);
        knight_attacks[square_i] = knight_mask(square);
        king_attacks[square_i] = king_mask(square);
    }
}

void attacks_init_sliders(int bishop) {
    for (Square square : SquareIter()) {
        uint8_t square_i;
        U64 attack_mask;

        if (bishop) {
            bishop_masks[square_i] = bishop_mask(square);
            attack_mask = bishop_masks[square_i];
        } else {
            rook_masks[square_i] = rook_mask(square);
            attack_mask = rook_masks[square_i];
        }

        int relevant_bits = bit_count(attack_mask);
        int occupancy_indicies = 1 << relevant_bits;

        for (int index = 0; index < occupancy_indicies; index++) {
            U64 occupancy = set_occupancy(index, relevant_bits, attack_mask);
            if (bishop) {
                int magic_index =
                    (occupancy * bishop_magic_numbers[square_i]) >>
                    (64 - bishop_relevant_bits[square_i]);
                bishop_attacks[square_i][magic_index] =
                    bishop_on_the_fly(square, occupancy);
            } else {
                int magic_index = hash(occupancy, rook_magic_numbers[square_i],
                                       rook_relevant_bits[square_i]);
                rook_attacks[square_i][magic_index] =
                    rook_on_the_fly(square, occupancy);
            }
        }
    }
}

void attacks_init(void) {
    attacks_init_leapers();
    attacks_init_sliders(0);
    attacks_init_sliders(1);
}
