#include "bishop.hpp"
#include "bit.hpp"
#include "bitboard.hpp"
#include "slider.hpp"

#include <array>
#include <iostream>

namespace attack {
namespace bishop {

inline constexpr const U64 bishop_magic_numbers[64] = {
    C64(0x40040844404084),   C64(0x2004208a004208),   C64(0x10190041080202),   C64(0x108060845042010),
    C64(0x581104180800210),  C64(0x2112080446200010), C64(0x1080820820060210), C64(0x3c0808410220200),
    C64(0x4050404440404),    C64(0x21001420088),      C64(0x24d0080801082102), C64(0x1020a0a020400),
    C64(0x40308200402),      C64(0x4011002100800),    C64(0x401484104104005),  C64(0x801010402020200),
    C64(0x400210c3880100),   C64(0x404022024108200),  C64(0x810018200204102),  C64(0x4002801a02003),
    C64(0x85040820080400),   C64(0x810102c808880400), C64(0xe900410884800),    C64(0x8002020480840102),
    C64(0x220200865090201),  C64(0x2010100a02021202), C64(0x152048408022401),  C64(0x20080002081110),
    C64(0x4001001021004000), C64(0x800040400a011002), C64(0xe4004081011002),   C64(0x1c004001012080),
    C64(0x8004200962a00220), C64(0x8422100208500202), C64(0x2000402200300c08), C64(0x8646020080080080),
    C64(0x80020a0200100808), C64(0x2010004880111000), C64(0x623000a080011400), C64(0x42008c0340209202),
    C64(0x209188240001000),  C64(0x400408a884001800), C64(0x110400a6080400),   C64(0x1840060a44020800),
    C64(0x90080104000041),   C64(0x201011000808101),  C64(0x1a2208080504f080), C64(0x8012020600211212),
    C64(0x500861011240000),  C64(0x180806108200800),  C64(0x4000020e01040044), C64(0x300000261044000a),
    C64(0x802241102020002),  C64(0x20906061210001),   C64(0x5a84841004010310), C64(0x4010801011c04),
    C64(0xa010109502200),    C64(0x4a02012000),       C64(0x500201010098b028), C64(0x8040002811040900),
    C64(0x28000010020204),   C64(0x6000020202d0240),  C64(0x8918844842082200), C64(0x4010011029020020),
};

static inline constexpr const int relevant_bits[64] = {
    // clang-format off
      6, 5, 5, 5, 5, 5, 5, 6,
      5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 7, 7, 7, 7, 5, 5,
      5, 5, 7, 9, 9, 7, 5, 5,
      5, 5, 7, 9, 9, 7, 5, 5,
      5, 5, 7, 7, 7, 7, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5,
      6, 5, 5, 5, 5, 5, 5, 6,
    // clang-format on
};

inline constexpr const bitboard::direction_f dir[4] = {bitboard::noEaOne, bitboard::noWeOne,
                                                       bitboard::soEaOne, bitboard::soWeOne};

inline constexpr U32 hash(const U64 key, const Square square) {
    return (key * bishop_magic_numbers[square]) >> (64 - relevant_bits[square]);
}

inline constexpr U64 mask_fly(const Square square, U64 block) {
    int tr = square / 8, tf = square % 8;
    int len[4] = {std::min(7 - tf, 7 - tr), std::min(tf, 7 - tr), std::min(7 - tf, tr), std::min(tf, tr)};

    return attack::slider::mask(square, block, dir, len);
}

std::array<U64, 64> mask = {{0}};
std::array<std::array<U64, 4098>, 64> attacks = {{{0}}};

void init(void) {
    for (Square square = Square::a1; square <= Square::h8; ++square) {
        int tr = square / 8, tf = square % 8;
        int len[4] = {std::min(7 - tf, 7 - tr) - 1, std::min(tf, 7 - tr) - 1, std::min(7 - tf, tr) - 1,
                      std::min(tf, tr) - 1};
        mask[square] = attack::slider::mask(square, C64(0), dir, len);
    }

    for (Square square = Square::a1; square <= Square::h8; ++square) {
        U64 attack_mask = mask[square];
        uint8_t relevant_bits = bit::count(attack_mask);
        U64 occupancy_indices = C64(1) << relevant_bits;

        for (U64 idx = 0; idx < occupancy_indices; idx++) {
            U64 occupancy = attack::slider::occupancy(idx, relevant_bits, attack_mask);
            U32 magic_index = hash(occupancy, square);
            attacks[square][magic_index] = mask_fly(square, occupancy);
        }
    }
}

U64 attack(const Square square, U64 occupancy) {
    occupancy &= mask[square];
    occupancy = hash(occupancy, square);
    return attacks[square][occupancy];
}

} // namespace bishop
} // namespace attack
