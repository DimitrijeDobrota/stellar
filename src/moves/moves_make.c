#include "board.h"
#include "moves.h"
#include "zobrist.h"

// clang-format off
const int castling_rights[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7,  15, 15, 15, 3,  15, 15, 11,
};
// clang-format on

void _piece_remove(Board *self, Piece piece, Square square) {
    board_piece_pop(self, piece, square);
    self->hash ^= zobrist_key_piece(piece, square);
}

void _piece_set(Board *self, Piece piece, Square square) {
    board_piece_set(self, piece, square);
    self->hash ^= zobrist_key_piece(piece, square);
}

void _piece_move(Board *self, Piece piece, Square source, Square target) {
    _piece_remove(self, piece, source);
    _piece_set(self, piece, target);
}

int move_make(Move move, Board *board, int flag) {
    if (flag) {
        if (move_capture(move)) return move_make(move, board, 0);
        return 0;
    } else {
        Piece piece = move_piece(move);
        eColor color = board_side(board);
        Square source = move_source(move);
        Square target = move_target(move);
        Square ntarget = target + (color == WHITE ? -8 : +8);

        if (!move_capture(move)) {
            if (move_promote(move)) {
                _piece_remove(board, piece, source);
                _piece_set(board, move_piece_promote(move), target);
            } else {
                _piece_move(board, piece, source, target);
            }
        } else {
            if (move_enpassant(move)) {
                _piece_move(board, piece, source, target);
                _piece_remove(board, move_piece_capture(move), ntarget);
            } else if (move_promote(move)) {
                _piece_remove(board, piece, source);
                _piece_remove(board, move_piece_capture(move), target);
                _piece_set(board, move_piece_promote(move), target);
            } else {
                _piece_remove(board, piece, source);
                _piece_remove(board, move_piece_capture(move), target);
                _piece_set(board, piece, target);
            }
        }

        board_enpassant_set(board, move_double(move) ? ntarget : no_sq);

        if (move_castle(move)) {
            Piece Rook = piece_get(ROOK, board_side(board));
            switch (target) {
            case g1:
                _piece_move(board, Rook, h1, f1);
                break;
            case c1:
                _piece_move(board, Rook, a1, d1);
                break;
            case g8:
                _piece_move(board, Rook, h8, f8);
                break;
            case c8:
                _piece_move(board, Rook, a8, d8);
                break;
            default:
                break;
            }
        }

        board->hash ^= zobrist_key_castle(board_castle(board));
        board_castle_and(board, castling_rights[source]);
        board_castle_and(board, castling_rights[target]);
        board->hash ^= zobrist_key_castle(board_castle(board));

        if (!board_isCheck(board)) {
            board_side_switch(board);
            return 1;
        }
        return 0;
    }
}
