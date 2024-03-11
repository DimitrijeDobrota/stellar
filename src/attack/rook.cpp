#include "rook.hpp"
#include "bit.hpp"
#include "bitboard.hpp"
#include "slider.hpp"

#include <array>

namespace attack {
namespace rook {

inline constexpr const int relevant_bits[64] = {
    // clang-format off
      12, 11, 11, 11, 11, 11, 11, 12,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      12, 11, 11, 11, 11, 11, 11, 12,
    // clang-format on
};

inline constexpr const U64 rook_magic_numbers[64] = {
    C64(0x8a80104000800020), C64(0x140002000100040),  C64(0x2801880a0017001),  C64(0x100081001000420),
    C64(0x200020010080420),  C64(0x3001c0002010008),  C64(0x8480008002000100), C64(0x2080088004402900),
    C64(0x800098204000),     C64(0x2024401000200040), C64(0x100802000801000),  C64(0x120800800801000),
    C64(0x208808088000400),  C64(0x2802200800400),    C64(0x2200800100020080), C64(0x801000060821100),
    C64(0x80044006422000),   C64(0x100808020004000),  C64(0x12108a0010204200), C64(0x140848010000802),
    C64(0x481828014002800),  C64(0x8094004002004100), C64(0x4010040010010802), C64(0x20008806104),
    C64(0x100400080208000),  C64(0x2040002120081000), C64(0x21200680100081),   C64(0x20100080080080),
    C64(0x2000a00200410),    C64(0x20080800400),      C64(0x80088400100102),   C64(0x80004600042881),
    C64(0x4040008040800020), C64(0x440003000200801),  C64(0x4200011004500),    C64(0x188020010100100),
    C64(0x14800401802800),   C64(0x2080040080800200), C64(0x124080204001001),  C64(0x200046502000484),
    C64(0x480400080088020),  C64(0x1000422010034000), C64(0x30200100110040),   C64(0x100021010009),
    C64(0x2002080100110004), C64(0x202008004008002),  C64(0x20020004010100),   C64(0x2048440040820001),
    C64(0x101002200408200),  C64(0x40802000401080),   C64(0x4008142004410100), C64(0x2060820c0120200),
    C64(0x1001004080100),    C64(0x20c020080040080),  C64(0x2935610830022400), C64(0x44440041009200),
    C64(0x280001040802101),  C64(0x2100190040002085), C64(0x80c0084100102001), C64(0x4024081001000421),
    C64(0x20030a0244872),    C64(0x12001008414402),   C64(0x2006104900a0804),  C64(0x1004081002402),
};

inline constexpr const bitboard::direction_f dir[4] = {bitboard::westOne, bitboard::soutOne,
                                                       bitboard::eastOne, bitboard::nortOne};

inline constexpr U32 hash(const U64 key, const square::Square square) {
    uint8_t square_i = to_underlying(square);
    return (key * rook_magic_numbers[square_i]) >> (64 - relevant_bits[square_i]);
}

inline constexpr U64 mask_fly(const square::Square square, U64 block) {
    uint8_t square_i = to_underlying(square);
    int tr = square_i / 8, tf = square_i % 8;
    int len[4] = {tf, tr, 7 - tf, 7 - tr};

    return attack::slider::mask(square, block, dir, len);
}

std::array<U64, 64> masks = {{0}};
U64 mask(const square::Square square) { return masks[to_underlying(square)]; }

std::array<std::array<U64, 4096>, 64> attacks = {{{0}}};

void init(void) {
    for (uint8_t square = 0; square < 64; square++) {
        const int tr = square / 8, tf = square % 8;
        const int len[4] = {tf - 1, tr - 1, 6 - tf, 6 - tr};

        masks[square] = attack::slider::mask(static_cast<square::Square>(square), C64(0), dir, len);
    }

    for (uint8_t square = 0; square < 64; square++) {
        square::Square Square = static_cast<square::Square>(square);
        U64 attack_mask = mask(Square);
        uint8_t relevant_bits = bit::count(attack_mask);
        U64 occupancy_indices = C64(1) << relevant_bits;

        for (U64 idx = 0; idx < occupancy_indices; idx++) {
            U64 occupancy = attack::slider::occupancy(idx, relevant_bits, attack_mask);
            U32 magic_index = hash(occupancy, Square);
            attacks[square][magic_index] = mask_fly(Square, occupancy);
        }
    }
}

U64 attack(const square::Square square, U64 occupancy) {
    occupancy &= mask(square);
    occupancy = hash(occupancy, square);
    return attacks[to_underlying(square)][occupancy];
}

} // namespace rook
} // namespace attack
