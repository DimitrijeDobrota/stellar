#ifndef STELLAR_ATTACKS_H
#define STELLAR_ATTACKS_H

#include "utils.h"

void attacks_init(void);

U64 attacks_wpawn_get(Square square, U64 occupancy);
U64 attacks_bpawn_get(Square square, U64 occupancy);
U64 attacks_knight_get(Square square, U64 occupancy);
U64 attakcs_king_get(Square square, U64 occupancy);
U64 attacks_bishop_get(Square square, U64 occupancy);
U64 attacks_rook_get(Square square, U64 occupancy);
U64 attacks_queen_get(Square square, U64 occupancy);

#endif
