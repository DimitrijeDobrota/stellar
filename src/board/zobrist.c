#include "zobrist.h"
#include "board.h"
#include "piece.h"
#include "random.h"
#include "utils.h"

U64 castle_keys[16];
U64 enpassant_keys[64];
U64 piece_keys[16][64];
U64 side_key;

void init_hash_keys() {
    random_state_reset();

    for (int piece = PAWN; piece <= KING; piece++) {
        int piece_index_white = piece_index(piece_get(piece, WHITE));
        int piece_index_black = piece_index(piece_get(piece, BLACK));
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

U64 zobrist_hash(const Board *board) {
    U64 key_final = C64(0);
    Square square;

    for (int piece = PAWN; piece <= KING; piece++) {
        Piece piece_white = piece_get(piece, WHITE);
        U64 bitboard_white = board_pieceSet(board, piece_white);
        int piece_white_index = piece_index(piece_white);

        bitboard_for_each_bit(square, bitboard_white) {
            key_final ^= piece_keys[piece_white_index][square];
        }

        Piece piece_black = piece_get(piece, BLACK);
        U64 bitboard_black = board_pieceSet(board, piece_black);
        int piece_black_index = piece_index(piece_black);

        bitboard_for_each_bit(square, bitboard_black) {
            key_final ^= piece_keys[piece_black_index][square];
        }
    }

    key_final ^= castle_keys[board_castle(board)];

    if (board_side(board)) key_final ^= side_key;
    if (board_enpassant(board) != no_sq)
        key_final ^= enpassant_keys[board_enpassant(board)];

    return key_final;
}
