#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <cul/assert.h>
#include <cul/mem.h>

#include "board.h"

struct Board {
    U64 colorBB[2];
    U64 pieceBB[6];
    eColor side;
    Square enpassant;
    eCastle castle;
};

Board board_new(void) {
    Board p;
    NEW0(p);
    return p;
}

void board_free(Board *p) { FREE(*p); }

void board_copy(Board self, Board dest) { *dest = *self; }

Square board_enpassant(Board self) { return self->enpassant; }
eCastle board_castle(Board self) { return self->castle; }
eColor board_side(Board self) { return self->side; }
U64 board_colorBB(Board self, eColor color) { return self->colorBB[color]; }
U64 board_pieceBB(Board self, ePiece piece) { return self->pieceBB[piece]; }
U64 board_occupancy(Board self) {
    return self->colorBB[WHITE] | self->colorBB[BLACK];
}

U64 board_pieceBB_get(Board self, ePiece piece, Square target) {
    return bit_get(self->pieceBB[piece], target);
}

U64 board_pieceSet(Board self, Piece piece) {
    return self->pieceBB[piece_piece(piece)] &
           self->colorBB[piece_color(piece)];
}

void board_enpassant_set(Board self, Square target) {
    self->enpassant = target;
}

void board_colorBB_pop(Board self, eColor color, Square target) {
    bit_pop(self->colorBB[color], target);
}
void board_colorBB_set(Board self, eColor color, Square target) {
    bit_set(self->colorBB[color], target);
}
U64 board_colorBB_get(Board self, eColor color, Square target) {
    return bit_get(self->colorBB[color], target);
}

int board_piece_get(Board self, Square square) {
    for (int i = 0; i < 6; i++)
        if (bit_get(self->pieceBB[i], square)) return i;
    return -1;
}

void board_piece_pop(Board self, Piece piece, Square square) {
    bit_pop(self->pieceBB[piece_piece(piece)], square);
    bit_pop(self->colorBB[piece_color(piece)], square);
}

void board_piece_set(Board self, Piece piece, Square square) {
    bit_set(self->pieceBB[piece_piece(piece)], square);
    bit_set(self->colorBB[piece_color(piece)], square);
}

void board_piece_move(Board self, Piece Piece, Square source, Square target) {
    board_piece_pop(self, Piece, source);
    board_piece_set(self, Piece, target);
}

U64 board_piece_attacks(Board self, Piece piece, Square src) {
    return piece_attacks(piece)(src, board_occupancy(self));
}

void board_piece_capture(Board self, Piece piece, Piece taken, Square source,
                         Square target) {
    board_piece_pop(self, piece, source);
    if (taken) board_piece_pop(self, taken, target);
    board_piece_set(self, piece, target);
}

void board_castle_pop(Board self, eCastle castle) {
    bit_pop(self->castle, bit_lsb_index(castle));
}

void board_castle_and(Board self, int exp) { self->castle &= exp; }
void board_side_switch(Board self) { self->side = !self->side; }

int board_isCheck(Board self) {
    U64 king = self->pieceBB[KING] & self->colorBB[self->side];
    return board_square_isAttack(self, bit_lsb_index(king), !self->side);
}
int board_square_isOccupied(Board self, Square square) {
    return bit_get(board_occupancy(self), square);
}

int board_square_isAttack(Board self, Square square, eColor side) {
    U64 occupancy = self->colorBB[WHITE] | self->colorBB[BLACK];

    for (int i = 0; i < 6; i++) {
        if (piece_attacks(piece_get(i, !side))(square, occupancy) &
            self->pieceBB[i] & self->colorBB[side])
            return 1;
    }

    return 0;
}

Piece board_square_piece(Board self, Square square, eColor color) {
    for (ePiece i = 0; i < 6; i++)
        if (board_pieceBB_get(self, i, square)) return piece_get(i, color);
    return NULL;
}

void board_print(Board self) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            Square square = (7 - rank) * 8 + file;
            Piece piece = NULL;

            int color = -1;
            if (bit_get(self->colorBB[WHITE], square))
                color = WHITE;
            else if (bit_get(self->colorBB[BLACK], square))
                color = BLACK;

            if (color != -1) {
                for (int piece_index = 0; piece_index < 6; piece_index++) {
                    if (bit_get(self->pieceBB[piece_index], square)) {
                        piece = piece_get(piece_index, color);
                        break;
                    }
                }
            }

            if (!file) printf(" %d  ", 8 - rank);

            printf("%s", (piece) ? piece_unicode(piece) : ". ");
        }
        printf("\n");
    }
    printf("    A B C D E F G H\n");
    printf("     Side: %s\n", (self->side == WHITE) ? "white" : "black");
    printf("Enpassant: %s\n", square_to_coordinates[self->enpassant]);
    printf(" Castling: %c%c%c%c\n", (self->castle & WK) ? 'K' : '-',
           (self->castle & WQ) ? 'Q' : '-', (self->castle & BK) ? 'k' : '-',
           (self->castle & BQ) ? 'q' : '-');
    printf("\n");
}

Board board_from_FEN(Board board, const char *fen) {
    if (!board) NEW(board);

    memset(board, 0, sizeof(*board));

    board->side = -1;
    board->enpassant = no_sq;
    board->castle = 0;

    int file = 0, rank = 7;
    for (Piece piece; *fen != ' '; fen++) {
        Square square = rank * 8 + file;
        if (isalpha(*fen)) {
            if (!(piece = piece_from_code(*fen))) assert(0);
            bit_set(board->colorBB[piece_color(piece)], square);
            bit_set(board->pieceBB[piece_piece(piece)], square);
            file++;
        } else if (isdigit(*fen)) {
            file += *fen - '0';
        } else if (*fen == '/') {
            file = 0;
            rank--;
        } else
            assert(0);
    }

    fen++;
    if (*fen == 'w')
        board->side = WHITE;
    else if (*fen == 'b')
        board->side = BLACK;
    else
        assert(0);

    for (fen += 2; *fen != ' '; fen++) {
        switch (*fen) {
        case 'K':
            board->castle |= WK;
            break;
        case 'Q':
            board->castle |= WQ;
            break;
        case 'k':
            board->castle |= BK;
            break;
        case 'q':
            board->castle |= BQ;
            break;
        case '-':
            break;
        default:
            assert(0);
        }
    }

    fen++;
    if (*fen != '-') {
        board->enpassant = coordinates_to_square(fen);
    }

    return board;
}
