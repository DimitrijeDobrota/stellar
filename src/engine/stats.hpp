#ifndef STELLAR_STATS_H
#define STELLAR_STATS_H

#include "moves.hpp"
#include "utils_cpp.hpp"

#define MAX_PLY 64

typedef struct Stats Stats;
struct Stats {
    struct TTable &ttable;
    Board &board;
    Move pv_table[MAX_PLY][MAX_PLY];
    Move killer[2][MAX_PLY];
    U32 history[16][64];
    int pv_length[MAX_PLY];
    int follow_pv;
    long nodes;
    int ply;
};

#endif
