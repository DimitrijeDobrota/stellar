#ifndef STELLAR_ZOBRIST_H
#define STELLAR_ZOBRIST_H

#include "board.h"

void zobrist_init(void);
U64 zobrist_hash(const Board *board);

U64 zobrist_key_side(void);
U64 zobrist_key_castle(int exp);
U64 zobrist_key_enpassant(Square square);
U64 zobrist_key_piece(Piece piece, Square square);

#endif
