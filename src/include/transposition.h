#ifndef STELLAR_TRANSPOSITION_H
#define STELLAR_TRANSPOSITION_H

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

int ttable_read(T *self, U64 hash, int alpha, int beta, int depth, int ply);
void ttable_write(T *self, U64 hash, int score, int depth, int ply,
                  HasheFlag flag);

#undef T
#endif
