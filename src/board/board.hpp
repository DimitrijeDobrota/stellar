#ifndef STELLAR_BOARD_H
#define STELLAR_BOARD_H

#include "attack.hpp"
#include "bit.hpp"
#include "piece.hpp"
#include "utils.hpp"
#include "zobrist.hpp"

#include <iostream>
#include <string>

#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "

class Board {
  public:
    enum Castle : uint8_t {
        WK = 1,
        WQ = 2,
        BK = 4,
        BQ = 8
    };

    Board() = default;
    Board(const std::string &fen);

    friend std::ostream &operator<<(std::ostream &os, const Board &board);

    /* Getters */

    [[nodiscard]] inline constexpr U64 get_hash() const;
    [[nodiscard]] inline constexpr Color get_side() const;
    [[nodiscard]] inline constexpr uint8_t get_castle() const;
    [[nodiscard]] inline constexpr Square get_enpassant() const;

    [[nodiscard]] inline constexpr U64 get_bitboard_color(Color side) const;
    [[nodiscard]] inline constexpr U64 get_bitboard_occupancy() const;

    [[nodiscard]] inline constexpr U64 get_bitboard_piece(Type piece) const;
    [[nodiscard]] inline constexpr U64 get_bitboard_piece(Type piece, Color color) const;

    [[nodiscard]] inline constexpr U64 get_bitboard_piece_attacks(Type piece, Color color, Square from) const;
    [[nodiscard]] inline constexpr U64 get_bitboard_piece_moves(Type piece, Color color, Square from) const;
    [[nodiscard]] inline constexpr U64 get_bitboard_square_land(Square land, Type piece, Color side) const;

    [[nodiscard]] inline constexpr Color get_square_piece_color(Square square) const;
    [[nodiscard]] inline constexpr Type get_square_piece_type(Square square) const;
    [[nodiscard]] inline constexpr const piece::Piece *get_square_piece(Square square) const;

    /* Setters */

    inline constexpr void xor_hash(U64 op);
    inline constexpr void switch_side();
    inline constexpr void and_castle(uint8_t right);
    inline constexpr void set_enpassant(Square target);

    inline constexpr void pop_bitboard_color(Color color, Square square);
    inline constexpr void set_bitboard_color(Color color, Square square);

    inline constexpr void pop_bitboard_piece(Type type, Square square);
    inline constexpr void set_bitboard_piece(Type type, Square square);

    inline constexpr void pop_piece(Type type, Color side, Square square);
    inline constexpr void set_piece(Type type, Color side, Square square);

    /* Queries */

    [[nodiscard]] inline constexpr bool is_square_attacked(Square square, Color side) const;
    [[nodiscard]] inline constexpr bool is_square_occupied(Square square) const;
    [[nodiscard]] inline constexpr bool is_piece_attack_square(Type type, Color color, Square source,
                                                               Square target) const;
    [[nodiscard]] inline constexpr bool is_check() const;

  private:
    U64 colors[2] = {0};
    U64 pieces[6] = {0};
    U64 hash = 0;
    Color side = WHITE;
    Square enpassant = Square::no_sq;
    uint8_t castle = 0;
};

constexpr Color Board::get_side() const { return side; }
constexpr U64 Board::get_hash() const { return hash; }
constexpr uint8_t Board::get_castle() const { return castle; }
constexpr Square Board::get_enpassant() const { return enpassant; }

constexpr U64 Board::get_bitboard_color(Color side) const { return colors[side]; }
constexpr U64 Board::get_bitboard_occupancy() const { return colors[WHITE] | colors[BLACK]; }
constexpr U64 Board::get_bitboard_piece(Type piece) const { return pieces[piece]; }

constexpr U64 Board::get_bitboard_piece(Type piece, Color color) const {
    return pieces[piece] & colors[color];
}

constexpr U64 Board::get_bitboard_piece_attacks(Type type, Color color, Square from) const {
    if (type == PAWN) return attack::attack_pawn(color, from);
    return attack::attack(type, from, get_bitboard_occupancy());
}

