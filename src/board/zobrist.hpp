#ifndef STELLAR_ZOBRIST_H
#define STELLAR_ZOBRIST_H

#include "piece.hpp"
#include "random.hpp"

#include <algorithm>
#include <array>
#include <random>

class Board;
namespace zobrist {

extern std::array<std::array<U64, 64>, 12> keys_piece;
extern std::array<U64, 64> keys_enpassant;
extern std::array<U64, 16> keys_castle;

const U64 keys_side = Random(C32(1699391443))();

inline void init() {
    Random gen1(C64(1804289383));
    for (piece::Type type : piece::TypeIter()) {
        int piece_index_white = piece::get(type, color::Color::WHITE).index;
        int piece_index_black = piece::get(type, color::Color::BLACK).index;
        for (int square = 0; square < 64; square++) {
            keys_piece[piece_index_white][square] = gen1();
            keys_piece[piece_index_black][square] = gen1();
        }
    }

    Random gen2(C32(337245213));
    for (int castle = 0; castle < 64; castle++) {
        keys_enpassant[castle] = gen2();
    }

    Random gen3(C32(3642040919));
    for (int castle = 0; castle < 16; castle++) {
        keys_castle[castle] = gen3();
    }
};

inline U64 hash(const Board &board);
inline constexpr U64 key_side() { return keys_side; }
inline constexpr U64 key_castle(int exp) { return keys_castle[exp]; }
inline constexpr U64 key_enpassant(square::Square square) { return keys_enpassant[to_underlying(square)]; }
inline constexpr U64 key_piece(piece::Type type, color::Color color, square::Square square) {
    return keys_piece[piece::get_index(type, color)][to_underlying(square)];
}

}; // namespace zobrist

#endif
