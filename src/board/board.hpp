#ifndef STELLAR_BOARD_H
#define STELLAR_BOARD_H

#include "piece.hpp"
#include "utils.hpp"
#include "zobrist.hpp"

#include <iostream>
#include <string>

class Board {
  public:
    enum class Castle : uint8_t {
        WK = 1,
        WQ = 2,
        BK = 4,
        BQ = 8
    };

    Board() = default;
    Board(const std::string &fen);

    friend std::ostream &operator<<(std::ostream &os, const Board &board);

    /* Getters */

    inline constexpr U64 get_hash(void) const;
    inline constexpr Color get_side(void) const;
    inline constexpr uint8_t get_castle(void) const;
    inline constexpr Square get_enpassant(void) const;

    inline constexpr U64 get_bitboard_color(Color side) const;
    inline constexpr U64 get_bitboard_occupancy(void) const;

    inline constexpr U64 get_bitboard_piece(piece::Type piece) const;
    inline constexpr U64 get_bitboard_piece(piece::Type piece, Color color) const;

    inline constexpr U64 get_bitboard_piece_attacks(piece::Type piece, Color color, Square from) const;
    inline constexpr U64 get_bitboard_piece_moves(piece::Type piece, Color color, Square from) const;

    inline constexpr Color get_square_piece_color(Square square) const;
    inline constexpr piece::Type get_square_piece_type(Square square) const;
    inline constexpr const piece::Piece *get_square_piece(Square square) const;

    /* Setters */

    inline constexpr void xor_hash(U64 op);
    inline constexpr void switch_side(void);
    inline constexpr void and_castle(uint8_t right);
    inline constexpr void set_enpassant(Square target);

    inline constexpr void pop_bitboard_color(Color color, Square square);
    inline constexpr void set_bitboard_color(Color color, Square square);

    inline constexpr void pop_bitboard_piece(piece::Type type, Square square);
    inline constexpr void set_bitboard_piece(piece::Type type, Square square);

    inline constexpr void pop_piece(piece::Type type, Color side, Square square);
    inline constexpr void set_piece(piece::Type type, Color side, Square square);

    /* Queries */

    inline constexpr bool is_square_attacked(Square Square, Color side) const;
    inline constexpr bool is_square_occupied(Square Square) const;
    inline constexpr bool is_piece_attack_square(piece::Type type, Color color, Square source,
                                                 Square target) const;
    inline constexpr bool is_check(void) const;

  private:
    U64 colors[2] = {0};
    U64 pieces[6] = {0};
    U64 hash = 0;
    Color side = Color::WHITE;
    Square enpassant = Square::no_sq;
    uint8_t castle = 0;
};

constexpr Color Board::get_side(void) const { return side; }
constexpr U64 Board::get_hash(void) const { return hash; }
constexpr uint8_t Board::get_castle(void) const { return castle; }
constexpr Square Board::get_enpassant(void) const { return enpassant; }

constexpr U64 Board::get_bitboard_color(Color side) const { return colors[to_underlying(side)]; }

constexpr U64 Board::get_bitboard_occupancy(void) const {
    return colors[to_underlying(Color::WHITE)] | colors[to_underlying(Color::BLACK)];
}

constexpr U64 Board::get_bitboard_piece(piece::Type piece) const { return pieces[to_underlying(piece)]; }

constexpr U64 Board::get_bitboard_piece(piece::Type piece, Color color) const {
    return pieces[to_underlying(piece)] & colors[to_underlying(color)];
}

constexpr U64 Board::get_bitboard_piece_attacks(piece::Type type, Color color, Square from) const {
    return piece::get_attack(type, color, from, get_bitboard_occupancy());
}

constexpr U64 Board::get_bitboard_piece_moves(piece::Type type, Color color, Square square) const {
    return get_bitboard_piece_attacks(type, color, square) & ~get_bitboard_color(color);
}

constexpr Color Board::get_square_piece_color(Square square) const {
    if (bit_get(colors[to_underlying(Color::WHITE)], to_underlying(square))) return Color::WHITE;
    if (bit_get(colors[to_underlying(Color::BLACK)], to_underlying(square))) return Color::BLACK;
    throw std::exception();
}

