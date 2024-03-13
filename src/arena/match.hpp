#ifndef STELLAR_ARENA_MATCH_H
#define STELLAR_ARENA_MATCH_H

#include "engine.hpp"
#include "game.hpp"

class Match {
  public:
    struct Settings {
        uint64_t time = 30000, inc = 0, movetime = 0;
        uint8_t depth = 0, togo = 0;
    };

    Match(Engine &white, Engine &black);
    ~Match();

    Game play(Settings swhite, Settings sblack, const std::string &fen);

  private:
    static std::string get_go(Settings &swhite, Settings &sblack, Color side);
    static Move parse_move(const MoveList &list, const std::string &move_string);

    std::array<Engine *, 2> engines;

    static uint16_t id_t;
    uint16_t id = id_t++;
};

#endif
