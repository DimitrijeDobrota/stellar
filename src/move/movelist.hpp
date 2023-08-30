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
    using index_t = std::vector<int>;

  public:
    MoveList() : list(){};
    MoveList(const Board &board) : list() {
        list.reserve(256);
        generate(board);
    }

    void clear() { list.clear(); }
    int size() const { return list.size(); }

    const Move operator[](size_t idx) const { return list[idx]; }
    void push(const Move move) { list.push_back(move); }

    friend std::ostream &operator<<(std::ostream &os, const MoveList &list);

  private:
    void generate(const Board &board);

    list_t list;
};

#endif
