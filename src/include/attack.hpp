#ifndef STELLAR_ATTAKCS_INTERNAL_H
#define STELLAR_ATTAKCS_INTERNAL_H

#include "magic.hpp"
#include "utils_cpp.hpp"

#include <array>

namespace attack {

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

    virtual constexpr U64 mask(Square square) const override {
        return masks[to_underlying(square)];
    }

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

    virtual constexpr U64 mask(Square square) const override {
        return masks[to_underlying(square)];
    }

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
    constexpr King() {
    }

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
    constexpr Knight() {
    }

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
    constexpr PawnW() {
    }

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
    constexpr PawnB() {
    }

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
    constexpr Queen() {
    }

    virtual constexpr U64 operator()(Square square, U64 occupancy) const override {
        return rook(square, occupancy) | bishop(square, occupancy);
    }
};

inline constexpr const Queen queen;

} // namespace attack
#endif
