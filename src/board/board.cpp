#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "board.hpp"
#include "piece.hpp"
#include "utils_cpp.hpp"
#include "zobrist.hpp"

/* Getters */

Color Board::get_side(void) const { return side; }
U64 Board::get_hash(void) const { return hash; }
uint8_t Board::get_castle(void) const { return castle; }
Square Board::get_enpassant(void) const { return enpassant; }

U64 Board::get_bitboard_color(Color side) const {
    return colors[to_underlying(side)];
}

U64 Board::get_bitboard_occupancy(void) const {
    return colors[to_underlying(Color::WHITE)] |
           colors[to_underlying(Color::BLACK)];
}

U64 Board::get_bitboard_piece(piece::Type piece) const {
    return pieces[to_underlying(piece)];
}

U64 Board::get_bitboard_piece(const piece::Piece &piece) const {
    return get_bitboard_piece(piece.type, piece.color);
}

U64 Board::get_bitboard_piece(piece::Type piece, Color color) const {
    return pieces[to_underlying(piece)] & colors[to_underlying(color)];
}

U64 Board::get_bitboard_piece_attacks(piece::Type piece, Color color,
                                      Square square) const {
    return get_bitboard_piece_attacks(piece::get(piece, color), square);
}

U64 Board::get_bitboard_piece_attacks(const piece::Piece &piece,
                                      Square square) const {
    return piece.attacks(square, get_bitboard_occupancy());
}

Color Board::get_square_piece_color(Square square) const {
    if (bit_get(colors[to_underlying(Color::WHITE)], to_underlying(square)))
        return Color::WHITE;
    if (bit_get(colors[to_underlying(Color::BLACK)], to_underlying(square)))
        return Color::BLACK;
    throw std::exception();
}

piece::Type Board::get_square_piece_type(Square square) const {
    for (piece::Type type : piece::TypeIter()) {
        if (bit_get(pieces[to_underlying(type)], to_underlying(square)))
            return type;
    }
    throw std::exception();
}

const piece::Piece &Board::get_square_piece(Square square) const {
    return piece::get(get_square_piece_type(square),
                      get_square_piece_color(square));
}

/* Setters */

void Board::and_castle(Castle right) { castle &= to_underlying(right); }

void Board::switch_side(void) {
    side = (side == Color::BLACK) ? Color::WHITE : Color::BLACK;
    hash ^= zobrist_key_side();
}

void Board::set_enpassant(Square target) {
    if (enpassant != Square::no_sq) hash ^= zobrist_key_enpassant(enpassant);

    if (target != Square::no_sq) hash ^= zobrist_key_enpassant(target);
    enpassant = target;
}

void Board::pop_bitboard_color(Color color, Square square) {
    bit_pop(colors[to_underlying(color)], to_underlying(square));
}

void Board::set_bitboard_color(Color color, Square square) {
    bit_set(colors[to_underlying(color)], to_underlying(square));
}

void Board::pop_bitboard_piece(const piece::Piece &piece, Square square) {
    bit_pop(pieces[to_underlying(piece.type)], to_underlying(square));
}

void Board::set_bitboard_piece(const piece::Piece &piece, Square square) {
    bit_set(pieces[to_underlying(piece.type)], to_underlying(square));
}

void Board::pop_piece(const piece::Piece &piece, Square square) {
    pop_bitboard_color(piece.color, square);
    pop_bitboard_piece(piece, square);
}

void Board::set_piece(const piece::Piece &piece, Square square) {
    set_bitboard_color(piece.color, square);
    set_bitboard_piece(piece, square);
}

/* Queries */

bool Board::is_square_occupied(Square square) const {
    return bit_get(get_bitboard_occupancy(), to_underlying(square));
}

bool Board::is_square_attacked(Square square, Color side) const {
    // side switch because of pawns
    Color side_other = (side == Color::BLACK) ? Color::WHITE : Color::BLACK;

    for (piece::Type type : piece::TypeIter()) {
        const piece::Piece &piece = piece::get(type, side_other);
        if (get_bitboard_piece_attacks(piece, square) &
            get_bitboard_piece(piece))
            return 1;
    }

    return 0;
}

bool Board::is_check(void) const {
    U64 king =
        pieces[to_underlying(piece::Type::KING)] & colors[to_underlying(side)];
    Square square = static_cast<Square>(bit_lsb_index(king));
    return is_square_attacked(square, side);
}

Board::Board(const std::string &fen) {
    *this = {0};

    side = Color::WHITE;
    enpassant = Square::no_sq;
    castle = 0;

    int file = 0, rank = 7, i;
    for (i = 0; i < fen.size(); i++) {
        if (isalpha(fen[i])) {
            set_piece(piece::get_from_code(fen[i]),
                      static_cast<Square>(rank * 8 + file));
            file++;
        } else if (isdigit(fen[i])) {
            file += fen[i] - '0';
        } else if (fen[i] == '/') {
            file = 0;
            rank--;
        } else {
            throw std::exception();
        }
    }

    i++;
    if (fen[i] == 'w')
        side = Color::WHITE;
    else if (fen[i] == 'b')
        side = Color::BLACK;
    else
        throw std::exception();

    for (i += 2; fen[i] != ' '; i++) {
        if (fen[i] == 'K')
            castle |= to_underlying(Castle::WK);
        else if (fen[i] == 'Q')
            castle |= to_underlying(Castle::WQ);
        else if (fen[i] == 'k')
            castle |= to_underlying(Castle::BK);
        else if (fen[i] == 'q')
            castle |= to_underlying(Castle::BQ);
        else
            throw std::exception();
    }

    if (fen[++i] != '-') enpassant = square_from_coordinates(fen.data() + i);
    hash = zobrist_hash(*this);
}

std::ostream &operator<<(std::ostream &os, const Board &board) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            try {
                Square square = static_cast<Square>((7 - rank) * 8 + file);
                os << board.get_square_piece(square).code;
            } catch (std::exception e) {
                os << ". ";
            }
        }
        printf("\n");
    }
    os << "    A B C D E F G H\n";
    os << "     Side: ";
    os << ((board.side == Color::WHITE) ? "white" : "black") << "\n";
    os << "Enpassant: " << square_to_coordinates(board.enpassant) << "\n";
    os << "   Castle:";
    os << ((board.castle & to_underlying(Board::Castle::WK)) ? 'K' : '-');
    os << ((board.castle & to_underlying(Board::Castle::WQ)) ? 'Q' : '-');
    os << ((board.castle & to_underlying(Board::Castle::BK)) ? 'k' : '-');
    os << ((board.castle & to_underlying(Board::Castle::BQ)) ? 'q' : '-');
    os << "\n     Hash:" << board.hash << "\n\n";

    return os;
}
