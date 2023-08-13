#ifndef STELLAR_BOARD_H
#define STELLAR_BOARD_H

#include "piece.hpp"
#include "utils_cpp.hpp"

#include <iostream>
#include <string>

typedef struct Board Board;
class Board {
  public:
    enum class Castle : uint8_t {
        WK = 1,
        WQ = 2,
        BK = 4,
        BQ = 8
    };

    Board() {}
    Board(const std::string &fen);

    friend std::ostream &operator<<(std::ostream &os, const Board &board);

    /* Getters */

    U64 get_hash(void) const;
    Color get_side(void) const;
    uint8_t get_castle(void) const;
    Square get_enpassant(void) const;

    U64 get_bitboard_color(Color side) const;
    U64 get_bitboard_occupancy(void) const;

    U64 get_bitboard_piece(piece::Type piece) const;
    U64 get_bitboard_piece(piece::Type piece, Color color) const;
    U64 get_bitboard_piece(const piece::Piece &piece) const;

    U64 get_bitboard_piece_attacks(piece::Type piece, Color color, Square square) const;
    U64 get_bitboard_piece_attacks(const piece::Piece &piece, Square square) const;
    U64 get_bitboard_piece_moves(piece::Type piece, Color color, Square square) const;
    U64 get_bitboard_piece_moves(const piece::Piece &piece, Square square) const;

    Color get_square_piece_color(Square square) const;
    piece::Type get_square_piece_type(Square square) const;
    const piece::Piece *get_square_piece(Square square) const;

    /* Setters */

    void xor_hash(U64 op);
    void switch_side(void);
    void and_castle(uint8_t right);
    void set_enpassant(Square target);

    void pop_bitboard_color(Color color, Square square);
    void set_bitboard_color(Color color, Square square);

    void pop_bitboard_piece(const piece::Piece &piece, Square square);
    void set_bitboard_piece(const piece::Piece &piece, Square square);

    void pop_piece(const piece::Piece &piece, Square square);
    void set_piece(const piece::Piece &piece, Square square);

    /* Queries */

    bool is_square_attacked(Square Square, Color side) const;
    bool is_square_occupied(Square Square) const;
    bool is_piece_attack_square(const piece::Piece &piece, Square source, Square target) const;
    bool is_check(void) const;

  private:
    U64 colors[2] = {0};
    U64 pieces[6] = {0};
    U64 hash = 0;
    Color side = Color::WHITE;
    Square enpassant = Square::no_sq;
    uint8_t castle = 0;
};

#endif
