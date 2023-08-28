#ifndef STELLAR_TRANSPOSITION_H
#define STELLAR_TRANSPOSITION_H

#include "move.hpp"
#include "utils.hpp"

#include <vector>

#define TTABLE_UNKNOWN 100000

#define T TTable

enum class HasheFlag : uint8_t {
    Exact,
    Alpha,
    Beta
};

typedef struct Hashe Hashe;
struct Hashe {
    U64 key;
    Move best;
    uint8_t depth;
    int score;
    HasheFlag flag;
};

class TTable {
  public:
    TTable(U64 size) : table(size, {0}) {}

    void clear() { table.clear(); };
    int read(const Board &board, int ply, Move *best, int alpha, int beta, uint8_t depth) const;
    void write(const Board &board, int ply, Move best, int score, uint8_t depth, HasheFlag flag);

  private:
    std::vector<Hashe> table;
};

#undef T
#endif
