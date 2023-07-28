#include <stdio.h>
#include <stdlib.h>

#include <cul/mem.h>

#include "moves.h"

int Move_cmp(Move a, Move b) { return *(uint32_t *)&a == *(uint32_t *)&b; }

Move Move_encode(Square src, Square tgt, Piece_T Piece, Piece_T Capture,
                 Piece_T Promote, int dbl, int enpassant, int castle) {
    return (Move){
        .source = src,
        .target = tgt,
        .dbl = dbl,
        .enpassant = enpassant,
        .castle = castle,
        .capture = Capture != NULL,
        .promote = Promote != NULL,
        .piece = Piece_index(Piece),
        .piece_capture = Capture ? Piece_index(Capture) : 0,
        .piece_promote = Promote ? Piece_index(Promote) : 0,
    };
}

void Move_print(Move move) {
    printf("%5s %5s  %2s  %2s   %2s %4d %4d %4d %4d %4d\n",
           square_to_coordinates[Move_source(move)],
           square_to_coordinates[Move_target(move)],
           Piece_unicode(Move_piece(move)),
           Move_capture(move) ? Piece_unicode(Move_piece_capture(move)) : "X ",
           Move_promote(move) ? Piece_unicode(Move_piece_promote(move)) : "X ",
           Move_double(move) ? 1 : 0, Move_enpassant(move) ? 1 : 0,
           Move_castle(move) ? 1 : 0, Move_capture(move) ? 1 : 0,
           Move_promote(move) ? 1 : 0);
}

MoveList_T MoveList_new(void) {
    MoveList_T p;
    NEW0(p);
    return p;
}

void MoveList_free(MoveList_T *p) { FREE(*p); }

Move MoveList_move(MoveList_T self, int index) { return self->moves[index]; }
int MoveList_size(MoveList_T self) { return self->count; }
void MoveList_reset(MoveList_T self) { self->count = 0; }

void MoveList_add(MoveList_T self, Move move) {
    self->moves[self->count++] = move;
}

void MoveList_print(MoveList_T self) {
    printf(" From    To  Pi  Cap  Prmt  Dbl  Enp  Cst  C   P\n");
    for (int i = 0; i < self->count; i++)
        Move_print(self->moves[i]);
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
        move = Move_encode(source, target, Piece, Capture,                     \
                           Piece_get(i, color), 0, 0, 0);                      \
        MoveList_add(moves, move);                                             \
    }

