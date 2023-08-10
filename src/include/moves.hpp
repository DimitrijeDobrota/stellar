#ifndef STELLAR_MOVES_H
#define STELLAR_MOVES_H

#include "board.hpp"
#include "piece.hpp"

#include <vector>

struct Move {
    unsigned source : 6;
    unsigned target : 6;
    unsigned piece : 5;
    unsigned piece_capture : 5;
    unsigned piece_promote : 5;
    bool dbl : 1;
    bool enpassant : 1;
    bool castle : 1;
    bool capture : 1;
    bool promote : 1;
};

struct MoveE {
    Move move;
    int score;
};

Move move_encode(uint8_t src, uint8_t tgt, const piece::Piece *piece, const piece::Piece *capture,
                 const piece::Piece *promote, bool dbl, bool enpassant, bool castle);
std::vector<MoveE> move_list_generate(const Board &board);
int move_make(Move move, Board &board, int flag);
void move_list_sort(std::vector<MoveE> &list);
int move_cmp(Move a, Move b);
void move_print(Move move);

#define move_source(move) (move.source)
#define move_target(move) (move.target)
#define move_double(move) (move.dbl)
#define move_enpassant(move) (move.enpassant)
#define move_castle(move) (move.castle)
#define move_capture(move) (move.capture)
#define move_promote(move) (move.promote)

#define move_piece(move) (piece::get_from_index(move.piece))
#define move_piece_capture(move) (piece::get_from_index(move.piece_capture))
#define move_piece_promote(move) (piece::get_from_index(move.piece_promote))

#endif
