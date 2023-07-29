#ifndef BOARD_H
#define BOARD_H

#include "piece.h"

enum enumCastle {
    WK = 1,
    WQ = 2,
    BK = 4,
    BQ = 8
};
typedef enum enumCastle eCastle;

typedef struct Board *Board;

Board board_new(void);
void board_free(Board *p);
void board_copy(Board self, Board dest);

U64 board_colorBB(Board self, eColor color);
U64 board_occupancy(Board self);
U64 board_pieceBB(Board self, ePiece piece);
eCastle board_castle(Board self);
eColor board_side(Board self);

Square board_enpassant(Board self);
void board_enpassant_set(Board self, Square target);

U64 board_pieceSet(Board self, Piece piece);
U64 board_piece_attacks(Board self, Piece piece, Square src);
void board_piece_capture(Board self, Piece piece, Piece taken, Square source,
                         Square target);
void board_piece_move(Board self, Piece Piece, Square square, Square target);
void board_piece_pop(Board self, Piece Piece, Square square);
void board_piece_set(Board self, Piece Piece, Square square);
int board_piece_get(Board self, Square square);

U64 board_colorBB_get(Board self, eColor color, Square target);
void board_colorBB_pop(Board self, eColor color, Square target);
void board_colorBB_set(Board self, eColor color, Square target);

void board_castle_and(Board self, int exp);
void board_castle_pop(Board self, eCastle castle);

Piece board_square_piece(Board self, Square square, eColor side);
int board_square_isAttack(Board self, Square square, eColor side);
int board_square_isOccupied(Board self, Square square);

Board board_from_FEN(Board board, char *fen);
int board_isCheck(Board self);
void board_print(Board self);
void board_side_switch(Board self);

#endif
