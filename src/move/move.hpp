#ifndef STELLAR_MOVES_H
#define STELLAR_MOVES_H

#include "board.hpp"
#include "piece.hpp"
#include "utils.hpp"

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
        PQUIET,
        PKNIGHT = 8,
        PBISHOP,
        PROOK,
        PQUEEN,
        PCKNIGHT,
        PCBISHOP,
        PCROOK,
        PCQUEEN,
    };

    Move() : source_i(0), target_i(0), flags_i(0) {}
    Move(square::Square source, square::Square target, Flag flags)
        : source_i(source), target_i(target), flags_i(flags) {}

    friend bool operator==(const Move a, const Move b) {
        return a.source_i == b.source_i && a.target_i == b.target_i && a.flags_i == b.flags_i;
    }

    [[nodiscard]] square::Square source() const { return static_cast<square::Square>(source_i); }
    [[nodiscard]] square::Square target() const { return static_cast<square::Square>(target_i); }

    [[nodiscard]] bool is_capture() const { return flags_i != PQUIET && (flags_i & CAPTURE); }
    [[nodiscard]] bool is_promote() const { return flags_i & 0x8; }

    [[nodiscard]] bool is_double() const { return flags_i == DOUBLE; }
    [[nodiscard]] bool is_repeatable() const { return flags_i == QUIET; }
    [[nodiscard]] bool is_quiet() const { return flags_i == QUIET || flags_i == PQUIET; }

    [[nodiscard]] bool is_castle() const { return flags_i == CASTLEK || flags_i == CASTLEQ; }
    [[nodiscard]] bool is_castle_king() const { return flags_i == CASTLEK; }
    [[nodiscard]] bool is_castle_queen() const { return flags_i == CASTLEQ; }

    [[nodiscard]] bool is_enpassant() const { return flags_i == ENPASSANT; }

    [[nodiscard]] const piece::Type promoted() const { return static_cast<piece::Type>((flags_i & 0x3) + 1); }

    bool make(Board &board) const;

    operator std::string() const;
    friend std::ostream &operator<<(std::ostream &os, Move move);
    void print() const;

  private:
    inline void piece_remove(Board &board, piece::Type type, color::Color color, square::Square square) const;
    inline void piece_set(Board &board, piece::Type type, color::Color color, square::Square square) const;
    inline void piece_move(Board &board, piece::Type type, color::Color color, square::Square source,
                           square::Square target) const;

    unsigned source_i : 6;
    unsigned target_i : 6;
    unsigned flags_i : 4;
};

#endif
