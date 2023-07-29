#ifndef STELLAR_SCORE_H
#define STELLAR_SCORE_H

#include "board.h"

int Score_capture(ePiece src, ePiece tgt);
int Score_position(ePiece piece, eColor color, Square square);
int Score_value(ePiece piece);

#endif
