#ifndef STELLAR_ATTAKCS_INTERNAL_H
#define STELLAR_ATTAKCS_INTERNAL_H

#include "utils.hpp"

#include <array>

namespace attack {

inline constexpr const U64 bishop_magic_numbers[64] = {
    C64(0x40040844404084),   C64(0x2004208a004208),   C64(0x10190041080202),   C64(0x108060845042010),
    C64(0x581104180800210),  C64(0x2112080446200010), C64(0x1080820820060210), C64(0x3c0808410220200),
    C64(0x4050404440404),    C64(0x21001420088),      C64(0x24d0080801082102), C64(0x1020a0a020400),
    C64(0x40308200402),      C64(0x4011002100800),    C64(0x401484104104005),  C64(0x801010402020200),
    C64(0x400210c3880100),   C64(0x404022024108200),  C64(0x810018200204102),  C64(0x4002801a02003),
    C64(0x85040820080400),   C64(0x810102c808880400), C64(0xe900410884800),    C64(0x8002020480840102),
    C64(0x220200865090201),  C64(0x2010100a02021202), C64(0x152048408022401),  C64(0x20080002081110),
    C64(0x4001001021004000), C64(0x800040400a011002), C64(0xe4004081011002),   C64(0x1c004001012080),
    C64(0x8004200962a00220), C64(0x8422100208500202), C64(0x2000402200300c08), C64(0x8646020080080080),
    C64(0x80020a0200100808), C64(0x2010004880111000), C64(0x623000a080011400), C64(0x42008c0340209202),
    C64(0x209188240001000),  C64(0x400408a884001800), C64(0x110400a6080400),   C64(0x1840060a44020800),
    C64(0x90080104000041),   C64(0x201011000808101),  C64(0x1a2208080504f080), C64(0x8012020600211212),
    C64(0x500861011240000),  C64(0x180806108200800),  C64(0x4000020e01040044), C64(0x300000261044000a),
    C64(0x802241102020002),  C64(0x20906061210001),   C64(0x5a84841004010310), C64(0x4010801011c04),
    C64(0xa010109502200),    C64(0x4a02012000),       C64(0x500201010098b028), C64(0x8040002811040900),
    C64(0x28000010020204),   C64(0x6000020202d0240),  C64(0x8918844842082200), C64(0x4010011029020020),
};

inline constexpr const U64 rook_magic_numbers[64] = {
    C64(0x8a80104000800020), C64(0x140002000100040),  C64(0x2801880a0017001),  C64(0x100081001000420),
    C64(0x200020010080420),  C64(0x3001c0002010008),  C64(0x8480008002000100), C64(0x2080088004402900),
    C64(0x800098204000),     C64(0x2024401000200040), C64(0x100802000801000),  C64(0x120800800801000),
    C64(0x208808088000400),  C64(0x2802200800400),    C64(0x2200800100020080), C64(0x801000060821100),
    C64(0x80044006422000),   C64(0x100808020004000),  C64(0x12108a0010204200), C64(0x140848010000802),
    C64(0x481828014002800),  C64(0x8094004002004100), C64(0x4010040010010802), C64(0x20008806104),
    C64(0x100400080208000),  C64(0x2040002120081000), C64(0x21200680100081),   C64(0x20100080080080),
    C64(0x2000a00200410),    C64(0x20080800400),      C64(0x80088400100102),   C64(0x80004600042881),
    C64(0x4040008040800020), C64(0x440003000200801),  C64(0x4200011004500),    C64(0x188020010100100),
    C64(0x14800401802800),   C64(0x2080040080800200), C64(0x124080204001001),  C64(0x200046502000484),
    C64(0x480400080088020),  C64(0x1000422010034000), C64(0x30200100110040),   C64(0x100021010009),
    C64(0x2002080100110004), C64(0x202008004008002),  C64(0x20020004010100),   C64(0x2048440040820001),
    C64(0x101002200408200),  C64(0x40802000401080),   C64(0x4008142004410100), C64(0x2060820c0120200),
    C64(0x1001004080100),    C64(0x20c020080040080),  C64(0x2935610830022400), C64(0x44440041009200),
    C64(0x280001040802101),  C64(0x2100190040002085), C64(0x80c0084100102001), C64(0x4024081001000421),
    C64(0x20030a0244872),    C64(0x12001008414402),   C64(0x2006104900a0804),  C64(0x1004081002402),
};

class Slider {
  public:
    virtual constexpr U32 hash(U64 key, Square square) const = 0;
    virtual constexpr U64 mask(Square square) const = 0;
    virtual constexpr U64 mask_fly(Square square, U64 block) const = 0;

