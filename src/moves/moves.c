#include <stdio.h>
#include <stdlib.h>

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
    printf("%5s %5s  %2s  %2s   %2s %4d %4d %4d %4d %4d\n",
           square_to_coordinates[move_source(move)],
           square_to_coordinates[move_target(move)],
           piece_unicode(move_piece(move)),
           move_capture(move) ? piece_unicode(move_piece_capture(move)) : "X ",
           move_promote(move) ? piece_unicode(move_piece_promote(move)) : "X ",
           move_double(move) ? 1 : 0, move_enpassant(move) ? 1 : 0,
           move_castle(move) ? 1 : 0, move_capture(move) ? 1 : 0,
           move_promote(move) ? 1 : 0);
}

MoveList move_list_new(void) {
    MoveList p;
    NEW0(p);
    return p;
}

void move_list_free(MoveList *p) { FREE(*p); }

Move move_list_move(MoveList self, int index) { return self->moves[index]; }
int move_list_size(MoveList self) { return self->count; }
void move_list_reset(MoveList self) { self->count = 0; }

void move_list_add(MoveList self, Move move) {
    self->moves[self->count++] = move;
}

void move_list_print(MoveList self) {
    printf(" From    To  Pi  Cap  Prmt  Dbl  Enp  Cst  C   P\n");
    for (int i = 0; i < self->count; i++)
        move_print(self->moves[i]);
    printf("Total: %d\n", self->count);
}

#define pawn_canPromote(color, source)                                         \
    ((color == WHITE && source >= a7 && source <= h7) ||                       \
     (color == BLACK && source >= a2 && source <= h2))

#define pawn_onStart(color, source)                                            \
    ((color == BLACK && source >= a7 && source <= h7) ||                       \
     (color == WHITE && source >= a2 && source <= h2))

#define pawn_promote(source, target, Piece, Capture)                           \
    for (int i = 1; i < 5; i++) {                                              \
        move = move_encode(source, target, Piece, Capture,                     \
                           piece_get(i, color), 0, 0, 0);                      \
        move_list_add(moves, move);                                            \
    }

