#ifndef STELLAR_ARENA_GAME_H
#define STELLAR_ARENA_GAME_H

#include "movelist.hpp"

class Game {
  public:
    static const std::string startPosition;
    enum Terminate {
        Deatch,
        Timeout,
        Illegal,
        Repetition,
    };

    Game(const uint16_t match_id, const std::string white, const std::string black, const std::string fen);
    ~Game();

    void play(const Move move) { list.push(move); }

    const std::string get_moves(void) const;
    const std::string &get_white(void) const { return white; }
    const std::string &get_black(void) const { return black; }
    const Terminate get_terminate(void) const { return terminate; }

    const bool is_win_white(void) const { return !draw && winner == color::WHITE; }
    const bool is_win_black(void) const { return !draw && winner == color::BLACK; }
    const bool is_draw(void) const { return draw; }

    void set_terminate(const Terminate terminate) { this->terminate = terminate; }
    void set_winner(const color::Color winner) { this->winner = winner; }
    void set_draw(const bool draw) { this->draw = draw; }

    static void set_san(bool san) { Game::san = san; }

    friend std::ostream &operator<<(std::ostream &os, const Game &game);

  private:
    static const std::string to_san(const Board &board, const Move move);

    static uint16_t id_t;
    uint16_t id = id_t++;
    uint16_t match_id;

    const std::string white, black;
    const std::string fen;
    MoveList list;

    bool draw = false;
    color::Color winner;
    Terminate terminate = Terminate::Deatch;

    static bool san;
};

#endif
