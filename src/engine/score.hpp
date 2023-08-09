#ifndef STELLAR_SCORE_H
#define STELLAR_SCORE_H

#include "board.hpp"
#include "moves.hpp"
#include "stats.hpp"

#define INFINITY 50000
#define MATE_VALUE 49000
#define MATE_SCORE 48000

void Score_move_list(const Stats &stats, std::vector<MoveE> &list);
int Score_position(piece::Type piece, Color color, Square square);
int Score_capture(piece::Type src, piece::Type tgt);
int Score_value(piece::Type piece);

#endif
