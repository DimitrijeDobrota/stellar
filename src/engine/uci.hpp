#ifndef STELLAR_UCI_H
#define STELLAR_UCI_H

#include "board.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "utils.hpp"

namespace uci {

struct Settings {
    MoveList searchmoves;
    Board board;
    uint32_t starttime;
    uint32_t stoptime;
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
void move_print(const Board &board, Move move);
void communicate(const uci::Settings *settings);
uint32_t get_time_ms(void);

} // namespace uci

#endif
