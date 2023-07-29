#ifndef STELLAR_PIECE_H
#define STELLAR_PIECE_H

#include "attack.h"

typedef const struct Piece *Piece;

typedef enum enumColor eColor;
enum enumColor {
    WHITE = 0,
    BLACK
};

typedef enum enumPiece ePiece;
enum enumPiece {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

char piece_asci(Piece self);
attack_f piece_attacks(Piece self);
char piece_code(Piece self);
char *piece_unicode(Piece self);
eColor piece_color(Piece self);
ePiece piece_piece(Piece self);
int piece_index(Piece self);

Piece piece_get(ePiece piece, eColor color);
Piece piece_from_code(char code);
Piece piece_from_index(int index);

#endif
