#include "string.h"

#include "attack.h"
#include "piece.h"

#define UNUSED(x) (void)(x)

// Magic constants

// clang-format off
const int bishop_relevant_bits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6,
};

const U64 bishop_magic_numbers[64] = {
    C64(0x40040844404084),   C64(0x2004208a004208),   C64(0x10190041080202),
    C64(0x108060845042010),  C64(0x581104180800210),  C64(0x2112080446200010),
    C64(0x1080820820060210), C64(0x3c0808410220200),  C64(0x4050404440404),
    C64(0x21001420088),      C64(0x24d0080801082102), C64(0x1020a0a020400),
    C64(0x40308200402),      C64(0x4011002100800),    C64(0x401484104104005),
    C64(0x801010402020200),  C64(0x400210c3880100),   C64(0x404022024108200),
    C64(0x810018200204102),  C64(0x4002801a02003),    C64(0x85040820080400),
    C64(0x810102c808880400), C64(0xe900410884800),    C64(0x8002020480840102),
    C64(0x220200865090201),  C64(0x2010100a02021202), C64(0x152048408022401),
    C64(0x20080002081110),   C64(0x4001001021004000), C64(0x800040400a011002),
    C64(0xe4004081011002),   C64(0x1c004001012080),   C64(0x8004200962a00220),
    C64(0x8422100208500202), C64(0x2000402200300c08), C64(0x8646020080080080),
    C64(0x80020a0200100808), C64(0x2010004880111000), C64(0x623000a080011400),
    C64(0x42008c0340209202), C64(0x209188240001000),  C64(0x400408a884001800),
    C64(0x110400a6080400),   C64(0x1840060a44020800), C64(0x90080104000041),
    C64(0x201011000808101),  C64(0x1a2208080504f080), C64(0x8012020600211212),
    C64(0x500861011240000),  C64(0x180806108200800),  C64(0x4000020e01040044),
    C64(0x300000261044000a), C64(0x802241102020002),  C64(0x20906061210001),
    C64(0x5a84841004010310), C64(0x4010801011c04),    C64(0xa010109502200),
    C64(0x4a02012000),       C64(0x500201010098b028), C64(0x8040002811040900),
    C64(0x28000010020204),   C64(0x6000020202d0240),  C64(0x8918844842082200),
    C64(0x4010011029020020),
};

const int rook_relevant_bits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12,
};

const U64 rook_magic_numbers[64] = {
    C64(0x8a80104000800020), C64(0x140002000100040),  C64(0x2801880a0017001),
    C64(0x100081001000420),  C64(0x200020010080420),  C64(0x3001c0002010008),
    C64(0x8480008002000100), C64(0x2080088004402900), C64(0x800098204000),
    C64(0x2024401000200040), C64(0x100802000801000),  C64(0x120800800801000),
    C64(0x208808088000400),  C64(0x2802200800400),    C64(0x2200800100020080),
    C64(0x801000060821100),  C64(0x80044006422000),   C64(0x100808020004000),
    C64(0x12108a0010204200), C64(0x140848010000802),  C64(0x481828014002800),
    C64(0x8094004002004100), C64(0x4010040010010802), C64(0x20008806104),
    C64(0x100400080208000),  C64(0x2040002120081000), C64(0x21200680100081),
    C64(0x20100080080080),   C64(0x2000a00200410),    C64(0x20080800400),
    C64(0x80088400100102),   C64(0x80004600042881),   C64(0x4040008040800020),
    C64(0x440003000200801),  C64(0x4200011004500),    C64(0x188020010100100),
    C64(0x14800401802800),   C64(0x2080040080800200), C64(0x124080204001001),
    C64(0x200046502000484),  C64(0x480400080088020),  C64(0x1000422010034000),
    C64(0x30200100110040),   C64(0x100021010009),     C64(0x2002080100110004),
    C64(0x202008004008002),  C64(0x20020004010100),   C64(0x2048440040820001),
    C64(0x101002200408200),  C64(0x40802000401080),   C64(0x4008142004410100),
    C64(0x2060820c0120200),  C64(0x1001004080100),    C64(0x20c020080040080),
    C64(0x2935610830022400), C64(0x44440041009200),   C64(0x280001040802101),
    C64(0x2100190040002085), C64(0x80c0084100102001), C64(0x4024081001000421),
    C64(0x20030a0244872),    C64(0x12001008414402),   C64(0x2006104900a0804),
    C64(0x1004081002402),
};


