#ifndef STELLAR_ATTAKCS_INTERNAL_H
#define STELLAR_ATTAKCS_INTERNAL_H

#include "utils_cpp.hpp"

extern U64 king_attacks[64];        // king attack table [square]
extern U64 knight_attacks[64];      // knight attack table [square]
extern U64 pawn_attacks[2][64];     // pawn attack table [side][square]
extern U64 rook_attacks[64][4096];  // rook attack table [square][occupancies]
extern U64 bishop_attacks[64][512]; // bishop attack table [square][occupancies]

extern U64 rook_masks[64];   // rook attack mask
extern U64 bishop_masks[64]; // bishop attack mask

extern const int rook_relevant_bits[64];
extern const int bishop_relevant_bits[64];

int hash(U64 key, U64 magic, int relevant_bits);

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);

U64 bishop_mask(Square square);
U64 bishop_on_the_fly(Square square, U64 block);
U64 king_mask(Square square);
U64 knight_mask(Square square);
U64 pawn_mask(Color side, Square square);
U64 rook_mask(Square square);
U64 rook_on_the_fly(Square square, U64 block);

#endif
