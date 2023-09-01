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

typedef std::array<std::array<U64, 64>, 2> mask_passed_array;
inline constexpr const mask_passed_array mask_passed = []() constexpr -> mask_passed_array {
    mask_passed_array mask_passed;

    for (uint8_t file = 0; file < 8; file++) {
        U64 mask = mask_file[file] | mask_isolated[file];
        for (uint8_t rank = 0; rank < 8; rank++) {
            mask = nortOne(mask);
            mask_passed[0][rank * 8 + file] = mask;
        }
    }

    for (uint8_t file = 0; file < 8; file++) {
        U64 mask = mask_file[file] | mask_isolated[file];
        for (int8_t rank = 7; rank >= 0; rank--) {
            mask = soutOne(mask);
            mask_passed[1][rank * 8 + file] = mask;
        }
    }

    return mask_passed;
}();

inline constexpr uint8_t get_file(uint8_t square) { return square & 0x07; }
inline constexpr uint8_t get_rank(uint8_t square) { return square >> 3; }

inline constexpr const int8_t penalty_pawn_double = -10;
inline constexpr const int8_t penalty_pawn_isolated = -10;

inline constexpr const int8_t score_open_semi = 10;
inline constexpr const int8_t score_open = 15;

inline constexpr const std::array<std::array<int16_t, 8>, 2> bonus_pawn_passed = {
    {{0, 10, 30, 50, 75, 100, 150, 200}, {200, 150, 100, 75, 50, 30, 10, 0}}};

using piece::Type::KING;
using piece::Type::PAWN;
using piece::Type::ROOK;

int16_t score_position_side(const Board &board, Color side) {
    static U64 bitboard;
    uint8_t square_i;
    int16_t score = 0;

    // Score all pieces except pawns
    for (const piece::Type type : ++piece::TypeIter()) {
        bitboard = board.get_bitboard_piece(type, side);
        bitboard_for_each_bit(square_i, bitboard) {
            Square square = static_cast<Square>(square_i);
            score += piece::score(type);
            score += piece::score(type, side, square);
        }
    }

    const U64 pawns = board.get_bitboard_piece(PAWN);
    const U64 pawnsS = board.get_bitboard_piece(PAWN, side);
    const U64 rookS = board.get_bitboard_piece(ROOK, side);
    const U64 kingS = board.get_bitboard_piece(KING, side);

    for (uint8_t file = 0; file < 8; file++) {
        // check doubled and isolated pawns

        uint8_t pawnsS_count = bit_count(pawnsS & mask_file[file]);
        uint8_t pawns_count = bit_count(pawns & mask_file[file]);

        if (pawnsS_count > 1) score += (pawnsS_count - 1) * penalty_pawn_double;
        if (!(mask_isolated[file] & pawnsS)) score += pawnsS_count * penalty_pawn_isolated;

        // rooks on open and semi-open files
        if (rookS & mask_file[file]) {
            if (!pawns_count) score += score_open;
            if (!pawnsS_count) score += score_open_semi;
        }

        // king on open and semi-open files
        if (kingS & mask_file[file]) {
            if (!pawns_count) score -= score_open;
            if (!pawnsS_count) score -= score_open_semi;
        }
    }

    // Score pawns, bonus for passed
    bitboard = pawnsS;
    bitboard_for_each_bit(square_i, bitboard) {
        Square square = static_cast<Square>(square_i);
        score += piece::score(PAWN);
        score += piece::score(PAWN, side, square);
        if (!(pawns & ~pawnsS & mask_passed[to_underlying(side)][square_i]))
            score += bonus_pawn_passed[to_underlying(side)][get_rank(square_i)];
    }

    return score;
}

int16_t score_position(const Board &board) {
    const int16_t score = score_position_side(board, Color::WHITE) - score_position_side(board, Color::BLACK);
    return board.get_side() == Color::WHITE ? score : -score;
}

} // namespace evaluate
