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

    U64 maskW, maskB;
    for (uint8_t file = 0; file < 8; file++) {
        maskW = maskB = mask_file[file] | mask_isolated[file];
        for (uint8_t rank = 0; rank < 8; rank++) {
            maskW = nortOne(maskW);
            mask_passed[0][rank * 8 + file] = maskW;

            maskB = soutOne(maskB);
            mask_passed[1][(7 - rank) * 8 + file] = maskB;
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

using piece::Type::BISHOP;
using piece::Type::KING;
using piece::Type::KNIGHT;
using piece::Type::PAWN;
using piece::Type::QUEEN;
using piece::Type::ROOK;

int16_t score_position_side(const Board &board, const Color side) {
    U64 bitboard;
    int16_t score = 0;
    int8_t square_i;

    const uint8_t side_i = to_underlying(side);
    const U64 pawns = board.get_bitboard_piece(PAWN);
    const U64 pawnsS = board.get_bitboard_piece(PAWN, side);
    const U64 pawnsO = pawns & ~pawnsS;

    bitboard = board.get_bitboard_piece(PAWN, side);
    bitboard_for_each_bit(square_i, bitboard) {
        score += piece::score(PAWN);
        score += piece::score(PAWN, side, square_i);

        // check isolated, doubled and passed pawns
        const uint8_t file = get_file(square_i), rank = get_rank(square_i);
        if (!(mask_isolated[file] & pawnsS)) score += penalty_pawn_isolated;
        if (bit_count(pawnsS & mask_file[file]) > 1) score += penalty_pawn_double;
        if (!(pawnsO & mask_passed[side_i][square_i])) score += bonus_pawn_passed[side_i][rank];
    }

    bitboard = board.get_bitboard_piece(KNIGHT, side);
    bitboard_for_each_bit(square_i, bitboard) {
        score += piece::score(KNIGHT);
        score += piece::score(KNIGHT, side, square_i);
    }

    bitboard = board.get_bitboard_piece(BISHOP, side);
    bitboard_for_each_bit(square_i, bitboard) {
        score += piece::score(BISHOP);
        score += piece::score(BISHOP, side, square_i);
    }

    bitboard = board.get_bitboard_piece(ROOK, side);
    bitboard_for_each_bit(square_i, bitboard) {
        score += piece::score(ROOK);
        score += piece::score(ROOK, side, square_i);

        // rook on open and semi-open files
        const uint8_t file = get_file(square_i);
        if (!(pawns & mask_file[file])) score += score_open;
        if (!(pawnsS & mask_file[file])) score += score_open_semi;
    }

    bitboard = board.get_bitboard_piece(QUEEN, side);
    bitboard_for_each_bit(square_i, bitboard) {
        score += piece::score(QUEEN);
        score += piece::score(QUEEN, side, square_i);
    }

    bitboard = board.get_bitboard_piece(KING, side);
    bitboard_for_each_bit(square_i, bitboard) {
        score += piece::score(KING);
        score += piece::score(KING, side, square_i);

        // king on open and semi-open files
        const uint8_t file = get_file(square_i);
        if (!(pawns & mask_file[file])) score -= score_open;
        if (!(pawnsS & mask_file[file])) score -= score_open_semi;
    }

    return score;
}

int16_t score_position(const Board &board) {
    const int16_t score = score_position_side(board, Color::WHITE) - score_position_side(board, Color::BLACK);
    return board.get_side() == Color::WHITE ? score : -score;
}

} // namespace evaluate
