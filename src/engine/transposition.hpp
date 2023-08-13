#ifndef STELLAR_TRANSPOSITION_H
#define STELLAR_TRANSPOSITION_H

#include "move.hpp"
#include "stats.hpp"
#include "utils_cpp.hpp"

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
    int depth;
    int score;
    HasheFlag flag;
};

class TTable {
  public:
    TTable(U64 size) : table(size, {0}) {}

    void clear() { table.clear(); };
    void write(const Stats &stats, Move best, int score, int depth, HasheFlag flag);
    int read(const Stats &stats, Move *best, int alpha, int beta, int depth) const;

  private:
    std::vector<Hashe> table;
};

#undef T
#endif
