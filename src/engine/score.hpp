#ifndef STELLAR_SCORE_H
#define STELLAR_SCORE_H

#define MAX_PLY 64

#define SCORE_INFINITY 32000
#define MATE_VALUE 31000
#define MATE_SCORE 30000

namespace score {

inline constexpr const uint16_t value[6] = {100, 300, 350, 500, 1000, 10000};
inline constexpr const uint16_t capture[6][6] = {
    // clang-format off
    {105, 205, 305, 405, 505, 605},
    {104, 204, 304, 404, 504, 604},
    {103, 203, 303, 403, 503, 603},
    {102, 202, 302, 402, 502, 602},
    {101, 201, 301, 401, 501, 601},
    {100, 200, 300, 400, 500, 600},
    // clang-format on
};

inline constexpr const int8_t position[6][64] = {
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

inline constexpr uint16_t get(const piece::Type piece) { return value[to_underlying(piece)]; }
inline constexpr uint16_t get(const piece::Type piece, const piece::Type captured) {
    return capture[to_underlying(piece)][to_underlying(captured)];
}

inline constexpr int8_t get(const piece::Type type, const color::Color color, const square::Square square) {
    uint8_t square_i = to_underlying(color == color::WHITE ? square : square::mirror(square));
    return position[to_underlying(type)][square_i];
}

inline constexpr const uint8_t pawn_double = 10;
inline constexpr const uint8_t pawn_isolated = 10;
inline constexpr const std::array<std::array<int16_t, 8>, 2> pawn_passed = {
    {{0, 10, 30, 50, 75, 100, 150, 200}, {200, 150, 100, 75, 50, 30, 10, 0}}};

inline constexpr const uint8_t score_open_semi = 10;
inline constexpr const uint8_t score_open = 15;

} // namespace score

#endif
