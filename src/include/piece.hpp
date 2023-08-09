#ifndef STELLAR_PIECE_H
#define STELLAR_PIECE_H

#include "attacks.hpp"
#include "utils_cpp.hpp"

#include <cctype>

namespace piece {

enum class Type {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    TypeSIZE
};
typedef Iterator<Type, Type::PAWN, Type::KING> TypeIter;

class Piece {
  public:
    const Type type;
    const Color color;
    const char code;
    const char *symbol;
    const attack_get_f attacks;
    const uint8_t index;

  protected:
    constexpr Piece(Type type, Color color, char code, const char *symbol,
                    attack_get_f attacks)
        : type(type), color(color), code(code), symbol(symbol),
          attacks(attacks), index(index_calc()) {}

    constexpr int index_calc() {
        return to_underlying(Type::TypeSIZE) * to_underlying(color) *
               to_underlying(type);
    }
};

class Pawn : public Piece {
  public:
    constexpr Pawn(Color color)
        : Piece(Type::PAWN, color, color == Color::WHITE ? 'P' : 'p',
                color == Color::WHITE ? "■ " : "■ ",
                color == Color::WHITE ? attacks_wpawn_get : attacks_bpawn_get) {
    }
};

class Knight : public Piece {
  public:
    constexpr Knight(Color color)
        : Piece(Type::KNIGHT, color, color == Color::WHITE ? 'N' : 'n',
                color == Color::WHITE ? "■ " : "■ ", attacks_knight_get) {}
};

class Bishop : public Piece {
  public:
    constexpr Bishop(Color color)
        : Piece(Type::BISHOP, color, color == Color::WHITE ? 'B' : 'b',
                color == Color::WHITE ? "■ " : "■ ", attacks_bishop_get) {}
};

class Rook : public Piece {
  public:
    constexpr Rook(Color color)
        : Piece(Type::ROOK, color, color == Color::WHITE ? 'R' : 'r',
                color == Color::WHITE ? "■ " : "■ ", attacks_rook_get) {}
};

class Queen : public Piece {
  public:
    constexpr Queen(Color color)
        : Piece(Type::QUEEN, color, color == Color::WHITE ? 'Q' : 'q',
                color == Color::WHITE ? "■ " : "■ ", attacks_queen_get) {}
};

class King : public Piece {
  public:
    constexpr King(Color color)
        : Piece(Type::KING, color, color == Color::WHITE ? 'K' : 'k',
                color == Color::WHITE ? "■ " : "■ ", attacks_king_get) {}
};

const constexpr Piece table[2][6] = {
    {Pawn(Color::WHITE), Knight(Color::WHITE), Bishop(Color::WHITE),
     Rook(Color::WHITE), Queen(Color::WHITE), King(Color::WHITE)},
    {Pawn(Color::BLACK), Knight(Color::BLACK), Bishop(Color::BLACK),
     Rook(Color::BLACK), Queen(Color::BLACK), King(Color::BLACK)},
};

constexpr const Piece &get(Type type, Color color) {
    return table[static_cast<int>(color)][static_cast<int>(type)];
}

constexpr const Piece &get_from_code(char code) {
    Color color = isupper(code) ? Color::WHITE : Color::BLACK;

    for (Type type : TypeIter()) {
        const Piece &piece = get(type, color);
        if (piece.code == code) return piece;
    }

    throw std::exception();
}

constexpr const Piece &get_from_index(uint8_t index) {
    return table[index / 6][index % 6];
}

} // namespace piece

#endif
