#ifndef STELLAR_ZOBRIST_H
#define STELLAR_ZOBRIST_H

#include "piece.hpp"
#include "random.hpp"

#include <algorithm>
#include <array>
#include <random>

class Board;
class Zobrist {
  public:
    Zobrist() = delete;

    static inline U64 hash(const Board &board);
    static inline constexpr U64 key_side(void) { return keys_side; }
    static inline constexpr U64 key_castle(int exp) { return keys_castle[exp]; }
    static inline constexpr U64 key_enpassant(square::Square square) {
        return keys_enpassant[to_underlying(square)];
    }
    static inline constexpr U64 key_piece(piece::Type type, color::Color color, square::Square square) {
        return keys_piece[piece::get_index(type, color)][to_underlying(square)];
    }

  private:
    typedef std::array<std::array<U64, 64>, 12> key_piece_array;
    static inline constexpr const key_piece_array keys_piece = []() constexpr -> key_piece_array {
        key_piece_array key_piece;
        Random gen(C64(1804289383));
        for (piece::Type type : piece::TypeIter()) {
            int piece_index_white = piece::get(type, color::Color::WHITE).index;
            int piece_index_black = piece::get(type, color::Color::BLACK).index;
            for (int square = 0; square < 64; square++) {
                key_piece[piece_index_white][square] = gen();
                key_piece[piece_index_black][square] = gen();
            }
        }
        return key_piece;
    }();

    typedef std::array<U64, 64> key_enpassant_array;
    static inline constexpr const key_enpassant_array keys_enpassant = []() constexpr -> key_enpassant_array {
        key_enpassant_array key_enpassant;
        Random gen(C32(337245213));
        for (int castle = 0; castle < 64; castle++) {
            key_enpassant[castle] = gen();
        }
        return key_enpassant;
    }();

    typedef std::array<U64, 16> key_castle_array;
    static inline constexpr const key_castle_array keys_castle = []() constexpr -> key_castle_array {
        key_castle_array key_castle;
        Random gen(C32(3642040919));
        for (int castle = 0; castle < 16; castle++) {
            key_castle[castle] = gen();
        }
        return key_castle;
    }();

    static inline constexpr const U64 keys_side = Random(C32(1699391443))();
};

#endif
