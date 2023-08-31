#ifndef STELLAR_UCI_H
#define STELLAR_UCI_H

#include "board.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "score.hpp"
#include "utils.hpp"

namespace uci {

struct Settings {
    mutable Board board;

    MoveList searchMoves;
    MoveList madeMoves;

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
void pv_print(int16_t score, uint8_t depth, uint64_t nodes, uint8_t pv_length[MAX_PLY],
              Move pv_table[MAX_PLY][MAX_PLY], const Board &board);
void move_print(const Board &board, Move move);
void communicate(const uci::Settings *settings);
uint32_t get_time_ms(void);

} // namespace uci

#endif
