#ifndef STELLAR_MOVE_LIST_H
#define STELLAR_MOVE_LIST_H

#include "board.hpp"
#include "move.hpp"
#include "utils.hpp"

#include <iostream>
#include <numeric>
#include <vector>

class MoveList {
  private:
    using list_t = std::vector<Move>;

  public:
    MoveList() : list(){};
    MoveList(const Board &board, bool attacks_only = false) : list() {
        list.reserve(256);
        generate(board, attacks_only);
    }

    MoveList(const Board &board, bool attacks_only, bool legal) : MoveList(board, attacks_only) {
        int size = 0;
        for (const auto &move : list) {
            Board copy = board;
            if (move.make(copy)) list[size++] = move;
        }
        list.resize(size);
    }

    void clear() { list.clear(); }
    [[nodiscard]] int size() const { return list.size(); }

    const Move operator[](size_t idx) const { return list[idx]; }
    Move &operator[](size_t idx) { return list[idx]; }
    void push(const Move move) { list.push_back(move); }

    friend std::ostream &operator<<(std::ostream &os, const MoveList &list);

  private:
    void generate(const Board &board, bool attacks_only);

    list_t list;
};

#endif
