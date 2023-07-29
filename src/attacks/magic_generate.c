#include "internal.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *FORMAT = "C64(0x%llx),\n";

// pseudo random numbers

U32 state = C32(1804289383);

U32 get_random_U32_number() {
    U32 number = state;

    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    return state = number;
}

U64 get_random_U64_number() {
    U64 n1, n2, n3, n4;

    n1 = (U64)(get_random_U32_number()) & C64(0xFFFF);
    n2 = (U64)(get_random_U32_number()) & C64(0xFFFF);
    n3 = (U64)(get_random_U32_number()) & C64(0xFFFF);
    n4 = (U64)(get_random_U32_number()) & C64(0xFFFF);

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 generate_magic_number() {
    return get_random_U64_number() & get_random_U64_number() &
           get_random_U64_number();
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

    printf("Bishup Magic Numbers:\n");
    for (int i = 0; i < 64; i++)
        printf(FORMAT, find_magic_number(i, bishop_relevant_bits[i], 1));

    printf("Rook Magic Numbers:\n");
    for (int i = 0; i < 64; i++)
        printf(FORMAT, find_magic_number(i, rook_relevant_bits[i], 0));
    return 0;
}
