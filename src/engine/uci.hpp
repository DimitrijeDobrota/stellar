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
    uint32_t wtime = 0;
    uint32_t btime = 0;
    uint32_t movetime = 0;
    uint32_t nodes = 0;
    uint16_t depth = 0;
    uint16_t winc = 0;
    uint16_t binc = 0;
    bool ponder = false;
    bool debug = false;
    bool mate = false;
    bool infinite = false;
};

void loop(void);
void move_print(const Board &board, Move move);

} // namespace uci

#endif
