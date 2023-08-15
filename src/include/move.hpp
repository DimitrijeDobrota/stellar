#ifndef STELLAR_MOVES_H
#define STELLAR_MOVES_H

#include "board.hpp"
#include "piece.hpp"
#include "utils_cpp.hpp"

#include <iostream>
#include <vector>

struct Move {
    enum Flag : uint8_t {
        QUIET,
        DOUBLE,
        CASTLEK,
        CASTLEQ,
        CAPTURE,
        ENPASSANT,
        PKNIGHT = 8,
        PBISHOP,
        PROOK,
        PQUEEN,
        PCKNIGHT,
        PCBISHOP,
        PCROOK,
        PCQUEEN,
    };

    Move() = default;
    Move(Square source, Square target, Flag flags)
        : source_i(to_underlying(source)), target_i(to_underlying(target)), flags_i(flags) {}

    bool operator==(Move &m) const {
        return source_i == m.source_i && target_i == m.target_i && flags_i == m.flags_i;
    }

    Square source(void) const { return static_cast<Square>(source_i); }
    Square target(void) const { return static_cast<Square>(target_i); }

    bool is_capture(void) const { return flags_i & CAPTURE; }
    bool is_promote(void) const { return flags_i & 0x8; }

    bool is_quiet(void) const { return flags_i == QUIET; }
    bool is_double(void) const { return flags_i == DOUBLE; }

    bool is_castle(void) const { return flags_i == CASTLEK || flags_i == CASTLEQ; }
    bool is_castle_king(void) const { return flags_i == CASTLEK; }
    bool is_castle_queen(void) const { return flags_i == CASTLEQ; }

    bool is_enpassant(void) const { return flags_i == ENPASSANT; }

    const piece::Type promoted(void) const { return static_cast<piece::Type>((flags_i & 0x3) + 1); }

    bool make(Board &board, bool attack_only) const;

    friend std::ostream &operator<<(std::ostream &os, Move move);

  private:
    inline void piece_remove(Board &board, piece::Type type, Color color, Square square) const;
    inline void piece_set(Board &board, piece::Type type, Color color, Square square) const;
    inline void piece_move(Board &board, piece::Type type, Color color, Square source, Square target) const;

    unsigned source_i : 6;
    unsigned target_i : 6;
    unsigned flags_i : 4;
};

#endif
