#include "evaluate.hpp"
#include "utils.hpp"
#include <array>

namespace evaluate {

typedef std::array<U64, 8> mask_fr_array;
inline constexpr const mask_fr_array mask_rank = []() constexpr -> mask_fr_array {
    mask_fr_array mask_rank;
    U64 mask = 0xFF;
    for (uint8_t rank = 0; rank < 8; rank++) {
        mask_rank[rank] = mask;
        mask = nortOne(mask);
    }
    return mask_rank;
}();

inline constexpr const mask_fr_array mask_file = []() constexpr -> mask_fr_array {
    mask_fr_array mask_file;
    U64 mask = 0x0101010101010101;
    for (uint8_t file = 0; file < 8; file++) {
        mask_file[file] = mask;
        mask = eastOne(mask);
    }
    return mask_file;
}();

inline constexpr const mask_fr_array mask_isolated = []() constexpr -> mask_fr_array {
    mask_fr_array mask_isolated;

    mask_isolated[0] = 0x0202020202020202;

    U64 mask = 0x0505050505050505;
    for (uint8_t file = 1; file < 8; file++) {
        mask_isolated[file] = mask;
        mask = eastOne(mask);
    }

    return mask_isolated;
}();

typedef std::array<U64, 64> mask_passed_array;
inline constexpr const mask_passed_array mask_passed_white = []() constexpr -> mask_passed_array {
    mask_passed_array mask_passed_white;
    for (uint8_t file = 0; file < 8; file++) {
        U64 mask = mask_file[file] | mask_isolated[file];
        for (uint8_t rank = 0; rank < 8; rank++) {
            mask = nortOne(mask);
            mask_passed_white[rank * 8 + file] = mask;
        }
    }
    return mask_passed_white;
}();

inline constexpr const mask_passed_array mask_passed_black = []() constexpr -> mask_passed_array {
    mask_passed_array mask_passed_black;
    for (uint8_t file = 0; file < 8; file++) {
        U64 mask = mask_file[file] | mask_isolated[file];
        for (int8_t rank = 7; rank >= 0; rank--) {
            mask = soutOne(mask);
            mask_passed_black[rank * 8 + file] = mask;
        }
    }
    return mask_passed_black;
}();

inline constexpr uint8_t get_file(uint8_t square) { return square & 0x07; }
inline constexpr uint8_t get_rank(uint8_t square) { return square >> 3; }

inline constexpr const int8_t penalty_pawn_double = -10;
inline constexpr const int8_t penalty_pawn_isolated = -10;

inline constexpr const std::array<int16_t, 8> bonus_pawn_passed = {0, 10, 30, 50, 75, 100, 150, 200};

using piece::Type::PAWN;

int16_t score_position(const Board &board) {
    uint8_t square_i;
    int16_t score = 0;

    // Score all White pieces except pawns
    for (const piece::Type type : ++piece::TypeIter()) {
        U64 bitboard = board.get_bitboard_piece(type, Color::WHITE);
        bitboard_for_each_bit(square_i, bitboard) {
            Square square = static_cast<Square>(square_i);
            score += piece::score(type);
            score += piece::score(type, Color::WHITE, square);
        }
    }

    // Score all Black pieces except pawns
    for (const piece::Type type : ++piece::TypeIter()) {
        U64 bitboard = board.get_bitboard_piece(type, Color::BLACK);
        bitboard_for_each_bit(square_i, bitboard) {
            Square square = static_cast<Square>(square_i);
            score -= piece::score(type);
            score -= piece::score(type, Color::BLACK, square);
        }
    }

    U64 pawns = board.get_bitboard_piece(PAWN);
    U64 pawnsW = board.get_bitboard_piece(PAWN, Color::WHITE);
    U64 pawnsB = board.get_bitboard_piece(PAWN, Color::BLACK);

    for (uint8_t file = 0; file < 8; file++) {
        // check doubled and isolated pawns
        U64 pawns_file = pawns & mask_file[file];

        uint8_t pawnsW_count = bit_count(pawns_file & pawnsW);
        if (pawnsW_count > 1) score += (pawnsW_count - 1) * penalty_pawn_double;
        if (!(mask_isolated[file] & pawnsW)) score += pawnsW_count * penalty_pawn_isolated;

        uint8_t pawnsB_count = bit_count(pawns_file & pawnsB);
        if (pawnsB_count > 1) score -= (pawnsB_count - 1) * penalty_pawn_double;
        if (!(mask_isolated[file] & pawnsB)) score -= pawnsB_count * penalty_pawn_isolated;
    }

    // Score White pawns, bonus for passed
    bitboard_for_each_bit(square_i, pawnsW) {
        Square square = static_cast<Square>(square_i);
        score += piece::score(PAWN);
        score += piece::score(PAWN, Color::WHITE, square);
        if (!(pawnsB & mask_passed_white[square_i])) score += bonus_pawn_passed[get_rank(square_i)];
    }

    // previous bitboard_for_each_bit consumed this mask
    pawnsW = board.get_bitboard_piece(PAWN, Color::WHITE);

    // Score White pawns, bonus for passed
    bitboard_for_each_bit(square_i, pawnsB) {
        Square square = static_cast<Square>(square_i);
        score -= piece::score(PAWN);
        score -= piece::score(PAWN, Color::BLACK, square);
        if (!(pawnsW & mask_passed_black[square_i])) score -= bonus_pawn_passed[7 - get_rank(square_i)];
    }

    return board.get_side() == Color::WHITE ? score : -score;
}

} // namespace evaluate
