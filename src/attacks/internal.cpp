#include "internal.hpp"
#include "utils_cpp.hpp"

#include <algorithm> // std::min

U64 king_attacks[64];
U64 knight_attacks[64];
U64 pawn_attacks[2][64];
U64 rook_attacks[64][4096];  // 2048K
U64 bishop_attacks[64][512]; // 256 K

U64 rook_masks[64];
U64 bishop_masks[64];

// clang-format off
const int bishop_relevant_bits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6,
};

const int rook_relevant_bits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12,
};
// clang-format on

int hash(U64 key, U64 magic, int relevant_bits) {
    return (key * magic) >> (64 - relevant_bits);
}

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = C64(0);

    for (int count = 0; count < bits_in_mask; count++) {
        uint8_t square = bit_lsb_index(attack_mask);
        bit_pop(attack_mask, square);

        if (index & (1 << count)) bit_set(occupancy, square);
    }

    return occupancy;
}

U64 attacks_slide_mask(Square square, U64 block, const direction_f dir[4],
                       int len[4]) {
    U64 bitboard = C64(0), attacks = C64(0), tmp;
    int i, j;

    bit_set(bitboard, to_underlying(square));
    for (i = 0; i < 4; i++) {
        for (j = 0, tmp = bitboard; j < len[i]; j++) {
            attacks |= tmp = (dir[i])(tmp);
            if (tmp & block) break;
        }
    }
    return attacks;
}

// Mask Attacks

const direction_f attacks_bishop_direction[4] = {noEaOne, noWeOne, soEaOne,
                                                 soWeOne};
const direction_f attacks_rook_direction[4] = {westOne, soutOne, eastOne,
                                               nortOne};

U64 pawn_mask(Color side, Square square) {
    U64 bitboard = C64(0);

    bit_set(bitboard, to_underlying(square));
    if (side == Color::WHITE)
        return noWeOne(bitboard) | noEaOne(bitboard);
    else
        return soWeOne(bitboard) | soEaOne(bitboard);
}

U64 knight_mask(Square square) {
    U64 bitboard = C64(0), attacks = C64(0), tmp;

    bit_set(bitboard, to_underlying(square));
    tmp = nortOne(nortOne(bitboard));
    attacks |= westOne(tmp) | eastOne(tmp);
    tmp = soutOne(soutOne(bitboard));
    attacks |= westOne(tmp) | eastOne(tmp);
    tmp = westOne(westOne(bitboard));
    attacks |= soutOne(tmp) | nortOne(tmp);
    tmp = eastOne(eastOne(bitboard));
    attacks |= soutOne(tmp) | nortOne(tmp);

    return attacks;
}

U64 king_mask(Square square) {
    U64 bitboard = C64(0), attacks = C64(0);

    bit_set(bitboard, to_underlying(square));
    attacks |= westOne(bitboard) | eastOne(bitboard);
    attacks |= soutOne(bitboard) | nortOne(bitboard);
    attacks |= soutOne(bitboard) | nortOne(bitboard);
    attacks |= soEaOne(bitboard) | noEaOne(bitboard);
    attacks |= soWeOne(bitboard) | noWeOne(bitboard);

    return attacks;
}

U64 bishop_mask(Square square) {
    uint8_t square_i = to_underlying(square);
    int tr = square_i / 8, tf = square_i % 8;
    int len[4] = {std::min(7 - tf, 7 - tr) - 1, std::min(tf, 7 - tr) - 1,
                  std::min(7 - tf, tr) - 1, std::min(tf, tr) - 1};
    return attacks_slide_mask(square, C64(0), attacks_bishop_direction, len);
}

U64 rook_mask(Square square) {
    uint8_t square_i = to_underlying(square);
    int tr = square_i / 8, tf = square_i % 8;
    int len[4] = {tf - 1, tr - 1, 6 - tf, 6 - tr};

    return attacks_slide_mask(square, C64(0), attacks_rook_direction, len);
}

U64 bishop_on_the_fly(Square square, U64 block) {
    uint8_t square_i = to_underlying(square);
    int tr = square_i / 8, tf = square_i % 8;
    int len[4] = {std::min(7 - tf, 7 - tr), std::min(tf, 7 - tr),
                  std::min(7 - tf, tr), std::min(tf, tr)};

    return attacks_slide_mask(square, block, attacks_bishop_direction, len);
}

U64 rook_on_the_fly(Square square, U64 block) {
    uint8_t square_i = to_underlying(square);
    int tr = square_i / 8, tf = square_i % 8;
    int len[4] = {tf, tr, 7 - tf, 7 - tr};

    return attacks_slide_mask(square, block, attacks_rook_direction, len);
}
