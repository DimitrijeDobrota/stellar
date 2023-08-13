#ifndef STELLAR_MOVE_LIST_H
#define STELLAR_MOVE_LIST_H

#include "board.hpp"
#include "move.hpp"
#include "utils_cpp.hpp"

#include <functional>
#include <iostream>
#include <vector>

class MoveList {
  public:
    struct MoveListE {
        Move move;
        U32 score;

        auto operator<=>(const MoveListE &mle) { return mle.score <=> score; }
    };

  private:
    using list_t = std::vector<MoveListE>;

  public:
    MoveList(const Board &board) : list() {
        list.reserve(256);
        generate(board);
    }

    auto size() const { return list.size(); }

    MoveListE &operator[](size_t idx) { return list[idx]; }
    const MoveListE &operator[](size_t idx) const { return list[idx]; }

    friend std::ostream &operator<<(std::ostream &os, const MoveList &list);

    using iterator = list_t::iterator;
    using const_iterator = list_t::const_iterator;

    iterator begin() { return list.begin(); }
    iterator end() { return list.end(); }

    const_iterator begin() const { return list.begin(); }
    const_iterator end() const { return list.end(); }
    const_iterator cbegin() const { return list.cbegin(); }
    const_iterator cend() const { return list.cend(); }

  private:
    void push_back(const Move &&move) { list.push_back({move, 0}); }

    void generate(const Board &board);

    void clear() { list.clear(); }

    list_t list;
};

#endif
