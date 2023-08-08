#ifndef STELLAR_MOVES_H
#define STELLAR_MOVES_H

#include <stdbool.h>
#include <stdint.h>

#include "board.h"

typedef struct Move Move;
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

typedef struct MoveList MoveList;
typedef struct MoveE MoveE;

struct MoveList {
    int count;
    struct MoveE {
        Move move;
        int score;
    } moves[256];
};

int move_cmp(Move a, Move b);
Move move_encode(Square src, Square tgt, Piece piece, Piece capture,
                 Piece promote, int dbl, int enpassant, int castle);
void move_print(Move move);
MoveList *move_list_new(void);

void move_list_free(MoveList **p);
Move move_list_index_move(const MoveList *self, int index);
int move_list_index_score(const MoveList *self, int index);
void move_list_index_score_set(MoveList *self, int index, int score);

int move_make(Move move, Board *board, int flag);

int move_list_size(const MoveList *self);
void move_list_reset(MoveList *self);
void move_list_add(MoveList *self, Move move);
void move_list_print(const MoveList *self);
void move_list_sort(MoveList *list);

MoveList *move_list_generate(MoveList *moves, const Board *board);

#define move_source(move) (move.source)
#define move_target(move) (move.target)
#define move_double(move) (move.dbl)
#define move_enpassant(move) (move.enpassant)
#define move_castle(move) (move.castle)
#define move_capture(move) (move.capture)
#define move_promote(move) (move.promote)

#define move_piece(move) (piece_from_index(move.piece))
#define move_piece_capture(move) (piece_from_index(move.piece_capture))
#define move_piece_promote(move) (piece_from_index(move.piece_promote))

#endif
