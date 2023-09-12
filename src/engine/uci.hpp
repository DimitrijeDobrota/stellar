#ifndef STELLAR_UCI_H
#define STELLAR_UCI_H

#include "board.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "score.hpp"
#include "utils.hpp"

namespace engine {

class PVTable;

} // namespace engine

namespace uci {

struct Settings {
    mutable Board board;

    MoveList searchMoves;
    MoveList madeMoves;

    uint64_t starttime;
    uint64_t stoptime;
    uint16_t depth = 64;

    uint32_t nodes = 0;
    bool ponder = false;
    bool debug = false;
    bool mate = false;
    bool infinite = true;
    bool newgame = true;

    mutable bool stopped = false;
};

void loop(void);
uint32_t get_time_ms(void);
void communicate(const uci::Settings *settings);
void pv_print(int16_t score, uint8_t depth, uint64_t nodes, const engine::PVTable &pvtable);

} // namespace uci
#endif
