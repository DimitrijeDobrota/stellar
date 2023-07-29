#ifndef STELLAR_PERFT_H
#define STELLAR_PERFT_H

#include "board.h"

void perft_test_threaded(Board board, int depth);
void perft_test(Board board, int depth);

#endif