// Utility functions

int hash(U64 key, U64 magic, int relevant_bits) {
  return (key * magic) >> (64 - relevant_bits);
}

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

// pawn attack table [side][square]
U64 pawn_attacks[2][64];

// knight attack table [square]
U64 knight_attacks[64];

// king attack table [square]
U64 king_attacks[64];

// bishop attack mask
U64 bishop_masks[64];

// rook attack mask
U64 rook_masks[64];

// bishop attack table [square][occupancies]
U64 bishop_attacks[64][512]; // 256 K

// rook attack table [square][occupancies]
U64 rook_attacks[64][4096]; // 2048K

U64 get_wpawn_attacks(Square square, U64 occupancy){
  UNUSED(occupancy);
  return pawn_attacks[WHITE][square];
}

U64 get_bpawn_attacks(Square square, U64 occupancy){
  UNUSED(occupancy);
  return pawn_attacks[BLACK][square];
}

U64 get_knight_attacks(Square square, U64 occupancy){
  UNUSED(occupancy);
  return knight_attacks[square];
}

U64 get_king_attacks(Square square, U64 occupancy){
  UNUSED(occupancy);
  return king_attacks[square];
}

U64 get_bishop_attacks(Square square, U64 occupancy){
  occupancy &= bishop_masks[square];
  occupancy = hash(occupancy, bishop_magic_numbers[square],
                   bishop_relevant_bits[square]);
  return bishop_attacks[square][occupancy];
}

U64 get_rook_attacks(Square square, U64 occupancy){
  occupancy &= rook_masks[square];
  occupancy =
      hash(occupancy, rook_magic_numbers[square], rook_relevant_bits[square]);
  return rook_attacks[square][occupancy];
}

U64 get_queen_attacks(Square square, U64 occupancy){
  return (get_bishop_attacks(square, occupancy) |
          get_rook_attacks(square, occupancy));
}


// generate pawn attack
U64 mask_pawn_attacks(int side, Square square) {
  U64 bitboard = C64(0);

  bit_set(bitboard, square);
  if (side == WHITE)
    return noWeOne(bitboard) | noEaOne(bitboard);
  else
    return soWeOne(bitboard) | soEaOne(bitboard);
}

U64 mask_knight_attacks(Square square) {
  U64 bitboard = C64(0), attacks = C64(0), tmp;

  bit_set(bitboard, square);
  tmp = nortOne(nortOne(bitboard));
  attacks |= westOne(tmp) | eastOne(tmp);
  tmp = soutOne(soutOne(bitboard));
  attacks |= westOne(tmp) | eastOne(tmp);
  tmp = westOne(westOne(bitboard));
  attacks |= soutOne(tmp) | nortOne(tmp);
  tmp = eastOne(eastOne(bitboard));
  attacks |= soutOne(tmp) | nortOne(tmp);

  return attacks;
}

U64 mask_king_attacks(Square square) {
  U64 bitboard = C64(0), attacks = C64(0);

  bit_set(bitboard, square);
  attacks |= westOne(bitboard) | eastOne(bitboard);
  attacks |= soutOne(bitboard) | nortOne(bitboard);
  attacks |= soutOne(bitboard) | nortOne(bitboard);
  attacks |= soEaOne(bitboard) | noEaOne(bitboard);
  attacks |= soWeOne(bitboard) | noWeOne(bitboard);

  return attacks;
}

U64 mask_slide_attacks(Square square, U64 block, const direction_f dir[4],
                       int len[4]) {
  U64 bitboard = C64(0), attacks = C64(0), tmp;
  int i, j;

  bit_set(bitboard, square);
  for (i = 0; i < 4; i++) {
    for (j = 0, tmp = bitboard; j < len[i]; j++) {
      attacks |= tmp = (dir[i])(tmp);
      if (tmp & block)
        break;
    }
  }
  return attacks;
}

const direction_f bishop_direction[4] = {noEaOne, noWeOne, soEaOne, soWeOne};
const direction_f rook_direction[4] = {westOne, soutOne, eastOne, nortOne};

U64 mask_bishop_attacks(Square square) {
  int tr = square / 8, tf = square % 8;
  int len[4] = {MIN(7 - tf, 7 - tr) - 1, MIN(tf, 7 - tr) - 1,
                MIN(7 - tf, tr) - 1, MIN(tf, tr) - 1};
  return mask_slide_attacks(square, C64(0), bishop_direction, len);
}