constexpr U64 Board::get_bitboard_piece_moves(Type type, Color color, Square square) const {
    return get_bitboard_piece_attacks(type, color, square) & ~get_bitboard_color(color);
}

constexpr U64 Board::get_bitboard_square_land(Square land, Type piece, Color side) const {

    return get_bitboard_piece_attacks(piece, other(side), land) & get_bitboard_piece(piece, side);
}

constexpr Color Board::get_square_piece_color(Square square) const {
    if (bit::get(colors[WHITE], square)) return WHITE;
    if (bit::get(colors[BLACK], square)) return BLACK;
    throw std::exception();
}

constexpr Type Board::get_square_piece_type(Square square) const {
    for (Type type = PAWN; type <= KING; ++type) {
        if (bit::get(pieces[type], square)) return type;
    }
    return Type::NONE;
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
    hash ^= zobrist::key_castle(castle);
    castle &= right;
    hash ^= zobrist::key_castle(castle);
}

constexpr void Board::switch_side() {
    side = other(side);
    hash ^= zobrist::key_side();
}

constexpr void Board::set_enpassant(Square target) {
    if (enpassant != Square::no_sq) hash ^= zobrist::key_enpassant(enpassant);
    if (target != Square::no_sq) hash ^= zobrist::key_enpassant(target);
    enpassant = target;
}

constexpr void Board::pop_bitboard_color(Color color, Square square) { bit::pop(colors[color], square); }
constexpr void Board::set_bitboard_color(Color color, Square square) { bit::set(colors[color], square); }
constexpr void Board::pop_bitboard_piece(Type type, Square square) { bit::pop(pieces[type], square); }
constexpr void Board::set_bitboard_piece(Type type, Square square) { bit::set(pieces[type], square); }

constexpr void Board::pop_piece(Type type, Color side, Square square) {
    pop_bitboard_color(side, square);
    pop_bitboard_piece(type, square);
}

constexpr void Board::set_piece(Type type, Color side, Square square) {
    set_bitboard_color(side, square);
    set_bitboard_piece(type, square);
}

/* Queries */

constexpr bool Board::is_square_occupied(Square square) const {
    return bit::get(get_bitboard_occupancy(), square);
}

constexpr bool Board::is_square_attacked(Square square, Color side) const {
    const Color side_other = other(side);

    for (Type type = PAWN; type <= KING; ++type) {
        if (get_bitboard_piece_attacks(type, side_other, square) & get_bitboard_piece(type, side)) {
            return true;
        }
    }

    return false;
}

constexpr bool Board::is_piece_attack_square(Type type, Color color, Square source, Square target) const {
    return get_bitboard_piece_attacks(type, color, source) & (C64(1) << target);
}

constexpr bool Board::is_check() const {
    U64 king = pieces[Type::KING] & colors[side];
    Color side_other = (side == BLACK) ? WHITE : BLACK;
    auto square = static_cast<Square>(bit::lsb_index(king));
    return is_square_attacked(square, side_other);
}

U64 zobrist::hash(const Board &board) {
    U64 key_final = C64(0);
    uint8_t square = 0;

    for (Type type = PAWN; type <= KING; ++type) {
        U64 bitboard_white = board.get_bitboard_piece(type, WHITE);
        bitboard_for_each_bit(square, bitboard_white) { key_final ^= keys_piece[WHITE][type][square]; }

        U64 bitboard_black = board.get_bitboard_piece(type, BLACK);
        bitboard_for_each_bit(square, bitboard_black) { key_final ^= keys_piece[BLACK][type][square]; }
    }

    key_final ^= keys_castle[board.get_castle()];

    if (board.get_side() == BLACK) key_final ^= keys_side;
    if (board.get_enpassant() != Square::no_sq) key_final ^= keys_enpassant[board.get_enpassant()];

    return key_final;
}

#endif
