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
    MoveList(const Board &board, bool attacks_only = false, bool legal = false) : list() {
        list.reserve(256);
        generate(board, attacks_only);
        if (!legal) return;

        int size = 0;
        for (int i = 0; i < list.size(); i++) {
            Board copy = board;
            if (list[i].make(copy)) list[size++] = list[i];
        }
        list.resize(size);
    }

    void clear() { list.clear(); }
    int size() const { return list.size(); }

    const Move operator[](size_t idx) const { return list[idx]; }
    void push(const Move move) { list.push_back(move); }

    friend std::ostream &operator<<(std::ostream &os, const MoveList &list);

  private:
    void generate(const Board &board, bool attacks_only);

    list_t list;
};

#endif
