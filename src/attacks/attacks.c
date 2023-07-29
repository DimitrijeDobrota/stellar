#include "attacks.h"
#include "internal.h"
#include "magic.h"

#define UNUSED(x) (void)(x)

// Magic constants

U64 attacks_wpawn_get(Square square, U64 occupancy) {
    UNUSED(occupancy);
    return pawn_attacks[WHITE][square];
}

U64 attacks_bpawn_get(Square square, U64 occupancy) {
    UNUSED(occupancy);
    return pawn_attacks[BLACK][square];
}

U64 attacks_knight_get(Square square, U64 occupancy) {
    UNUSED(occupancy);
    return knight_attacks[square];
}

U64 attakcs_king_get(Square square, U64 occupancy) {
    UNUSED(occupancy);
    return king_attacks[square];
}

U64 attacks_bishop_get(Square square, U64 occupancy) {
    occupancy &= bishop_masks[square];
    occupancy = hash(occupancy, bishop_magic_numbers[square],
                     bishop_relevant_bits[square]);
    return bishop_attacks[square][occupancy];
}

U64 attacks_rook_get(Square square, U64 occupancy) {
    occupancy &= rook_masks[square];
    occupancy =
        hash(occupancy, rook_magic_numbers[square], rook_relevant_bits[square]);
    return rook_attacks[square][occupancy];
}

U64 attacks_queen_get(Square square, U64 occupancy) {
    return (attacks_bishop_get(square, occupancy) |
            attacks_rook_get(square, occupancy));
}

void attacks_init_leapers(void) {
    for (Square square = 0; square < 64; square++) {
        pawn_attacks[WHITE][square] = pawn_mask(WHITE, square);
        pawn_attacks[BLACK][square] = pawn_mask(BLACK, square);
        knight_attacks[square] = knight_mask(square);
        king_attacks[square] = king_mask(square);
    }
}

void attacks_init_sliders(int bishop) {
    for (Square square = 0; square < 64; square++) {
        U64 attack_mask;

        if (bishop) {
            bishop_masks[square] = bishop_mask(square);
            attack_mask = bishop_masks[square];
        } else {
            rook_masks[square] = rook_mask(square);
            attack_mask = rook_masks[square];
        }

        int relevant_bits = bit_count(attack_mask);
        int occupancy_indicies = 1 << relevant_bits;

        for (int index = 0; index < occupancy_indicies; index++) {
            U64 occupancy = set_occupancy(index, relevant_bits, attack_mask);
            if (bishop) {
                int magic_index = (occupancy * bishop_magic_numbers[square]) >>
                                  (64 - bishop_relevant_bits[square]);
                bishop_attacks[square][magic_index] =
                    bishop_on_the_fly(square, occupancy);
            } else {
                int magic_index = hash(occupancy, rook_magic_numbers[square],
                                       rook_relevant_bits[square]);
                rook_attacks[square][magic_index] =
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