MoveList move_list_generate(MoveList moves, Board board) {
    Move move;
    Square src, tgt;
    eColor color = board_side(board);

    if (!moves) moves = move_list_new();

    { // pawn moves
        Piece Piece = piece_get(PAWN, color);
        U64 bitboard = board_pieceSet(board, Piece);
        bitboard_for_each_bit(src, bitboard) {
            { // quiet
                int add = (color == WHITE) ? +8 : -8;
                tgt = src + add;
                if (tgt > a1 && tgt < h8 &&
                    !board_square_isOccupied(board, tgt)) {
                    if (pawn_canPromote(color, src)) {
                        pawn_promote(src, tgt, Piece, 0);
                    } else {
                        move_list_add(
                            moves, move_encode(src, tgt, Piece, 0, 0, 0, 0, 0));

                        // two ahead
                        if (pawn_onStart(color, src) &&
                            !board_square_isOccupied(board, tgt += add))
                            move_list_add(moves, move_encode(src, tgt, Piece, 0,
                                                             0, 1, 0, 0));
                    }
                }
            }
            { // capture
                U64 attack = board_piece_attacks(board, Piece, src) &
                             board_colorBB(board, !color);
                bitboard_for_each_bit(tgt, attack) {
                    if (pawn_canPromote(color, src)) {
                        pawn_promote(src, tgt, Piece,
                                     board_square_piece(board, tgt, !color));
                    } else {
                        move_list_add(
                            moves,
                            move_encode(src, tgt, Piece,
                                        board_square_piece(board, tgt, !color),
                                        0, 0, 0, 0));
                    }
                }
            }

            { // en passant
                if (board_enpassant(board) != no_sq &&
                    board_piece_attacks(board, Piece, src) &
                        (C64(1) << board_enpassant(board)))
                    move_list_add(
                        moves,
                        move_encode(src, board_enpassant(board), Piece,
                                    board_square_piece(board, tgt, !color), 0,
                                    0, 1, 0));
            }
        }
    }

    // All piece move
    for (int piece = 1; piece < 6; piece++) {
        Piece Piece = piece_get(piece, color);
        U64 bitboard = board_pieceSet(board, Piece);
        bitboard_for_each_bit(src, bitboard) {
            U64 attack = board_piece_attacks(board, Piece, src) &
                         ~board_colorBB(board, color);
            bitboard_for_each_bit(tgt, attack) {
                /* int take = bit_get(board_colorBB(board, !color), tgt); */
                move_list_add(
                    moves, move_encode(src, tgt, Piece,
                                       board_square_piece(board, tgt, !color),
                                       0, 0, 0, 0));
            }
        }
    }

    // Castling
    {
        if (color == WHITE) {
            Piece Piece = piece_get(KING, WHITE);
            if (board_castle(board) & WK) {
                if (!board_square_isOccupied(board, f1) &&
                    !board_square_isOccupied(board, g1) &&
                    !board_square_isAttack(board, e1, BLACK) &&
                    !board_square_isAttack(board, f1, BLACK))
                    move_list_add(moves,
                                  move_encode(e1, g1, Piece, 0, 0, 0, 0, 1));
            }
            if (board_castle(board) & WQ) {
                if (!board_square_isOccupied(board, d1) &&
                    !board_square_isOccupied(board, c1) &&
                    !board_square_isOccupied(board, b1) &&
                    !board_square_isAttack(board, e1, BLACK) &&
                    !board_square_isAttack(board, d1, BLACK))
                    move_list_add(moves,
                                  move_encode(e1, c1, Piece, 0, 0, 0, 0, 1));
            }
        } else {
            Piece Piece = piece_get(KING, BLACK);
            if (board_castle(board) & BK) {
                if (!board_square_isOccupied(board, f8) &&
                    !board_square_isOccupied(board, g8) &&
                    !board_square_isAttack(board, e8, WHITE) &&
                    !board_square_isAttack(board, f8, WHITE))
                    move_list_add(moves,
                                  move_encode(e8, g8, Piece, 0, 0, 0, 0, 1));
            }
            if (board_castle(board) & BQ) {
                if (!board_square_isOccupied(board, d8) &&
                    !board_square_isOccupied(board, c8) &&
                    !board_square_isOccupied(board, b8) &&
                    !board_square_isAttack(board, e8, WHITE) &&
                    !board_square_isAttack(board, d8, WHITE))
                    move_list_add(moves,
                                  move_encode(e8, c8, Piece, 0, 0, 0, 0, 1));
            }
        }
    }

    return moves;
}

// clang-format off
const int castling_rights[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7,  15, 15, 15, 3,  15, 15, 11,
};
// clang-format on

int move_make(Move move, Board board, int flag) {
    if (flag == 0) {

        Square source = move_source(move);
        Square target = move_target(move);
        Piece piece = move_piece(move);
        eColor color = board_side(board);

        if (!move_capture(move))
            board_piece_move(board, piece, source, target);
        else
            board_piece_capture(board, piece, move_piece_capture(move), source,
                                target);

        if (move_promote(move)) {
            board_piece_pop(board, piece, target);
            board_piece_set(board, move_piece_promote(move), target);
        }

        {
            int ntarget = target + (color == WHITE ? -8 : +8);
            if (move_enpassant(move))
                board_piece_pop(board, piece_get(PAWN, !color), ntarget);

            board_enpassant_set(board, move_double(move) ? ntarget : no_sq);
        }

        if (move_castle(move)) {
            Piece Rook = piece_get(ROOK, board_side(board));
            switch (target) {
            case g1:
                board_piece_move(board, Rook, h1, f1);
                break;
            case c1:
                board_piece_move(board, Rook, a1, d1);
                break;
            case g8:
                board_piece_move(board, Rook, h8, f8);
                break;
            case c8:
                board_piece_move(board, Rook, a8, d8);
                break;
            default:
                break;
            }
        }

        board_castle_and(board, castling_rights[source]);
        board_castle_and(board, castling_rights[target]);

        if (!board_isCheck(board)) {
            board_side_switch(board);
            return 1;
        } else
            return 0;
    } else {
        if (move_capture(move))
            return move_make(move, board, 0);
        else
            return 0;
    }
}
