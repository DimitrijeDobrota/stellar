#include "board.h"
#include "moves.h"

#define pawn_canPromote(color, source)                                         \
    ((color == WHITE && source >= a7 && source <= h7) ||                       \
     (color == BLACK && source >= a2 && source <= h2))

#define pawn_onStart(color, source)                                            \
    ((color == BLACK && source >= a7 && source <= h7) ||                       \
     (color == WHITE && source >= a2 && source <= h2))

#define pawn_promote(source, target, Piece, Capture)                           \
    for (int i = 1; i < 5; i++) {                                              \
        move = move_encode(source, target, Piece, Capture,                     \
                           piece_get(i, color), 0, 0, 0);                      \
        move_list_add(moves, move);                                            \
    }

MoveList *move_list_generate(MoveList *moves, const Board *board) {
    Move move;
    Square src, tgt;
    eColor color = board_side(board);

    if (!moves)
        moves = move_list_new();
    else
        move_list_reset(moves);

    // pawn moves
    Piece piece = piece_get(PAWN, color);
    U64 bitboard = board_pieceSet(board, piece);
    bitboard_for_each_bit(src, bitboard) {
        { // quiet
            int add = (color == WHITE) ? +8 : -8;
            tgt = src + add;
            if (!board_square_isOccupied(board, tgt)) {
                if (pawn_canPromote(color, src)) {
                    pawn_promote(src, tgt, piece, 0);
                } else {
                    move_list_add(moves,
                                  move_encode(src, tgt, piece, 0, 0, 0, 0, 0));

                    // two ahead
                    if (pawn_onStart(color, src) &&
                        !board_square_isOccupied(board, tgt += add))
                        move_list_add(
                            moves, move_encode(src, tgt, piece, 0, 0, 1, 0, 0));
                }
            }
        }
        { // capture
            U64 attack = board_piece_attacks(board, piece, src) &
                         board_color(board, !color);
            bitboard_for_each_bit(tgt, attack) {
                if (pawn_canPromote(color, src)) {
                    pawn_promote(src, tgt, piece,
                                 board_square_piece(board, tgt, !color));
                } else {
                    move_list_add(moves, move_encode(src, tgt, piece,
                                                     board_square_piece(
                                                         board, tgt, !color),
                                                     0, 0, 0, 0));
                }
            }
        }

        { // en passant
            if (board_enpassant(board) != no_sq &&
                board_piece_attacks(board, piece, src) &
                    (C64(1) << board_enpassant(board)))
                move_list_add(moves,
                              move_encode(src, board_enpassant(board), piece,
                                          piece_get(PAWN, !color), 0, 0, 1, 0));
        }
    }

    // All piece move
    for (int piece_idx = 1; piece_idx < 6; piece_idx++) {
        Piece piece = piece_get(piece_idx, color);
        U64 bitboard = board_pieceSet(board, piece);
        bitboard_for_each_bit(src, bitboard) {
            U64 attack = board_piece_attacks(board, piece, src) &
                         ~board_color(board, color);
            bitboard_for_each_bit(tgt, attack) {
                move_list_add(
                    moves, move_encode(src, tgt, piece,
                                       board_square_piece(board, tgt, !color),
                                       0, 0, 0, 0));
            }
        }
    }

    // Castling
    if (color == WHITE) {
        Piece piece = piece_get(KING, WHITE);
        if (board_castle(board) & WK) {
            if (!board_square_isOccupied(board, f1) &&
                !board_square_isOccupied(board, g1) &&
                !board_square_isAttack(board, e1, BLACK) &&
                !board_square_isAttack(board, f1, BLACK))
                move_list_add(moves, move_encode(e1, g1, piece, 0, 0, 0, 0, 1));
        }
        if (board_castle(board) & WQ) {
            if (!board_square_isOccupied(board, d1) &&
                !board_square_isOccupied(board, c1) &&
                !board_square_isOccupied(board, b1) &&
                !board_square_isAttack(board, e1, BLACK) &&
                !board_square_isAttack(board, d1, BLACK))
                move_list_add(moves, move_encode(e1, c1, piece, 0, 0, 0, 0, 1));
        }
    } else {
        Piece piece = piece_get(KING, BLACK);
        if (board_castle(board) & BK) {
            if (!board_square_isOccupied(board, f8) &&
                !board_square_isOccupied(board, g8) &&
                !board_square_isAttack(board, e8, WHITE) &&
                !board_square_isAttack(board, f8, WHITE))
                move_list_add(moves, move_encode(e8, g8, piece, 0, 0, 0, 0, 1));
        }
        if (board_castle(board) & BQ) {
            if (!board_square_isOccupied(board, d8) &&
                !board_square_isOccupied(board, c8) &&
                !board_square_isOccupied(board, b8) &&
                !board_square_isAttack(board, e8, WHITE) &&
                !board_square_isAttack(board, d8, WHITE))
                move_list_add(moves, move_encode(e8, c8, piece, 0, 0, 0, 0, 1));
        }
    }

    return moves;
}
