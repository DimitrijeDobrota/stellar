#include "move.hpp"
#include "utils.hpp"

#include <algorithm>
#include <iomanip>

void Move::piece_remove(Board &board, piece::Type type, Color color, Square square) const {
    board.pop_piece(type, color, square);
    board.xor_hash(Zobrist::key_piece(type, color, square));
}

void Move::piece_set(Board &board, piece::Type type, Color color, Square square) const {
    board.set_piece(type, color, square);
    board.xor_hash(Zobrist::key_piece(type, color, square));
}

void Move::piece_move(Board &board, piece::Type type, Color color, Square source, Square target) const {
    piece_remove(board, type, color, source);
    piece_set(board, type, color, target);
}

using piece::Type::PAWN;
using piece::Type::ROOK;

bool Move::make(Board &board) const {
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

    const Color color = board.get_side();
    const Color colorOther = color == Color::BLACK ? Color::WHITE : Color::BLACK;
    const Square source = this->source();
    const Square target = this->target();

    const Square ntarget =
        static_cast<Square>(to_underlying(this->target()) + (color == Color::WHITE ? -8 : +8));

    const piece::Type piece = board.get_square_piece_type(source);

    if (!is_capture()) {
        if (is_promote()) {
            piece_remove(board, piece, color, source);
            piece_set(board, promoted(), color, target);
        } else {
            piece_move(board, piece, color, source, target);
        }
    } else {
        const piece::Type captured = board.get_square_piece_type(target);
        if (is_enpassant()) {
            piece_move(board, piece, color, source, target);
            piece_remove(board, PAWN, colorOther, ntarget);
        } else if (is_promote()) {
            piece_remove(board, piece, color, source);
            piece_remove(board, captured, colorOther, target);
            piece_set(board, promoted(), color, target);
        } else {
            piece_remove(board, captured, colorOther, target);
            piece_move(board, piece, color, source, target);
        }
    }

    board.set_enpassant(is_double() ? ntarget : Square::no_sq);

    if (is_castle()) {
        if (color == Color::WHITE) {
            if (is_castle_king()) piece_move(board, ROOK, Color::WHITE, Square::h1, Square::f1);
            if (is_castle_queen()) piece_move(board, ROOK, Color::WHITE, Square::a1, Square::d1);
        } else {
            if (is_castle_king()) piece_move(board, ROOK, Color::BLACK, Square::h8, Square::f8);
            if (is_castle_queen()) piece_move(board, ROOK, Color::BLACK, Square::a8, Square::d8);
        }
    }

    board.and_castle(castling_rights[to_underlying(this->source())] &
                     castling_rights[to_underlying(this->target())]);

    if (!board.is_check()) {
        board.switch_side();
        return 1;
    }
    return 0;
}

std::ostream &operator<<(std::ostream &os, Move move) {
    os << square_to_coordinates(move.source()) << " ";
    os << square_to_coordinates(move.target()) << " ";
    os << (move.is_promote() ? piece::get_code(move.promoted()) : '.') << " ";
    os << move.is_double() << " ";
    os << move.is_enpassant() << " ";
    os << move.is_castle();

    return os;
}
