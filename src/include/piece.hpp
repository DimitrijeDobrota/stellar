#ifndef STELLAR_PIECE_H
#define STELLAR_PIECE_H

#include "attack.hpp"
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
    constexpr U64 operator()(Square square, U64 occupancy) const { return attack(square, occupancy); }

    const Type type;
    const Color color;
    const char code;
    const char *symbol;
    const uint8_t index;

  protected:
    constexpr Piece(Type type, Color color, char code, const char *symbol, const attack::Attack &attack)
        : type(type), color(color), code(code), symbol(symbol), index(index_calc(color, type)),
          attack(attack) {}

    constexpr uint8_t index_calc(Color color, Type type) {
        return to_underlying(color) * 6 + to_underlying(type);
    }

  private:
    const attack::Attack &attack;
};

class Pawn : public Piece {
  public:
    constexpr Pawn(Color color)
        : Piece(Type::PAWN, color, color == Color::WHITE ? 'P' : 'p', color == Color::WHITE ? "■ " : "■ ",
                color == Color::WHITE ? *(attack::Attack *)&attack::pawnW
                                      : *(attack::Attack *)&attack::pawnB) {}
};

class Knight : public Piece {
  public:
    constexpr Knight(Color color)
        : Piece(Type::KNIGHT, color, color == Color::WHITE ? 'N' : 'n', color == Color::WHITE ? "■ " : "■ ",
                attack::knight) {}
};

class Bishop : public Piece {
  public:
    constexpr Bishop(Color color)
        : Piece(Type::BISHOP, color, color == Color::WHITE ? 'B' : 'b', color == Color::WHITE ? "■ " : "■ ",
                attack::bishop) {}
};

class Rook : public Piece {
  public:
    constexpr Rook(Color color)
        : Piece(Type::ROOK, color, color == Color::WHITE ? 'R' : 'r', color == Color::WHITE ? "■ " : "■ ",
                attack::rook) {}
};

class Queen : public Piece {
  public:
    constexpr Queen(Color color)
        : Piece(Type::QUEEN, color, color == Color::WHITE ? 'Q' : 'q', color == Color::WHITE ? "■ " : "■ ",
                attack::queen) {}
};

class King : public Piece {
  public:
    constexpr King(Color color)
        : Piece(Type::KING, color, color == Color::WHITE ? 'K' : 'k', color == Color::WHITE ? "■ " : "■ ",
                attack::king) {}
};

const constexpr Piece table[2][6] = {
    {Pawn(Color::WHITE), Knight(Color::WHITE), Bishop(Color::WHITE), Rook(Color::WHITE), Queen(Color::WHITE),
     King(Color::WHITE)},
    {Pawn(Color::BLACK), Knight(Color::BLACK), Bishop(Color::BLACK), Rook(Color::BLACK), Queen(Color::BLACK),
     King(Color::BLACK)},
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

constexpr const Piece &get_from_index(uint8_t index) { return table[index / 6][index % 6]; }

} // namespace piece

#endif
