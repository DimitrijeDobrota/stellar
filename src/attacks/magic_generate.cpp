#include "internal.hpp"
#include "random.hpp"
#include "utils_cpp.hpp"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *FORMAT = "C64(0x%llx),\n";

U64 generate_magic_number() {
    return random_get_U64() & random_get_U64() & random_get_U64();
}

U64 find_magic_number(Square square, int relevant_bits, int bishop) {
    U64 occupancies[4096], attacks[4096], used_attacks[4096];
    U64 attack_mask = bishop ? bishop_mask(square) : rook_mask(square);
    int occupancy_indicies = 1 << relevant_bits;

    for (int index = 0; index < occupancy_indicies; index++) {
        occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);
        attacks[index] = bishop ? bishop_on_the_fly(square, occupancies[index])
                                : rook_on_the_fly(square, occupancies[index]);
    }

    for (int random_count = 0; random_count < 100000000; random_count++) {
        U64 magic_number = generate_magic_number();
        if (bit_count((attack_mask * magic_number) & C64(0xFF00000000000000)) <
            6)
            continue;

        memset(used_attacks, C64(0), sizeof(used_attacks));
        int index, fail;

        for (index = 0, fail = 0; !fail && index < occupancy_indicies;
             index++) {
            int magic_index =
                hash(occupancies[index], magic_number, relevant_bits);

            if (used_attacks[magic_index] == C64(0))
                used_attacks[magic_index] = attacks[index];
            else if (used_attacks[magic_index] != attacks[index])
                fail = 1;
        }

        if (!fail) return magic_number;
    }

    return C64(0);
}

int main(void) {
    random_state_reset();

    printf("Bishup Magic Numbers:\n");
    for (Square square : SquareIter())
        printf(FORMAT,
               find_magic_number(
                   square, bishop_relevant_bits[to_underlying(square)], 1));

    printf("Rook Magic Numbers:\n");
    for (Square square : SquareIter())
        printf(FORMAT,
               find_magic_number(square,
                                 rook_relevant_bits[to_underlying(square)], 0));
    return 0;
}
