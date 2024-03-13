#include "move.hpp"
#include "utils.hpp"
#include "utils_ui.hpp"

#include <algorithm>
#include <iomanip>

void Move::piece_remove(Board &board, Type type, Color color, Square square) const {
    board.pop_piece(type, color, square);
    board.xor_hash(zobrist::key_piece(type, color, square));
}

void Move::piece_set(Board &board, Type type, Color color, Square square) const {
    board.set_piece(type, color, square);
    board.xor_hash(zobrist::key_piece(type, color, square));
}

void Move::piece_move(Board &board, Type type, Color color, Square source, Square target) const {
    piece_remove(board, type, color, source);
    piece_set(board, type, color, target);
}

using Type::PAWN;
using Type::ROOK;

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

    const Color color = board.get_side(), colorOther = other(color);
    const Square source = this->source(), target = this->target();

    const auto ntarget = static_cast<Square>(this->target() + (color == Color::WHITE ? -8 : +8));

    const Type piece = board.get_square_piece_type(source);

    if (!is_capture()) {
        if (is_promote()) {
            piece_remove(board, piece, color, source);
            piece_set(board, promoted(), color, target);
        } else {
            piece_move(board, piece, color, source, target);
        }
    } else {
        const Type captured = board.get_square_piece_type(target);
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

    board.and_castle(castling_rights[this->source()] & castling_rights[this->target()]);

    if (!board.is_check()) {
        board.switch_side();
        return true;
    }
    return false;
}

void Move::print() const {
    std::cout << to_coordinates(source()) << " ";
    std::cout << to_coordinates(target()) << " ";
    std::cout << (is_promote() ? piece::get_code(promoted()) : '.') << " ";
    std::cout << is_double() << " ";
    std::cout << is_enpassant() << " ";
    std::cout << is_castle();
}

Move::operator std::string() const {
    std::string res = to_coordinates(source()) + to_coordinates(target());
    if (is_promote()) res += piece::get_code(promoted());
    return res;
}

std::ostream &operator<<(std::ostream &os, Move move) { return os << (std::string)move; }
