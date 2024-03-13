#include "bitboard.hpp"
#include "bit.hpp"
#include "piece.hpp"

#include <iostream>

namespace bitboard {

/*
void init() {
for (Square s1 = Square::a1; s1 <= Square::h8; ++s1) {
    for (Square s2 = Square::a1; s2 <= Square::h8; ++s2) {
        line[s1][s2] |=
            (piece::get_attack(piece::BISHOP, s1, 0) & piece::get_attack(piece::BISHOP, s2, 0)) | s1 | s2;
    }
}
}
*/

void print(U64 bitboard) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            uint8_t square = (7 - rank) * 8 + file;
            if (!file) printf(" %d  ", 8 - rank);
            std::cout << bit::get(bitboard, square) << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n    A B C D E F G H\n\n";
    std::cout << "    Bitboard: " << std::hex << bitboard << std::dec << std::endl;
}

} // namespace bitboard
