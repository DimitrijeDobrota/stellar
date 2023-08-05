#ifndef STELLAR_BOARD_H
#define STELLAR_BOARD_H

#include "piece.h"

enum enumCastle {
    WK = 1,
    WQ = 2,
    BK = 4,
    BQ = 8
};
typedef enum enumCastle eCastle;

typedef struct Board Board;
struct Board {
    U64 color[2];
    U64 piece[6];
    eColor side;
    Square enpassant;
    eCastle castle;
};

Board *board_new(void);
void board_free(Board **p);
void board_copy(const Board *self, Board *dest);

U64 board_color(const Board *self, eColor color);
U64 board_occupancy(const Board *self);
U64 board_piece(const Board *self, ePiece piece);
eCastle board_castle(const Board *self);
eColor board_side(const Board *self);
Square board_enpassant(const Board *self);

void board_enpassant_set(Board *self, Square target);

U64 board_pieceSet(const Board *self, Piece piece);
U64 board_piece_attacks(const Board *self, Piece piece, Square src);
void board_piece_capture(Board *self, Piece piece, Piece taken, Square source,
                         Square target);

void board_piece_move(Board *self, Piece Piece, Square square, Square target);
void board_piece_pop(Board *self, Piece Piece, Square square);
void board_piece_set(Board *self, Piece Piece, Square square);
int board_piece_get(const Board *self, Square square);

U64 board_color_get(const Board *self, eColor color, Square target);
void board_color_pop(Board *self, eColor color, Square target);
void board_color_set(Board *self, eColor color, Square target);

void board_castle_and(Board *self, int exp);
void board_castle_pop(Board *self, eCastle castle);

Piece board_square_piece(const Board *self, Square square, eColor side);
int board_square_isAttack(const Board *self, Square square, eColor side);
int board_square_isOccupied(const Board *self, Square square);

Board *board_from_FEN(Board *board, const char *fen);
int board_isCheck(const Board *self);
void board_print(const Board *self);
void board_side_switch(Board *self);

#endif
