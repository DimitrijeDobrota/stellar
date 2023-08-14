#ifndef STELLAR_MOVES_H
#define STELLAR_MOVES_H

#include "board.hpp"
#include "piece.hpp"
#include "utils_cpp.hpp"

#include <iostream>
#include <vector>

struct Move {
    Move() = default;

    Move(Square source, Square target, const piece::Piece *piece, const piece::Piece *capture,
         const piece::Piece *promote, bool dbl, bool enpassant, bool castle)
        : source_i(to_underlying(source)), target_i(to_underlying(target)), piece_i(piece->index),
          piece_capture_i(capture ? capture->index : 0), piece_promote_i(promote ? promote->index : 0),
          dbl(dbl), enpassant(enpassant), castle(castle), capture(capture != NULL), promote(promote != NULL) {
    }

    bool operator==(const Move &m) const = default;

    Square source(void) const { return static_cast<Square>(source_i); }
    Square target(void) const { return static_cast<Square>(target_i); }

    bool is_double(void) const { return dbl; }
    bool is_enpassant(void) const { return enpassant; }
    bool is_castle(void) const { return castle; }
    bool is_capture(void) const { return capture; }
    bool is_promote(void) const { return promote; }

    const piece::Piece &piece(void) const { return piece::get_from_index(piece_i); }
    const piece::Piece &piece_capture(void) const { return piece::get_from_index(piece_capture_i); }
    const piece::Piece &piece_promote(void) const { return piece::get_from_index(piece_promote_i); }

    bool make(Board &board, bool attack_only) const;

    friend std::ostream &operator<<(std::ostream &os, Move move);

  private:
    inline void piece_remove(Board &board, const piece::Piece &piece, Square square) const;
    inline void piece_set(Board &board, const piece::Piece &piece, Square square) const;
    inline void piece_move(Board &board, const piece::Piece &piece, Square source, Square target) const;

    unsigned source_i : 6;
    unsigned target_i : 6;
    unsigned piece_i : 5;
    unsigned piece_capture_i : 5;
    unsigned piece_promote_i : 5;
    bool dbl : 1;
    bool enpassant : 1;
    bool castle : 1;
    bool capture : 1;
    bool promote : 1;
};

#endif