constexpr piece::Type Board::get_square_piece_type(Square square) const {
    for (piece::Type type : piece::TypeIter()) {
        if (bit_get(pieces[to_underlying(type)], to_underlying(square))) return type;
    }
    return piece::Type::NONE;
}

constexpr const piece::Piece *Board::get_square_piece(Square square) const {
    try {
        return &piece::get(get_square_piece_type(square), get_square_piece_color(square));
    } catch (std::exception &e) {
        return nullptr;
    }
}

/* Setters */

constexpr void Board::xor_hash(U64 op) { hash ^= op; }
constexpr void Board::and_castle(uint8_t right) {
    hash ^= Zobrist::key_castle(castle);
    castle &= right;
    hash ^= Zobrist::key_castle(castle);
}

constexpr void Board::switch_side(void) {
    side = (side == Color::BLACK) ? Color::WHITE : Color::BLACK;
    hash ^= Zobrist::key_side();
}

constexpr void Board::set_enpassant(Square target) {
    if (enpassant != Square::no_sq) hash ^= Zobrist::key_enpassant(enpassant);
    if (target != Square::no_sq) hash ^= Zobrist::key_enpassant(target);
    enpassant = target;
}

constexpr void Board::pop_bitboard_color(Color color, Square square) {
    bit_pop(colors[to_underlying(color)], to_underlying(square));
}

constexpr void Board::set_bitboard_color(Color color, Square square) {
    bit_set(colors[to_underlying(color)], to_underlying(square));
}

constexpr void Board::pop_bitboard_piece(piece::Type type, Square square) {
    bit_pop(pieces[to_underlying(type)], to_underlying(square));
}

constexpr void Board::set_bitboard_piece(piece::Type type, Square square) {
    bit_set(pieces[to_underlying(type)], to_underlying(square));
}

constexpr void Board::pop_piece(piece::Type type, Color side, Square square) {
    pop_bitboard_color(side, square);
    pop_bitboard_piece(type, square);
}

constexpr void Board::set_piece(piece::Type type, Color side, Square square) {
    set_bitboard_color(side, square);
    set_bitboard_piece(type, square);
}

/* Queries */

constexpr bool Board::is_square_occupied(Square square) const {
    return bit_get(get_bitboard_occupancy(), to_underlying(square));
}

constexpr bool Board::is_square_attacked(Square square, Color side) const {
    Color side_other = (side == Color::BLACK) ? Color::WHITE : Color::BLACK;

    for (piece::Type type : piece::TypeIter()) {
        if (get_bitboard_piece_attacks(type, side_other, square) & get_bitboard_piece(type, side)) {
            return 1;
        }
    }

    return 0;
}

constexpr bool Board::is_piece_attack_square(piece::Type type, Color color, Square source,
                                             Square target) const {
    return get_bitboard_piece_attacks(type, color, source) & (C64(1) << to_underlying(target));
}

constexpr bool Board::is_check(void) const {
    U64 king = pieces[to_underlying(piece::Type::KING)] & colors[to_underlying(side)];
    Color side_other = (side == Color::BLACK) ? Color::WHITE : Color::BLACK;
    Square square = static_cast<Square>(bit_lsb_index(king));
    return is_square_attacked(square, side_other);
}

U64 Zobrist::hash(const Board &board) {
    U64 key_final = C64(0);
    uint8_t square;

    for (piece::Type type : piece::TypeIter()) {
        int piece_white_index = piece::get_index(type, Color::WHITE);
        U64 bitboard_white = board.get_bitboard_piece(type, Color::WHITE);
        bitboard_for_each_bit(square, bitboard_white) { key_final ^= keys_piece[piece_white_index][square]; }

        int piece_black_index = piece::get_index(type, Color::BLACK);
        U64 bitboard_black = board.get_bitboard_piece(type, Color::BLACK);
        bitboard_for_each_bit(square, bitboard_black) { key_final ^= keys_piece[piece_black_index][square]; }
    }

    key_final ^= keys_castle[board.get_castle()];

    if (board.get_side() == Color::BLACK) key_final ^= keys_side;
    if (board.get_enpassant() != Square::no_sq)
        key_final ^= keys_enpassant[to_underlying(board.get_enpassant())];

    return key_final;
}

#endif