    static inline constexpr U64 occupancy(U64 index, uint8_t bits_in_mask, U64 attack_mask) {
        U64 occupancy = C64(0);

        for (uint8_t count = 0; count < bits_in_mask; count++) {
            uint8_t square = bit_lsb_index(attack_mask);
            bit_pop(attack_mask, square);

            if (bit_get(index, count)) bit_set(occupancy, square);
        }

        return occupancy;
    }

  protected:
    static inline constexpr U64 mask_slide(Square square, U64 block, const direction_f dir[4],
                                           const int len[4]) {
        U64 bitboard = C64(0), attacks = C64(0);
        bit_set(bitboard, to_underlying(square));
        for (int i = 0; i < 4; i++) {
            U64 tmp = bitboard;
            for (int j = 0; j < len[i]; j++) {
                attacks |= tmp = (dir[i])(tmp);
                if (tmp & block) break;
            }
        }
        return attacks;
    }
};

class SliderRook : public Slider {
  public:
    virtual constexpr U32 hash(U64 key, Square square) const override {
        uint8_t square_i = to_underlying(square);
        return (key * rook_magic_numbers[square_i]) >> (64 - relevant_bits[square_i]);
    }

    virtual constexpr U64 mask(Square square) const override { return masks[to_underlying(square)]; }

    virtual constexpr U64 mask_fly(Square square, U64 block) const override {
        uint8_t square_i = to_underlying(square);
        int tr = square_i / 8, tf = square_i % 8;
        int len[4] = {tf, tr, 7 - tf, 7 - tr};

        return mask_slide(square, block, dir, len);
    }

  private:
    static inline constexpr const direction_f dir[4] = {westOne, soutOne, eastOne, nortOne};

    // clang-format off
    static inline constexpr const int relevant_bits[64] = {
      12, 11, 11, 11, 11, 11, 11, 12,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      12, 11, 11, 11, 11, 11, 11, 12,
    };
    // clang-format on

    static inline constexpr const std::array<U64, 64> masks = []() constexpr -> std::array<U64, 64> {
        std::array<U64, 64> masks;
        for (const Square square : SquareIter()) {
            const uint8_t square_i = to_underlying(square);
            const int tr = square_i / 8, tf = square_i % 8;
            const int len[4] = {tf - 1, tr - 1, 6 - tf, 6 - tr};

            masks[square_i] = mask_slide(square, C64(0), dir, len);
        }
        return masks;
    }();
};

class SliderBishop : public Slider {
  public:
    virtual constexpr U32 hash(U64 key, Square square) const override {
        uint8_t square_i = to_underlying(square);
        return (key * bishop_magic_numbers[square_i]) >> (64 - relevant_bits[square_i]);
    }

    virtual constexpr U64 mask(Square square) const override { return masks[to_underlying(square)]; }

    virtual constexpr U64 mask_fly(Square square, U64 block) const override {
        uint8_t square_i = to_underlying(square);
        int tr = square_i / 8, tf = square_i % 8;
        int len[4] = {std::min(7 - tf, 7 - tr), std::min(tf, 7 - tr), std::min(7 - tf, tr), std::min(tf, tr)};

        return mask_slide(square, block, dir, len);
    }

  private:
    static inline constexpr const direction_f dir[4] = {noEaOne, noWeOne, soEaOne, soWeOne};

    // clang-format off
    static inline constexpr const int relevant_bits[64] = {
      6, 5, 5, 5, 5, 5, 5, 6,
      5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 7, 7, 7, 7, 5, 5,
      5, 5, 7, 9, 9, 7, 5, 5,
      5, 5, 7, 9, 9, 7, 5, 5,
      5, 5, 7, 7, 7, 7, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5,
      6, 5, 5, 5, 5, 5, 5, 6,
    };
    // clang-format on

    static inline constexpr const std::array<U64, 64> masks = []() constexpr -> std::array<U64, 64> {
        std::array<U64, 64> masks;
        for (const Square square : SquareIter()) {

            uint8_t square_i = to_underlying(square);
            int tr = square_i / 8, tf = square_i % 8;
            int len[4] = {std::min(7 - tf, 7 - tr) - 1, std::min(tf, 7 - tr) - 1, std::min(7 - tf, tr) - 1,
                          std::min(tf, tr) - 1};
            masks[square_i] = mask_slide(square, C64(0), dir, len);
        }
        return masks;
    }();
};

inline constexpr const SliderRook slider_rook;
inline constexpr const SliderBishop slider_bishop;

class Attack {
  public:
    virtual constexpr U64 operator()(Square square, U64 occupancy) const = 0;

  protected:
    template <std::size_t size> using slider_attack_array = std::array<std::array<U64, size>, 64>;

