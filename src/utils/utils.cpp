#include "utils.hpp"
#include <iostream>

void bitboard_print(U64 bitboard) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            uint8_t square = (7 - rank) * 8 + file;
            if (!file) printf(" %d  ", 8 - rank);
            std::cout << bit_get(bitboard, square) << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n    A B C D E F G H\n\n";
    std::cout << "    Bitboard: " << std::hex << bitboard << std::dec;
}
