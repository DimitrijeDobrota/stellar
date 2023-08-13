#include "move.hpp"
#include "utils_cpp.hpp"
#include "zobrist.hpp"

#include <algorithm>
#include <iomanip>

void Move::piece_remove(Board &board, const piece::Piece &piece, Square square) const {
    board.pop_piece(piece, square);
    board.xor_hash(Zobrist::key_piece(piece, square));
}

void Move::piece_set(Board &board, const piece::Piece &piece, Square square) const {
    board.set_piece(piece, square);
    board.xor_hash(Zobrist::key_piece(piece, square));
}

void Move::piece_move(Board &board, const piece::Piece &piece, Square source, Square target) const {
    piece_remove(board, piece, source);
    piece_set(board, piece, target);
}

bool Move::make(Board &board, bool attack_only) const {
    static constexpr const int castling_rights[64] = {
        // clang-format off
        13, 15, 15, 15, 12, 15, 15, 14,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        7,  15, 15, 15, 3,  15, 15, 11,
        // clang-format on
    };

    if (attack_only) {
        if (is_capture()) return make(board, false);
        return 0;
    } else {
        const piece::Piece &piece = this->piece();
        const Color color = board.get_side();
        const Square source = this->source();
        const Square target = this->target();

        const Square ntarget =
            static_cast<Square>(to_underlying(this->target()) + (color == Color::WHITE ? -8 : +8));

        if (!is_capture()) {
            if (is_promote()) {
                piece_remove(board, piece, source);
                piece_set(board, piece_promote(), target);
            } else {
                piece_move(board, piece, source, target);
            }
        } else {
            if (is_enpassant()) {
                piece_move(board, piece, source, target);
                piece_remove(board, piece_capture(), ntarget);
            } else if (is_promote()) {
                piece_remove(board, piece, source);
                piece_remove(board, piece_capture(), target);
                piece_set(board, piece_promote(), target);
            } else {
                piece_remove(board, piece_capture(), target);
                piece_move(board, piece, source, target);
            }
        }

        board.set_enpassant(is_double() ? ntarget : Square::no_sq);

        if (is_castle()) {
            static constexpr const piece::Piece &rook_white = piece::get(piece::Type::ROOK, Color::WHITE);
            static constexpr const piece::Piece &rook_black = piece::get(piece::Type::ROOK, Color::BLACK);
            if (target == Square::g1) piece_move(board, rook_white, Square::h1, Square::f1);
            if (target == Square::c1) piece_move(board, rook_white, Square::a1, Square::d1);
            if (target == Square::g8) piece_move(board, rook_black, Square::h8, Square::f8);
            if (target == Square::c8) piece_move(board, rook_black, Square::a8, Square::d8);
        }

        board.xor_hash(Zobrist::key_castle(board.get_castle()));
        board.and_castle(castling_rights[to_underlying(this->source())]);
        board.and_castle(castling_rights[to_underlying(this->target())]);
        board.xor_hash(Zobrist::key_castle(board.get_castle()));

        if (!board.is_check()) {
            board.switch_side();
            return 1;
        }
        return 0;
    }
}

std::ostream &operator<<(std::ostream &os, const Move &move) {
    os << square_to_coordinates(move.source()) << " ";
    os << square_to_coordinates(move.target()) << " ";
    os << move.piece().code << " ";
    os << (move.is_capture() ? move.piece_capture().code : '.') << " ";
    os << (move.is_promote() ? move.piece_promote().code : '.') << " ";
    os << move.is_double() << " ";
    os << move.is_enpassant() << " ";
    os << move.is_castle();

    return os;
}