    static inline constexpr const auto slider_attacks =
        []<std::size_t size>(const Slider &slider) constexpr -> slider_attack_array<size> {
        slider_attack_array<size> attacks;
        for (const Square square : SquareIter()) {
            uint8_t square_i = to_underlying(square);
            U64 attack_mask = slider.mask(square);
            uint8_t relevant_bits = bit_count(attack_mask);
            U64 occupancy_indices = C64(1) << relevant_bits;

            for (U64 idx = 0; idx < occupancy_indices; idx++) {
                U64 occupancy = Slider::occupancy(idx, relevant_bits, attack_mask);
                U32 magic_index = slider.hash(occupancy, square);
                attacks[square_i][magic_index] = slider.mask_fly(square, occupancy);
            }
        }
        return attacks;
    };
};

class Rook : public Attack {
  public:
    virtual constexpr U64 operator()(Square square, U64 occupancy) const override {
        occupancy &= slider_rook.mask(square);
        occupancy = slider_rook.hash(occupancy, square);
        return attacks[to_underlying(square)][occupancy];
    }

  private:
    static inline constexpr const slider_attack_array<4096> attacks =
        slider_attacks.operator()<4096>(slider_rook);
};

class Bishop : public Attack {
  public:
    virtual constexpr U64 operator()(Square square, U64 occupancy) const override {
        occupancy &= slider_bishop.mask(square);
        occupancy = slider_bishop.hash(occupancy, square);
        return attacks[to_underlying(square)][occupancy];
    }

  private:
    static inline constexpr const slider_attack_array<512> attacks =
        slider_attacks.operator()<512>(slider_bishop);
};

class King : public Attack {
  public:
    constexpr King() {}

    virtual constexpr U64 operator()(Square square, U64 occupancy) const override {
        return attacks[to_underlying(square)];
    }

  private:
    static constexpr U64 mask(Square square) {
        U64 bitboard = C64(0), attacks = C64(0);

        bit_set(bitboard, to_underlying(square));
        attacks |= westOne(bitboard) | eastOne(bitboard);
        attacks |= soutOne(bitboard) | nortOne(bitboard);
        attacks |= soutOne(bitboard) | nortOne(bitboard);
        attacks |= soEaOne(bitboard) | noEaOne(bitboard);
        attacks |= soWeOne(bitboard) | noWeOne(bitboard);

        return attacks;
    }

    typedef std::array<U64, 64> attack_array;
    const attack_array attacks = []() -> attack_array {
        std::array<U64, 64> attacks;
        for (const Square square : SquareIter())
            attacks[to_underlying(square)] = mask(square);
        return attacks;
    }();
};

class Knight : public Attack {
  public:
    constexpr Knight() {}

    virtual constexpr U64 operator()(Square square, U64 occupancy) const override {
        return attacks[to_underlying(square)];
    }

  private:
    static constexpr U64 mask(Square square) {
        U64 bitboard = C64(0), attacks = C64(0), tmp;

        bit_set(bitboard, to_underlying(square));
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

    typedef std::array<U64, 64> attack_array;
    const attack_array attacks = []() -> attack_array {
        std::array<U64, 64> attacks;
        for (const Square square : SquareIter())
            attacks[to_underlying(square)] = mask(square);
        return attacks;
    }();
};

class PawnW : public Attack {
  public:
    constexpr PawnW() {}

    virtual constexpr U64 operator()(Square square, U64 occupancy) const override {
        return attacks[to_underlying(square)];
    }

  private:
    static constexpr U64 mask(Square square) {
        U64 bitboard = C64(0);

        bit_set(bitboard, to_underlying(square));
        return noWeOne(bitboard) | noEaOne(bitboard);
    }

    typedef std::array<U64, 64> attack_array;
    const attack_array attacks = []() -> attack_array {
        std::array<U64, 64> attacks;
        for (const Square square : SquareIter())
            attacks[to_underlying(square)] = mask(square);
        return attacks;
    }();
};

class PawnB : public Attack {
  public:
    constexpr PawnB() {}

    virtual constexpr U64 operator()(Square square, U64 occupancy) const override {
        return attacks[to_underlying(square)];
    }

  private:
    static constexpr U64 mask(Square square) {
        U64 bitboard = C64(0);

        bit_set(bitboard, to_underlying(square));
        return soWeOne(bitboard) | soEaOne(bitboard);
    }

    typedef std::array<U64, 64> attack_array;
    const attack_array attacks = []() -> attack_array {
        std::array<U64, 64> attacks;
        for (const Square square : SquareIter())
            attacks[to_underlying(square)] = mask(square);
        return attacks;
    }();
};

inline constexpr const Rook rook;
inline constexpr const Bishop bishop;
inline constexpr const King king;
inline constexpr const Knight knight;
inline constexpr const PawnW pawnW;
inline constexpr const PawnB pawnB;

class Queen : public Attack {
  public:
    constexpr Queen() {}

    virtual constexpr U64 operator()(Square square, U64 occupancy) const override {
        return rook(square, occupancy) | bishop(square, occupancy);
    }
};

inline constexpr const Queen queen;

} // namespace attack
#endif
