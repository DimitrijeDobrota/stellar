#ifndef STELLAR_ZOBRIST_H
#define STELLAR_ZOBRIST_H

#include "board.h"

void zobrist_init(void);
U64 zobrist_hash(const Board *board);

#endif
