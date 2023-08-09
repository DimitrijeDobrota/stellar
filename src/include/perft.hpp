#ifndef STELLAR_PERFT_H
#define STELLAR_PERFT_H

#include "board.hpp"

typedef unsigned long long ull;
typedef struct PerftResult PerftResult;
struct PerftResult {
    ull node;
#ifdef USE_FULL_COUNT
    ull capture;
    ull enpassant;
    ull castle;
    ull promote;
    ull check;
    // ull checkDiscovered;
    // ull checkDouble;
    // ull checkmate;
#endif
};

PerftResult perft_test(const char *fen, int depth, int thread_num);

#endif
