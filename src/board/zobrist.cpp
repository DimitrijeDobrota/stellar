#include "zobrist.hpp"
#include "piece.hpp"
#include "random.hpp"
#include "utils_cpp.hpp"

U64 castle_keys[16];
U64 enpassant_keys[64];
U64 piece_keys[12][64];
U64 side_key;

U64 zobrist_key_side(void) { return side_key; }
U64 zobrist_key_castle(int exp) { return castle_keys[exp]; }
U64 zobrist_key_enpassant(Square square) {
    return enpassant_keys[to_underlying(square)];
}
U64 zobrist_key_piece(const piece::Piece &piece, Square square) {
    return piece_keys[piece.index][to_underlying(square)];
}

void init_hash_keys() {
    random_state_reset();

    for (piece::Type type : piece::TypeIter()) {
        int piece_index_white = piece::get(type, Color::WHITE).index;
        int piece_index_black = piece::get(type, Color::BLACK).index;
        for (int square = 0; square < 64; square++) {
            piece_keys[piece_index_white][square] = random_get_U64();
            piece_keys[piece_index_black][square] = random_get_U64();
        }
    }

    for (int square = 0; square < 64; square++) {
        enpassant_keys[square] = random_get_U64();
    }

    for (int castle = 0; castle < 16; castle++) {
        castle_keys[castle] = random_get_U64();
    }

    side_key = random_get_U64();
}

void zobrist_init(void) { init_hash_keys(); }

U64 zobrist_hash(const Board &board) {
    U64 key_final = C64(0);
    uint8_t square;

    for (piece::Type type : piece::TypeIter()) {
        const piece::Piece &piece_white = piece::get(type, Color::WHITE);
        int piece_white_index = piece_white.index;
        U64 bitboard_white = board.get_bitboard_piece(piece_white);

        bitboard_for_each_bit(square, bitboard_white) {
            key_final ^= piece_keys[piece_white_index][square];
        }

        const piece::Piece &piece_black = piece::get(type, Color::BLACK);
        int piece_black_index = piece_black.index;
        U64 bitboard_black = board.get_bitboard_piece(piece_black);

        bitboard_for_each_bit(square, bitboard_black) {
            key_final ^= piece_keys[piece_black_index][square];
        }
    }

    key_final ^= castle_keys[board.get_castle()];

    if (board.get_side() == Color::BLACK) key_final ^= side_key;
    if (board.get_enpassant() != Square::no_sq)
        key_final ^= enpassant_keys[to_underlying(board.get_enpassant())];

    return key_final;
}