MoveList_T MoveList_generate(MoveList_T moves, CBoard_T board) {
    Move move;
    Square src, tgt;
    eColor color = CBoard_side(board);

    if (!moves) moves = MoveList_new();

    { // pawn moves
        Piece_T Piece = Piece_get(PAWN, color);
        U64 bitboard = CBoard_pieceSet(board, Piece);
        bitboard_for_each_bit(src, bitboard) {
            { // quiet
                int add = (color == WHITE) ? +8 : -8;
                tgt = src + add;
                if (tgt > a1 && tgt < h8 &&
                    !CBoard_square_isOccupied(board, tgt)) {
                    if (pawn_canPromote(color, src)) {
                        pawn_promote(src, tgt, Piece, 0);
                    } else {
                        MoveList_add(
                            moves, Move_encode(src, tgt, Piece, 0, 0, 0, 0, 0));

                        // two ahead
                        if (pawn_onStart(color, src) &&
                            !CBoard_square_isOccupied(board, tgt += add))
                            MoveList_add(moves, Move_encode(src, tgt, Piece, 0,
                                                            0, 1, 0, 0));
                    }
                }
            }
            { // capture
                U64 attack = CBoard_piece_attacks(board, Piece, src) &
                             CBoard_colorBB(board, !color);
                bitboard_for_each_bit(tgt, attack) {
                    if (pawn_canPromote(color, src)) {
                        pawn_promote(src, tgt, Piece,
                                     CBoard_square_piece(board, tgt, !color));
                    } else {
                        MoveList_add(moves, Move_encode(src, tgt, Piece,
                                                        CBoard_square_piece(
                                                            board, tgt, !color),
                                                        0, 0, 0, 0));
                    }
                }
            }

            { // en passant
                if (CBoard_enpassant(board) != no_sq &&
                    CBoard_piece_attacks(board, Piece, src) &
                        (C64(1) << CBoard_enpassant(board)))
                    MoveList_add(
                        moves,
                        Move_encode(src, CBoard_enpassant(board), Piece,
                                    CBoard_square_piece(board, tgt, !color), 0,
                                    0, 1, 0));
            }
        }
    }

    // All piece move
    for (int piece = 1; piece < 6; piece++) {
        Piece_T Piece = Piece_get(piece, color);
        U64 bitboard = CBoard_pieceSet(board, Piece);
        bitboard_for_each_bit(src, bitboard) {
            U64 attack = CBoard_piece_attacks(board, Piece, src) &
                         ~CBoard_colorBB(board, color);
            bitboard_for_each_bit(tgt, attack) {
                /* int take = bit_get(CBoard_colorBB(board, !color), tgt); */
                MoveList_add(
                    moves, Move_encode(src, tgt, Piece,
                                       CBoard_square_piece(board, tgt, !color),
                                       0, 0, 0, 0));
            }
        }
    }

    // Castling
    {
        if (color == WHITE) {
            Piece_T Piece = Piece_get(KING, WHITE);
            if (CBoard_castle(board) & WK) {
                if (!CBoard_square_isOccupied(board, f1) &&
                    !CBoard_square_isOccupied(board, g1) &&
                    !CBoard_square_isAttack(board, e1, BLACK) &&
                    !CBoard_square_isAttack(board, f1, BLACK))
                    MoveList_add(moves,
                                 Move_encode(e1, g1, Piece, 0, 0, 0, 0, 1));
            }
            if (CBoard_castle(board) & WQ) {
                if (!CBoard_square_isOccupied(board, d1) &&
                    !CBoard_square_isOccupied(board, c1) &&
                    !CBoard_square_isOccupied(board, b1) &&
                    !CBoard_square_isAttack(board, e1, BLACK) &&
                    !CBoard_square_isAttack(board, d1, BLACK))
                    MoveList_add(moves,
                                 Move_encode(e1, c1, Piece, 0, 0, 0, 0, 1));
            }
        } else {
            Piece_T Piece = Piece_get(KING, BLACK);
            if (CBoard_castle(board) & BK) {
                if (!CBoard_square_isOccupied(board, f8) &&
                    !CBoard_square_isOccupied(board, g8) &&
                    !CBoard_square_isAttack(board, e8, WHITE) &&
                    !CBoard_square_isAttack(board, f8, WHITE))
                    MoveList_add(moves,
                                 Move_encode(e8, g8, Piece, 0, 0, 0, 0, 1));
            }
            if (CBoard_castle(board) & BQ) {
                if (!CBoard_square_isOccupied(board, d8) &&
                    !CBoard_square_isOccupied(board, c8) &&
                    !CBoard_square_isOccupied(board, b8) &&
                    !CBoard_square_isAttack(board, e8, WHITE) &&
                    !CBoard_square_isAttack(board, d8, WHITE))
                    MoveList_add(moves,
                                 Move_encode(e8, c8, Piece, 0, 0, 0, 0, 1));
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

int Move_make(Move move, CBoard_T board, int flag) {
    if (flag == 0) {

        Square source = Move_source(move);
        Square target = Move_target(move);
        Piece_T Piece = Move_piece(move);
        eColor color = CBoard_side(board);

        if (!Move_capture(move))
            CBoard_piece_move(board, Piece, source, target);
        else
            CBoard_piece_capture(board, Piece, Move_piece_capture(move), source,
                                 target);

        if (Move_promote(move)) {
            CBoard_piece_pop(board, Piece, target);
            CBoard_piece_set(board, Move_piece_promote(move), target);
        }

        {
            int ntarget = target + (color == WHITE ? -8 : +8);
            if (Move_enpassant(move))
                CBoard_piece_pop(board, Piece_get(PAWN, !color), ntarget);

            CBoard_enpassant_set(board, Move_double(move) ? ntarget : no_sq);
        }

        if (Move_castle(move)) {
            Piece_T Rook = Piece_get(ROOK, CBoard_side(board));
            switch (target) {
            case g1:
                CBoard_piece_move(board, Rook, h1, f1);
                break;
            case c1:
                CBoard_piece_move(board, Rook, a1, d1);
                break;
            case g8:
                CBoard_piece_move(board, Rook, h8, f8);
                break;
            case c8:
                CBoard_piece_move(board, Rook, a8, d8);
                break;
            default:
                break;
            }
        }

        CBoard_castle_and(board, castling_rights[source]);
        CBoard_castle_and(board, castling_rights[target]);

        if (!CBoard_isCheck(board)) {
            CBoard_side_switch(board);
            return 1;
        } else
            return 0;
    } else {
        if (Move_capture(move))
            return Move_make(move, board, 0);
        else
            return 0;
    }
}