U64 mask_rook_attacks(Square square) {
  int tr = square / 8, tf = square % 8;
  int len[4] = {tf - 1, tr - 1, 6 - tf, 6 - tr};

  return mask_slide_attacks(square, C64(0), rook_direction, len);
}

U64 bishop_attacks_on_the_fly(Square square, U64 block) {
  int tr = square / 8, tf = square % 8;
  int len[4] = {MIN(7 - tf, 7 - tr), MIN(tf, 7 - tr), MIN(7 - tf, tr),
                MIN(tf, tr)};

  return mask_slide_attacks(square, block, bishop_direction, len);
}

U64 rook_attacks_on_the_fly(Square square, U64 block) {
  int tr = square / 8, tf = square % 8;
  int len[4] = {tf, tr, 7 - tf, 7 - tr};

  return mask_slide_attacks(square, block, rook_direction, len);
}

void init_leapers_attacks(void) {
  for (Square square = 0; square < 64; square++) {
    pawn_attacks[WHITE][square] = mask_pawn_attacks(WHITE, square);
    pawn_attacks[BLACK][square] = mask_pawn_attacks(BLACK, square);
    knight_attacks[square] = mask_knight_attacks(square);
    king_attacks[square] = mask_king_attacks(square);
  }
}

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
  U64 occupancy = C64(0);

  for (int count = 0; count < bits_in_mask; count++) {
    Square square = bit_lsb_index(attack_mask);
    bit_pop(attack_mask, square);

    if (index & (1 << count))
      bit_set(occupancy, square);
  }

  return occupancy;
}

void init_sliders_attacks_internal(int bishop) {
  for (Square square = 0; square < 64; square++) {
    U64 attack_mask;

    if (bishop) {
      bishop_masks[square] = mask_bishop_attacks(square);
      attack_mask = bishop_masks[square];
    } else {
      rook_masks[square] = mask_rook_attacks(square);
      attack_mask = rook_masks[square];
    }

    int relevant_bits = bit_count(attack_mask);
    int occupancy_indicies = 1 << relevant_bits;

    for (int index = 0; index < occupancy_indicies; index++) {
      U64 occupancy = set_occupancy(index, relevant_bits, attack_mask);
      if (bishop) {
        int magic_index = (occupancy * bishop_magic_numbers[square]) >>
                          (64 - bishop_relevant_bits[square]);
        bishop_attacks[square][magic_index] =
            bishop_attacks_on_the_fly(square, occupancy);
      } else {
        int magic_index = hash(occupancy, rook_magic_numbers[square],
                               rook_relevant_bits[square]);
        rook_attacks[square][magic_index] =
            rook_attacks_on_the_fly(square, occupancy);
      }
    }
  }
}

void init_sliders_attacks(void) {
  init_sliders_attacks_internal(0);
  init_sliders_attacks_internal(1);
}

// magic numbers

U64 generate_magic_number() {
  return get_random_U64_number() & get_random_U64_number() &
         get_random_U64_number();
}

U64 find_magic_number(Square square, int relevant_bits, int bishop) {
  U64 occupancies[4096], attacks[4096], used_attacks[4096];
  U64 attack_mask =
      bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
  int occupancy_indicies = 1 << relevant_bits;

  for (int index = 0; index < occupancy_indicies; index++) {
    occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);
    attacks[index] = bishop
                         ? bishop_attacks_on_the_fly(square, occupancies[index])
                         : rook_attacks_on_the_fly(square, occupancies[index]);
  }

  for (int random_count = 0; random_count < 100000000; random_count++) {
    U64 magic_number = generate_magic_number();
    if (bit_count((attack_mask * magic_number) & C64(0xFF00000000000000)) < 6)
      continue;

    memset(used_attacks, C64(0), sizeof(used_attacks));
    int index, fail;

    for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++) {
      int magic_index = hash(occupancies[index], magic_number, relevant_bits);

      if (used_attacks[magic_index] == C64(0))
        used_attacks[magic_index] = attacks[index];
      else if (used_attacks[magic_index] != attacks[index])
        fail = 1;
    }

    if (!fail)
      return magic_number;
  }

  return C64(0);
}

void init_attacks(void) {
    init_leapers_attacks();
    init_sliders_attacks();
}
