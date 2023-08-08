#ifndef STELLAR_SCORE_H
#define STELLAR_SCORE_H

#include "board.h"
#include "moves.h"
#include "stats.h"

int Score_position(ePiece piece, eColor color, Square square);
void Score_move_list(const Stats *stats, MoveList *list);
int Score_capture(ePiece src, ePiece tgt);
int Score_value(ePiece piece);

#endif
