#include <cul/assert.h>
#include <cul/mem.h>
#include <string.h>

#include "board.h"
#include "moves.h"
#include "transposition.h"

#define TTABLE_SIZE 0x400000

#define T TTable

typedef struct Hashe Hashe;
struct Hashe {
    U64 key;
    Move best;
    int depth;
    int score;
    HasheFlag flag;
};

struct T {
    U64 size;
    Hashe table[];
};

T *ttable_new(U64 size) {
    T *self = CALLOC(1, sizeof(T) + size * sizeof(Hashe));
    self->size = size;
    return self;
}

void ttable_free(T **self) {
    assert(self && *self);
    FREE(*self);
}

void ttable_clear(T *self) {
    assert(self);
    memset(self->table, 0x0, sizeof(T) + self->size * sizeof(Hashe));
}

int ttable_read(const Stats *stats, int alpha, int beta, int depth) {
    assert(stats);
    assert(stats->ttable);

    T *self = stats->ttable;
    U64 hash = board_hash(stats->board);

    Hashe *phashe = &self->table[hash % self->size];
    if (phashe->key == hash) {
        if (phashe->depth >= depth) {
            int score = phashe->score;

            if (score < -MATE_SCORE) score += stats->ply;
            if (score > MATE_SCORE) score -= stats->ply;

            if (phashe->flag == flagExact) return score;
            if ((phashe->flag == flagAlpha) && (score <= alpha)) return alpha;
            if ((phashe->flag == flagBeta) && (score >= beta)) return beta;
        }
    }
    return TTABLE_UNKNOWN;
}

void ttable_write(const Stats *stats, int score, int depth, HasheFlag flag) {
    assert(stats);
    assert(stats->ttable);

    T *self = stats->ttable;
    U64 hash = board_hash(stats->board);

    Hashe *phashe = &self->table[hash % self->size];

    if (score < -MATE_SCORE) score += stats->ply;
    if (score > MATE_SCORE) score -= stats->ply;

    *phashe = (Hashe){
        .key = hash,
        .best = 0,
        .depth = depth,
        .score = score,
        .flag = flag,
    };
}
