#include <stdio.h>
#include <stdlib.h>

#include <cul/assert.h>
#include <cul/mem.h>

#include "moves.h"

int move_cmp(Move a, Move b) { return *(uint32_t *)&a == *(uint32_t *)&b; }

Move move_encode(Square src, Square tgt, Piece piece, Piece capture,
                 Piece promote, int dbl, int enpassant, int castle) {
    return (Move){
        .source = src,
        .target = tgt,
        .dbl = dbl,
        .enpassant = enpassant,
        .castle = castle,
        .capture = capture != NULL,
        .promote = promote != NULL,
        .piece = piece_index(piece),
        .piece_capture = capture ? piece_index(capture) : 0,
        .piece_promote = promote ? piece_index(promote) : 0,
    };
}

void move_print(Move move) {
    printf("%5s %5s  %2c  %2c   %2c %4d %4d %4d %4d %4d\n",
           square_to_coordinates[move_source(move)],
           square_to_coordinates[move_target(move)],
           piece_asci(move_piece(move)),
           move_capture(move) ? piece_asci(move_piece_capture(move)) : '.',
           move_promote(move) ? piece_asci(move_piece_promote(move)) : '.',
           move_double(move) ? 1 : 0, move_enpassant(move) ? 1 : 0,
           move_castle(move) ? 1 : 0, move_capture(move) ? 1 : 0,
           move_promote(move) ? 1 : 0);
}

MoveList *move_list_new(void) {
    MoveList *p;
    NEW0(p);
    return p;
}

void move_list_free(MoveList **p) { FREE(*p); }

Move move_list_index_move(const MoveList *self, int index) {
    return self->moves[index].move;
}

int move_list_index_score(const MoveList *self, int index) {
    return self->moves[index].score;
}

void move_list_index_score_set(MoveList *self, int index, int score) {
    self->moves[index].score = score;
}

int move_list_size(const MoveList *self) { return self->count; }
void move_list_reset(MoveList *self) { self->count = 0; }

void move_list_add(MoveList *self, Move move) {
    self->moves[self->count++].move = move;
}

int move_list_cmp(const void *a, const void *b) {
    return ((MoveE *)a)->score <= ((MoveE *)b)->score;
}

void move_list_sort(MoveList *list) {
    qsort(list->moves, list->count, sizeof(MoveE), move_list_cmp);
}

void move_list_print(const MoveList *self) {
    printf("Score   From    To  Pi  Cap  Prmt  Dbl  Enp  Cst  C   P\n");
    for (int i = 0; i < self->count; i++) {
        printf("%5d: ", self->moves[i].score);
        move_print(self->moves[i].move);
    }
    printf("Total: %d\n", self->count);
}
