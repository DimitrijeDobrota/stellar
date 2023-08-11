#include "board.hpp"
#include "moves.hpp"
#include "zobrist.hpp"

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

void _piece_remove(Board &board, const piece::Piece &piece, Square square) {
    board.pop_piece(piece, square);
    board.xor_hash(zobrist_key_piece(piece, square));
}

void _piece_set(Board &board, const piece::Piece &piece, Square square) {
    board.set_piece(piece, square);
    board.xor_hash(zobrist_key_piece(piece, square));
}

void _piece_move(Board &board, const piece::Piece &piece, Square source, Square target) {
    _piece_remove(board, piece, source);
    _piece_set(board, piece, target);
}

int move_make(Move move, Board &board, int flag) {
    if (flag) {
        if (move_capture(move)) return move_make(move, board, 0);
        return 0;
    } else {
        const piece::Piece &piece = move_piece(move);
        const Color color = board.get_side();
        const Square source = static_cast<Square>(move_source(move));
        const Square target = static_cast<Square>(move_target(move));
        const Square ntarget = static_cast<Square>(move_target(move) + (color == Color::WHITE ? -8 : +8));

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

        board.set_enpassant(move_double(move) ? ntarget : Square::no_sq);

        if (move_castle(move)) {
            static constexpr const piece::Piece &rook_white = piece::get(piece::Type::ROOK, Color::WHITE);
            static constexpr const piece::Piece &rook_black = piece::get(piece::Type::ROOK, Color::BLACK);
            if (target == Square::g1) _piece_move(board, rook_white, Square::h1, Square::f1);
            if (target == Square::c1) _piece_move(board, rook_white, Square::a1, Square::d1);
            if (target == Square::g8) _piece_move(board, rook_black, Square::h8, Square::f8);
            if (target == Square::c8) _piece_move(board, rook_black, Square::a8, Square::d8);
        }

        board.xor_hash(zobrist_key_castle(board.get_castle()));
        board.and_castle(castling_rights[move_source(move)]);
        board.and_castle(castling_rights[move_target(move)]);
        board.xor_hash(zobrist_key_castle(board.get_castle()));

        if (!board.is_check()) {
            board.switch_side();
            return 1;
        }
        return 0;
    }
}
