#ifndef MOVES_H
#define MOVES_H

#include <stdint.h>

#include "board.h"

typedef struct Move Move;
struct Move {
    unsigned source : 6;
    unsigned target : 6;
    unsigned piece : 5;
    unsigned piece_capture : 5;
    unsigned piece_promote : 5;
    unsigned dbl : 1;
    unsigned enpassant : 1;
    unsigned castle : 1;
    unsigned capture : 1;
    unsigned promote : 1;
};

typedef struct MoveList *MoveList;
struct MoveList {
    Move moves[256];
    int count;
};

int move_cmp(Move a, Move b);
Move move_encode(Square src, Square tgt, Piece piece, Piece capture,
                 Piece promote, int dbl, int enpassant, int castle);
void move_print(Move move);
MoveList move_list_new(void);
void move_list_free(MoveList *p);
Move move_list_move(MoveList self, int index);
int move_list_size(MoveList self);
void move_list_reset(MoveList self);
void move_list_add(MoveList self, Move move);
void move_list_print(MoveList self);
MoveList move_list_generate(MoveList moves, Board board);
int move_make(Move move, Board board, int flag);

#define move_source(move) (move.source)
#define move_target(move) (move.target)
#define move_double(move) (move.dbl)
#define move_enpassant(move) (move.enpassant)
#define move_castle(move) (move.castle)
#define move_capture(move) (move.capture)
#define move_promote(move) (move.promote)

#define move_piece(move) (Piece_fromIndex(move.piece))
#define move_piece_capture(move) (Piece_fromIndex(move.piece_capture))
#define move_piece_promote(move) (Piece_fromIndex(move.piece_promote))

#endif
