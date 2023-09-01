#ifndef STELLAR_PIECE_H
#define STELLAR_PIECE_H

#include "attack.hpp"
#include "utils.hpp"

#include <cctype>

namespace piece {

enum class Type {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE = 7,
};
typedef Iterator<Type, Type::PAWN, Type::KING> TypeIter;

class Piece {
  public:
    constexpr U64 operator()(Square from, U64 occupancy) const { return attack(from, occupancy); }

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

constexpr const U64 get_attack(Type type, Color color, Square from, U64 occupancy) {
    return get(type, color)(from, occupancy);
}

constexpr const char get_code(Type type, Color color = Color::WHITE) { return get(type, color).code; }
constexpr const U64 get_index(Type type, Color color) { return get(type, color).index; }
constexpr const Piece &get_from_code(char code) {
    Color color = isupper(code) ? Color::WHITE : Color::BLACK;

    for (Type type : TypeIter()) {
        const Piece &piece = get(type, color);
        if (piece.code == code) return piece;
    }

    throw std::exception();
}

constexpr const Piece &get_from_index(uint8_t index) { return table[index / 6][index % 6]; }

inline constexpr const Square mirror[65] = {
    // clang-format off
        Square::a8, Square::b8, Square::c8, Square::d8, Square::e8, Square::f8, Square::g8, Square::h8,
        Square::a7, Square::b7, Square::c7, Square::d7, Square::e7, Square::f7, Square::g7, Square::h7,
        Square::a6, Square::b6, Square::c6, Square::d6, Square::e6, Square::f6, Square::g6, Square::h6,
        Square::a5, Square::b5, Square::c5, Square::d5, Square::e5, Square::f5, Square::g5, Square::h5,
        Square::a4, Square::b4, Square::c4, Square::d4, Square::e4, Square::f4, Square::g4, Square::h4,
        Square::a3, Square::b3, Square::c3, Square::d3, Square::e3, Square::f3, Square::g3, Square::h3,
        Square::a2, Square::b2, Square::c2, Square::d2, Square::e2, Square::f2, Square::g2, Square::h2,
        Square::a1, Square::b1, Square::c1, Square::d1, Square::e1, Square::f1, Square::g1, Square::h1, Square::no_sq,
    // clang-format on
};

constexpr inline const uint16_t value[6] = {100, 300, 350, 500, 1000, 10000};
constexpr inline const uint16_t capture[6][6] = {
    // clang-format off
    {105, 205, 305, 405, 505, 605},
    {104, 204, 304, 404, 504, 604},
    {103, 203, 303, 403, 503, 603},
    {102, 202, 302, 402, 502, 602},
    {101, 201, 301, 401, 501, 601},
    {100, 200, 300, 400, 500, 600},
    // clang-format on
};
constexpr inline const int8_t position[6][64] = {
    // clang-format off
    {
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0, -10, -10,   0,   0,   0,
         0,   0,   0,   5,   5,   0,   0,   0,
         5,   5,  10,  20,  20,   5,   5,   5,
        10,  10,  10,  20,  20,  10,  10,  10,
        20,  20,  20,  30,  30,  30,  20,  20,
        30,  30,  30,  40,  40,  30,  30,  30,
        90,  90,  90,  90,  90,  90,  90,  90
    }, {
        -5,  -10 , 0,   0,   0,   0, -10,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   5,  20,  10,  10,  20,   5,  -5,
        -5,  10,  20,  30,  30,  20,  10,  -5,
        -5,  10,  20,  30,  30,  20,  10,  -5,
        -5,   5,  20,  20,  20,  20,   5,  -5,
        -5,   0,   0,  10,  10,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5
    }, {
         0,   0, -10,   0,   0, -10,   0,   0,
         0,  30,   0,   0,   0,   0,  30,   0,
         0,  10,   0,   0,   0,   0,  10,   0,
         0,   0,  10,  20,  20,  10,   0,   0,
         0,   0,  10,  20,  20,  10,   0,   0,
         0,   0,   0,  10,  10,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0
    }, {
         0,   0,   0,  20,  20,   0,   0,   0,
         0,   0,  10,  20,  20,  10,   0,   0,
         0,   0,  10,  20,  20,  10,   0,   0,
         0,   0,  10,  20,  20,  10,   0,   0,
         0,   0,  10,  20,  20,  10,   0,   0,
         0,   0,  10,  20,  20,  10,   0,   0,
        50,  50,  50,  50,  50,  50,  50,  50,
        50,  50,  50,  50,  50,  50,  50,  50
    }, {
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0
    }, {
         0,   0,   5,   0, -15,   0,  10,   0,
         0,   5,   5,  -5,  -5,   0,   5,   0,
         0,   0,   5,  10,  10,   5,   0,   0,
         0,   5,  10,  20,  20,  10,   5,   0,
         0,   5,  10,  20,  20,  10,   5,   0,
         0,   5,   5,  10,  10,   5,   5,   0,
         0,   0,   5,   5,   5,   5,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0
    },
    // clang-format on
};

constexpr uint16_t score(Type piece) { return value[to_underlying(piece)]; }
constexpr uint16_t score(Type piece, Type captured) {
    return capture[to_underlying(piece)][to_underlying(captured)];
}

constexpr int8_t score(Type type, Color color, uint8_t square_i) {
    if (color == Color::BLACK) square_i = to_underlying(mirror[square_i]);
    return position[to_underlying(type)][square_i];
}

} // namespace piece

#endif
