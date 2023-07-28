#ifndef MOVES_H
#define MOVES_H

#include <stdint.h>

#include "CBoard.h"

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

typedef struct MoveList_T *MoveList_T;
struct MoveList_T {
    Move moves[256];
    int count;
};

int Move_cmp(Move a, Move b);
Move Move_encode(Square src, Square tgt, Piece_T Piece, Piece_T Capture,
                 Piece_T Promote, int dbl, int enpassant, int castle);
void Move_print(Move move);
MoveList_T MoveList_new(void);
void MoveList_free(MoveList_T *p);
Move MoveList_move(MoveList_T self, int index);
int MoveList_size(MoveList_T self);
void MoveList_reset(MoveList_T self);
void MoveList_add(MoveList_T self, Move move);
void MoveList_print(MoveList_T self);
MoveList_T MoveList_generate(MoveList_T moves, CBoard_T board);
int Move_make(Move move, CBoard_T board, int flag);

#define Move_source(move) (move.source)
#define Move_target(move) (move.target)
#define Move_double(move) (move.dbl)
#define Move_enpassant(move) (move.enpassant)
#define Move_castle(move) (move.castle)
#define Move_capture(move) (move.capture)
#define Move_promote(move) (move.promote)

#define Move_piece(move) (Piece_fromIndex(move.piece))
#define Move_piece_capture(move) (Piece_fromIndex(move.piece_capture))
#define Move_piece_promote(move) (Piece_fromIndex(move.piece_promote))

#endif
