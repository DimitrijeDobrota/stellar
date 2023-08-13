#include "score.hpp"
#include "move.hpp"
#include "stats.hpp"
#include "utils_cpp.hpp"

// clang-format off
struct Score_T {
  int value;
  int position[64];
  int capture[6];
};

static const struct Score_T Scores[] = {
{
.value = 100,
.position = {
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0, -10, -10,   0,   0,   0,
             0,   0,   0,   5,   5,   0,   0,   0,
             5,   5,  10,  20,  20,   5,   5,   5,
            10,  10,  10,  20,  20,  10,  10,  10,
            20,  20,  20,  30,  30,  30,  20,  20,
            30,  30,  30,  40,  40,  30,  30,  30,
            90,  90,  90,  90,  90,  90,  90,  90, },
.capture = { 105, 205, 305, 405, 505, 605} },
{
.value = 300,
.position = {
            -5,  -10 , 0,   0,   0,   0, -10,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   5,  20,  10,  10,  20,   5,  -5,
            -5,  10,  20,  30,  30,  20,  10,  -5,
            -5,  10,  20,  30,  30,  20,  10,  -5,
            -5,   5,  20,  20,  20,  20,   5,  -5,
            -5,   0,   0,  10,  10,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5, },
.capture = { 104, 204, 304, 404, 504, 604} },
{
.value = 350,
.position = {
             0,   0, -10,   0,   0, -10,   0,   0,
             0,  30,   0,   0,   0,   0,  30,   0,
             0,  10,   0,   0,   0,   0,  10,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,   0,  10,  10,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0, },
.capture = { 103, 203, 303, 403, 503, 603} },
{
.value = 500,
.position = {
             0,   0,   0,  20,  20,   0,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
             0,   0,  10,  20,  20,  10,   0,   0,
            50,  50,  50,  50,  50,  50,  50,  50,
            50,  50,  50,  50,  50,  50,  50,  50, },
.capture = { 102, 202, 302, 402, 502, 602} },
{
.value = 1000,
.position = {
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0, },
.capture = { 101, 201, 301, 401, 501, 601} },
{
.value = 10000,
.position = {
             0,   0,   5,   0, -15,   0,  10,   0,
             0,   5,   5,  -5,  -5,   0,   5,   0,
             0,   0,   5,  10,  10,   5,   0,   0,
             0,   5,  10,  20,  20,  10,   5,   0,
             0,   5,  10,  20,  20,  10,   5,   0,
             0,   5,   5,  10,  10,   5,   5,   0,
             0,   0,   5,   5,   5,   5,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0, },
.capture = { 100, 200, 300, 400, 500, 600} },
};

const Square mirror_score[128] =
{
    Square::a8, Square::b8, Square::c8, Square::d8, Square::e8, Square::f8, Square::g8, Square::h8,
	Square::a7, Square::b7, Square::c7, Square::d7, Square::e7, Square::f7, Square::g7, Square::h7,
	Square::a6, Square::b6, Square::c6, Square::d6, Square::e6, Square::f6, Square::g6, Square::h6,
	Square::a5, Square::b5, Square::c5, Square::d5, Square::e5, Square::f5, Square::g5, Square::h5,
	Square::a4, Square::b4, Square::c4, Square::d4, Square::e4, Square::f4, Square::g4, Square::h4,
	Square::a3, Square::b3, Square::c3, Square::d3, Square::e3, Square::f3, Square::g3, Square::h3,
	Square::a2, Square::b2, Square::c2, Square::d2, Square::e2, Square::f2, Square::g2, Square::h2,
	Square::a1, Square::b1, Square::c1, Square::d1, Square::e1, Square::f1, Square::g1, Square::h1, Square::no_sq,
};
// clang-format on

int Score_value(piece::Type piece) { return Scores[to_underlying(piece)].value; }

int Score_position(piece::Type piece, Color color, Square square) {
    if (color == Color::BLACK) square = mirror_score[to_underlying(square)];
    return Scores[to_underlying(piece)].position[to_underlying(square)];
}

U32 Score_move(const Stats &stats, Move move) {
    const int piece = to_underlying(move.piece().type);
    const int capture = to_underlying(move.piece_capture().type);

    if (move.is_capture()) return Scores[piece].capture[capture] + 10000;
    if (stats.killer[0][stats.ply] == move) return 9000;
    if (stats.killer[1][stats.ply] == move) return 8000;

    return stats.history[piece][to_underlying(move.target())];
}

void Score_move_list(const Stats &stats, MoveList &list) {
    for (auto &moveE : list) {
        moveE.score = Score_move(stats, moveE.move);
    }
}
