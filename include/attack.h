#ifndef ATTACK_H
#define ATTACK_H

#include "utils.h"

void init_leapers_attacks(void);
void init_sliders_attacks(void);

U64 get_wpawn_attacks(Square square, U64 occupancy);
U64 get_bpawn_attacks(Square square, U64 occupancy);
U64 get_knight_attacks(Square square, U64 occupancy);
U64 get_king_attacks(Square square, U64 occupancy);
U64 get_bishop_attacks(Square square, U64 occupancy);
U64 get_rook_attacks(Square square, U64 occupancy);
U64 get_queen_attacks(Square square, U64 occupancy);

#endif
