#ifndef STELLAR_MOVES_H
#define STELLAR_MOVES_H

#include "board.hpp"
#include "piece.hpp"
#include "utils_cpp.hpp"

#include <iostream>
#include <vector>

struct Move {
    Move() = default;

    Move(Square source, Square target, piece::Type piece, piece::Type capture, piece::Type promote, bool dbl,
         bool enpassant, bool castle)
        : source_i(to_underlying(source)), target_i(to_underlying(target)), piece_i(to_underlying(piece)),
          capture_i(to_underlying(capture)), promote_i(to_underlying(promote)), dbl(dbl),
          enpassant(enpassant), castle(castle) {}

    bool operator==(const Move &m) const = default;

    Square source(void) const { return static_cast<Square>(source_i); }
    Square target(void) const { return static_cast<Square>(target_i); }

    bool is_double(void) const { return dbl; }
    bool is_enpassant(void) const { return enpassant; }
    bool is_castle(void) const { return castle; }
    bool is_capture(void) const { return capture_i != to_underlying(piece::Type::NONE); }
    bool is_promote(void) const { return promote_i != to_underlying(piece::Type::NONE); }

    const piece::Type piece(void) const { return static_cast<piece::Type>(piece_i); }
    const piece::Type captured(void) const { return static_cast<piece::Type>(capture_i); }
    const piece::Type promoted(void) const { return static_cast<piece::Type>(promote_i); }

    bool make(Board &board, bool attack_only) const;

    friend std::ostream &operator<<(std::ostream &os, Move move);

  private:
    inline void piece_remove(Board &board, piece::Type type, Color color, Square square) const;
    inline void piece_set(Board &board, piece::Type type, Color color, Square square) const;
    inline void piece_move(Board &board, piece::Type type, Color color, Square source, Square target) const;

    unsigned source_i : 6;
    unsigned target_i : 6;
    unsigned piece_i : 3;
    unsigned capture_i : 3;
    unsigned promote_i : 3;
    bool dbl : 1;
    bool enpassant : 1;
    bool castle : 1;
};

#endif
