#include "evaluate.hpp"
#include "bit.hpp"
#include "bitboard.hpp"
#include "piece.hpp"
#include "score.hpp"
#include "square.hpp"
#include "utils.hpp"

#include <array>

namespace evaluate {

typedef std::array<U64, 8> mask_fr_array;
inline constexpr const mask_fr_array mask_rank = []() constexpr -> mask_fr_array {
    mask_fr_array mask_rank;
    U64 mask = 0xFF;
    for (uint8_t rank = 0; rank < 8; rank++) {
        mask_rank[rank] = mask;
        mask = bitboard::nortOne(mask);
    }
    return mask_rank;
}();

inline constexpr const mask_fr_array mask_file = []() constexpr -> mask_fr_array {
    mask_fr_array mask_file;
    U64 mask = 0x0101010101010101;
    for (uint8_t file = 0; file < 8; file++) {
        mask_file[file] = mask;
        mask = bitboard::eastOne(mask);
    }
    return mask_file;
}();

inline constexpr const mask_fr_array mask_isolated = []() constexpr -> mask_fr_array {
    mask_fr_array mask_isolated;

    mask_isolated[0] = 0x0202020202020202;

    U64 mask = 0x0505050505050505;
    for (uint8_t file = 1; file < 8; file++) {
        mask_isolated[file] = mask;
        mask = bitboard::eastOne(mask);
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
            maskW = bitboard::nortOne(maskW);
            mask_passed[0][rank * 8 + file] = maskW;

            maskB = bitboard::soutOne(maskB);
            mask_passed[1][(7 - rank) * 8 + file] = maskB;
        }
    }

    return mask_passed;
}();

using piece::Type::BISHOP;
using piece::Type::KING;
using piece::Type::KNIGHT;
using piece::Type::PAWN;
using piece::Type::QUEEN;
using piece::Type::ROOK;

int16_t score_position_side(const Board &board, const color::Color side) {
    U64 bitboard;
    int16_t total = 0;
    int8_t square_i;

    const uint8_t side_i = to_underlying(side);
    const U64 pawns = board.get_bitboard_piece(PAWN);
    const U64 pawnsS = board.get_bitboard_piece(PAWN, side);
    const U64 pawnsO = pawns & ~pawnsS;

    bitboard = board.get_bitboard_piece(PAWN, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const square::Square square = static_cast<square::Square>(square_i);
        total += score::get(PAWN, side, square);
        total += score::get(PAWN);

        // check isolated, doubled and passed pawns
        const uint8_t file = square::file(square), rank = square::rank(square);
        if (!(mask_isolated[file] & pawnsS)) total -= score::pawn_isolated;
        if (bit::count(pawnsS & mask_file[file]) > 1) total -= score::pawn_double;
        if (!(pawnsO & mask_passed[side_i][square_i])) total += score::pawn_passed[side_i][rank];
    }

    bitboard = board.get_bitboard_piece(KNIGHT, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const square::Square square = static_cast<square::Square>(square_i);
        total += score::get(KNIGHT, side, square);
        total += score::get(KNIGHT);
    }

    bitboard = board.get_bitboard_piece(BISHOP, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const square::Square square = static_cast<square::Square>(square_i);
        total += score::get(BISHOP, side, square);
        total += score::get(BISHOP);
    }

    bitboard = board.get_bitboard_piece(ROOK, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const square::Square square = static_cast<square::Square>(square_i);
        total += score::get(ROOK, side, square);
        total += score::get(ROOK);

        // rook on open and semi-open files
        const uint8_t file = square::file(square);
        if (!(pawns & mask_file[file])) total += score::score_open;
        if (!(pawnsS & mask_file[file])) total += score::score_open_semi;
    }

    bitboard = board.get_bitboard_piece(QUEEN, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const square::Square square = static_cast<square::Square>(square_i);
        total += score::get(QUEEN, side, square);
        total += score::get(QUEEN);
    }

    bitboard = board.get_bitboard_piece(KING, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const square::Square square = static_cast<square::Square>(square_i);
        total += score::get(KING, side, square);
        total += score::get(KING);

        // king on open and semi-open files
        const uint8_t file = square::file(square);
        if (!(pawns & mask_file[file])) total -= score::score_open;
        if (!(pawnsS & mask_file[file])) total -= score::score_open_semi;
    }

    return total;
}

int16_t score_position(const Board &board) {
    const int16_t score = score_position_side(board, color::WHITE) - score_position_side(board, color::BLACK);
    return board.get_side() == color::WHITE ? score : -score;
}

} // namespace evaluate
