#include "score.h"
#include "moves.h"
#include "stats.h"

// clang-format off
struct Score_T {
  int value;
  int position[64];
  int capture[6];
};

static const struct Score_T Scores[] = {
[PAWN] = {
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
.capture = {   [PAWN] = 105, [KNIGHT] = 205,
             [BISHOP] = 305,   [ROOK] = 405,
              [QUEEN] = 505,   [KING] = 605} },
[KNIGHT] = {
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
.capture = {   [PAWN] = 104, [KNIGHT] = 204,
             [BISHOP] = 304,   [ROOK] = 404,
              [QUEEN] = 504,   [KING] = 604} },
[BISHOP] = {
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
.capture = {   [PAWN] = 103, [KNIGHT] = 203,
             [BISHOP] = 303,   [ROOK] = 403,
              [QUEEN] = 503,   [KING] = 603} },
[ROOK] = {
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
.capture = {   [PAWN] = 102, [KNIGHT] = 202,
             [BISHOP] = 302,   [ROOK] = 402,
              [QUEEN] = 502,   [KING] = 602} },
[QUEEN] = {
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
.capture = {   [PAWN] = 101, [KNIGHT] = 201,
             [BISHOP] = 301,   [ROOK] = 401,
              [QUEEN] = 501,   [KING] = 601} },
[KING] = {
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
.capture = {   [PAWN] = 100, [KNIGHT] = 200,
             [BISHOP] = 300,   [ROOK] = 400,
              [QUEEN] = 500,   [KING] = 600} },
};

const int mirror_score[128] =
{
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1, no_sq,
};
// clang-format on

int Score_value(ePiece piece) { return Scores[piece].value; }

int Score_position(ePiece piece, eColor color, Square square) {
    if (color == BLACK) square = mirror_score[square];
    return Scores[piece].position[square];
}

int Score_capture(ePiece src, ePiece tgt) {
    return Scores[src].capture[tgt] + 10000;
}

int Score_move(const Stats *stats, Move move) {
    if (move_capture(move)) {
        return Score_capture(piece_piece(move_piece(move)),
                             piece_piece(move_piece_capture(move)));
    }

    if (move_cmp(stats->killer[0][stats->ply], move))
        return 9000;
    else if (move_cmp(stats->killer[1][stats->ply], move))
        return 8000;

    return stats->history[piece_index(move_piece(move))][move_target(move)];
}

void Score_move_list(const Stats *stats, MoveList *list) {
    for (int i = 0; i < move_list_size(list); i++) {
        list->moves[i].score = Score_move(stats, move_list_index_move(list, i));
    }
}
