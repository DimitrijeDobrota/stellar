#include "moves.hpp"
#include "utils_cpp.hpp"

#include <algorithm>
#include <cstdio>

int move_cmp(Move a, Move b) { return *(uint32_t *)&a == *(uint32_t *)&b; }

Move move_encode(uint8_t src, uint8_t tgt, const piece::Piece *piece,
                 const piece::Piece *capture, const piece::Piece *promote,
                 bool dbl, bool enpassant, bool castle) {
    return (Move){
        .source = src,
        .target = tgt,
        .piece = piece->index,
        .piece_capture = capture ? capture->index : 0u,
        .piece_promote = promote ? promote->index : 0u,
        .dbl = dbl,
        .enpassant = enpassant,
        .castle = castle,
        .capture = capture != NULL,
        .promote = promote != NULL,
    };
}

void move_print(Move move) {
    printf("%5s %5s  %2c  %2c   %2c %4d %4d %4d %4d %4d\n",
           square_to_coordinates(static_cast<Square>(move_source(move))),
           square_to_coordinates(static_cast<Square>(move_target(move))),
           move_piece(move).code,
           move_capture(move) ? move_piece_capture(move).code : '.',
           move_promote(move) ? move_piece_promote(move).code : '.',
           move_double(move) ? 1 : 0, move_enpassant(move) ? 1 : 0,
           move_castle(move) ? 1 : 0, move_capture(move) ? 1 : 0,
           move_promote(move) ? 1 : 0);
}

void move_list_sort(std::vector<MoveE> &list) {
    std::sort(list.begin(), list.end(),
              [](const MoveE &a, const MoveE &b) { return a.score < b.score; });
}

void move_list_print(const std::vector<MoveE> &list) {
    printf("Score   From    To  Pi  Cap  Prmt  Dbl  Enp  Cst  C   P\n");
    for (const MoveE &move : list) {
        printf("%5d: ", move.score);
        move_print(move.move);
    }
    printf("Total: %lu\n", list.size());
}
