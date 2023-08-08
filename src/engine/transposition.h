#ifndef STELLAR_TRANSPOSITION_H
#define STELLAR_TRANSPOSITION_H

#include "stats.h"
#include "utils.h"

#define TTABLE_UNKNOWN 100000

#define T TTable

typedef enum HasheFlag HasheFlag;
enum HasheFlag {
    flagExact,
    flagAlpha,
    flagBeta
};

typedef struct T T;

T *ttable_new(U64 size);
void ttable_free(T **self);
void ttable_clear(T *self);

int ttable_read(const Stats *stats, int alpha, int beta, int depth);
void ttable_write(const Stats *stats, int score, int depth, HasheFlag flag);

#undef T
#endif