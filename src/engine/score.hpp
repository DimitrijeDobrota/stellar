#ifndef STELLAR_SCORE_H
#define STELLAR_SCORE_H

#include "board.hpp"
#include "movelist.hpp"
#include "stats.hpp"

#define SCORE_INFINITY 50000
#define MATE_VALUE 49000
#define MATE_SCORE 48000

U32 Score_move(const Stats &stats, Move move);
void Score_move_list(const Stats &stats, MoveList &list);
int Score_position(piece::Type piece, Color color, Square square);
int Score_capture(piece::Type src, piece::Type tgt);
int Score_value(piece::Type piece);

#endif
