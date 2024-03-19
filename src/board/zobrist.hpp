#ifndef STELLAR_ZOBRIST_H
#define STELLAR_ZOBRIST_H

#include "piece.hpp"
#include "random.hpp"

#include <algorithm>
#include <array>
#include <random>

class Board;
namespace zobrist {

extern U32 keys_pawn[2][64];
extern U64 keys_piece[2][12][64];
extern U64 keys_enpassant[64];
extern U64 keys_castle[16];

const U64 keys_side = Random(C32(1699391443))();

inline void init() {
    Random gen1(C64(1804289383));
    for (Type type = PAWN; type <= KING; ++type) {
        for (int square = 0; square < 64; square++) {
            keys_piece[WHITE][type][square] = gen1();
            keys_piece[BLACK][type][square] = gen1();
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

    Random gen4(C32(3642040919));
    for (int c = 0; c < 2; c++) {
        for (int square = 0; square < 64; square++) {
            keys_pawn[c][square] = gen4.get_U32();
        }
    }
};

inline U64 hash(const Board &board);
inline U32 hash_pawn(const Board &board);

inline constexpr U64 key_side() { return keys_side; }
inline constexpr U64 key_castle(int exp) { return keys_castle[exp]; }
inline constexpr U64 key_enpassant(Square square) { return keys_enpassant[square]; }
inline constexpr U64 key_pawn(Color color, Square square) { return keys_pawn[color][square]; }
inline constexpr U64 key_piece(Type type, Color color, Square square) {
    return keys_piece[color][type][square];
}

}; // namespace zobrist

#endif
