#include "evaluate.hpp"
#include "bit.hpp"
#include "bitboard.hpp"
#include "piece.hpp"
#include "score.hpp"
#include "utils.hpp"

#include <array>
#include <vector>
#include <numeric>

namespace evaluate {

using mask_fr_array = std::array<U64, 8>;
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

using mask_passed_array = std::array<std::array<U64, 64>, 2>;
inline constexpr const mask_passed_array mask_passed = []() constexpr -> mask_passed_array {
    mask_passed_array mask_passed;

    U64 maskW = 0, maskB = 0;
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

struct Hashe {
    U32 key;
    int16_t opening;
    int16_t endgame;
    int16_t total;
};

template <U32 SIZE> struct PTable {

    static void clear() { memset(table, 0x00, SIZE * sizeof(Hashe)); }

    [[nodiscard]] static inline U32 read(U32 hash, Color side) {
        const U32 key = side * SIZE + hash % SIZE;
        const Hashe &phashe = table[key];
        return phashe.key == hash ? key : std::numeric_limits<uint32_t>::max();
    }

    [[nodiscard]] static inline int16_t read_opening(U32 key) { return table[key].opening; }
    [[nodiscard]] static inline int16_t read_endgame(U32 key) { return table[key].endgame; }
    [[nodiscard]] static inline int16_t read_total(U32 key) { return table[key].total; }

    static inline void write(U32 hash, Color side, int16_t opening, int16_t endgame, int16_t total) {
        table[side * SIZE + hash % SIZE] = {hash, opening, endgame, total};
    }

  private:
    static std::array<Hashe, 2 * SIZE> table;
};

template <U32 SIZE> std::array<Hashe, 2 * SIZE> PTable<SIZE>::table = {{{0}}};

PTable<65537> ptable;

uint16_t score_game_phase(const Board &board) {
    int16_t total = 0;
    for (int type_i = KNIGHT; type_i < KING; type_i++) {
        const Type type = static_cast<Type>(type_i);
        total += bit::count(board.get_bitboard_piece(type)) * score::get(type);
    }
    return total;
}

int16_t score_position_side(const Board &board, const Color side, const uint16_t phase_score) {
    using score::Phase::ENDGAME;
    using score::Phase::OPENING;

    U64 bitboard;
    int8_t square_i;

    const U64 pawns = board.get_bitboard_piece(PAWN);
    const U64 pawnsS = board.get_bitboard_piece(PAWN, side);
    const U64 pawnsO = pawns & ~pawnsS;

    int16_t total = 0, opening = 0, endgame = 0;

    const U32 hash = board.get_hash_pawn();
    const U32 key = ptable.read(hash, side);
    if (key == std::numeric_limits<uint32_t>::max()) {
        bitboard = board.get_bitboard_piece(PAWN, side);
        bitboard_for_each_bit(square_i, bitboard) {
            const auto square = static_cast<Square>(square_i);
            opening += score::get(PAWN, side, square, OPENING) + score::get(PAWN, OPENING);
            endgame += score::get(PAWN, side, square, ENDGAME) + score::get(PAWN, ENDGAME);

            // check isolated, doubled and passed pawns
            const uint8_t file = get_file(square), rank = get_rank(square);
            if (!(mask_isolated[file] & pawnsS)) {
                opening -= score::pawn_isolated_opening;
                endgame -= score::pawn_isolated_endgame;
            }

            if (bit::count(pawnsS & mask_file[file]) > 1) {
                opening -= score::pawn_double_opening;
                endgame -= score::pawn_double_endgame;
            }

            if (!(pawnsO & mask_passed[side][square_i])) total += score::pawn_passed[side][rank];
        }

        ptable.write(hash, side, opening, endgame, total);
    } else {
        opening = ptable.read_opening(key);
        endgame = ptable.read_endgame(key);
        total = ptable.read_total(key);
    }

    bitboard = board.get_bitboard_piece(KNIGHT, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const auto square = static_cast<Square>(square_i);
        opening += score::get(KNIGHT, side, square, OPENING) + score::get(KNIGHT, OPENING);
        endgame += score::get(KNIGHT, side, square, ENDGAME) + score::get(KNIGHT, ENDGAME);
    }

    bitboard = board.get_bitboard_piece(BISHOP, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const auto square = static_cast<Square>(square_i);
        opening += score::get(BISHOP, side, square, OPENING) + score::get(BISHOP, OPENING);
        endgame += score::get(BISHOP, side, square, ENDGAME) + score::get(BISHOP, ENDGAME);
    }

    bitboard = board.get_bitboard_piece(ROOK, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const auto square = static_cast<Square>(square_i);
        opening += score::get(ROOK, side, square, OPENING) + score::get(ROOK, OPENING);
        endgame += score::get(ROOK, side, square, ENDGAME) + score::get(ROOK, ENDGAME);

        // rook on open and semi-open files
        const uint8_t file = get_file(square);
        if (!(pawns & mask_file[file])) total += score::file_open;
        if (!(pawnsS & mask_file[file])) total += score::file_open_semi;
    }

    bitboard = board.get_bitboard_piece(QUEEN, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const auto square = static_cast<Square>(square_i);
        opening += score::get(QUEEN, side, square, OPENING) + score::get(QUEEN, OPENING);
        endgame += score::get(QUEEN, side, square, ENDGAME) + score::get(QUEEN, ENDGAME);
    }

    bitboard = board.get_bitboard_piece(KING, side);
    bitboard_for_each_bit(square_i, bitboard) {
        const auto square = static_cast<Square>(square_i);
        opening += score::get(KING, side, square, OPENING) + score::get(KING, OPENING);
        endgame += score::get(KING, side, square, ENDGAME) + score::get(KING, ENDGAME);

        // king on open and semi-open files
        const uint8_t file = get_file(square);
        if (!(pawns & mask_file[file])) total -= score::file_open;
        if (!(pawnsS & mask_file[file])) total -= score::file_open_semi;
    }

    if (phase_score > score::phase_opening) return opening + total;
    if (phase_score < score::phase_endgame) return endgame + total;
    return score::interpolate(phase_score, opening, endgame) + total;
}

int16_t score_position(const Board &board) {
    const uint16_t phase_score = score_game_phase(board);
    const int16_t score =
        score_position_side(board, WHITE, phase_score) - score_position_side(board, BLACK, phase_score);
    return board.get_side() == WHITE ? score : -score;
}

} // namespace evaluate
