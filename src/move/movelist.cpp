#include "movelist.hpp"
#include "color.hpp"
#include "piece.hpp"
#include <iomanip>

#define pawn_canPromote(color, source)                                                                       \
    ((color == color::WHITE && source >= square::a7 && source <= square::h7) ||                              \
     (color == color::BLACK && source >= square::a2 && source <= square::h2))

#define pawn_onStart(color, source)                                                                          \
    ((color == color::BLACK && source >= square::a7 && source <= square::h7) ||                              \
     (color == color::WHITE && source >= square::a2 && source <= square::h2))

using piece::Type::PAWN;

void MoveList::generate(const Board &board, bool attacks_only) {
    uint8_t src_i = 0, tgt_i = 0;

    const color::Color color = board.get_side(), colorOther = color::other(color);

    // pawn moves
    const int add = (color == color::WHITE) ? +8 : -8;

    U64 bitboard = board.get_bitboard_piece(PAWN, color);
    bitboard_for_each_bit(src_i, bitboard) {
        const auto src = static_cast<square::Square>(src_i);
        const auto tgt = static_cast<square::Square>(tgt_i = src_i + add);
        if (!attacks_only && !board.is_square_occupied(tgt)) {
            if (pawn_canPromote(color, src)) {
                list.emplace_back(src, tgt, Move::PKNIGHT);
                list.emplace_back(src, tgt, Move::PBISHOP);
                list.emplace_back(src, tgt, Move::PROOK);
                list.emplace_back(src, tgt, Move::PQUEEN);
            } else {
                list.emplace_back(src, tgt, Move::PQUIET);

                // two ahead
                const auto tgt = static_cast<square::Square>(tgt_i + add);
                if (pawn_onStart(color, src) && !board.is_square_occupied(tgt))
                    list.emplace_back(src, tgt, Move::DOUBLE);
            }
        }

        // capture
        U64 attack =
            board.get_bitboard_piece_attacks(PAWN, color, src) & board.get_bitboard_color(colorOther);
        bitboard_for_each_bit(tgt_i, attack) {
            const auto tgt = static_cast<square::Square>(tgt_i);
            if (pawn_canPromote(color, src)) {
                list.emplace_back(src, tgt, Move::PCKNIGHT);
                list.emplace_back(src, tgt, Move::PCBISHOP);
                list.emplace_back(src, tgt, Move::PCROOK);
                list.emplace_back(src, tgt, Move::PCQUEEN);
            } else {
                list.emplace_back(src, tgt, Move::CAPTURE);
            }
        }

        // en passant
        const square::Square enpassant = board.get_enpassant();
        if (enpassant != square::no_sq && board.is_piece_attack_square(PAWN, color, src, enpassant))
            list.emplace_back(src, enpassant, Move::ENPASSANT);
    }

    // All piece move
    for (piece::Type type = piece::KNIGHT; type <= piece::KING; ++type) {
        U64 bitboard = board.get_bitboard_piece(type, color);
        bitboard_for_each_bit(src_i, bitboard) {
            const auto src = static_cast<square::Square>(src_i);
            U64 attack = board.get_bitboard_piece_moves(type, color, src);
            bitboard_for_each_bit(tgt_i, attack) {
                const auto tgt = static_cast<square::Square>(tgt_i);
                if (board.is_square_occupied(tgt)) {
                    list.emplace_back(src, tgt, Move::CAPTURE);
                } else {
                    if (attacks_only) continue;
                    list.emplace_back(src, tgt, Move::QUIET);
                }
            }
        }
    }

    if (attacks_only) return;

    // Castling
    if (color == color::WHITE) {
        if (!board.is_square_attacked(square::e1, color::BLACK)) {
            if (board.get_castle() & Board::Castle::WK) {
                if (!board.is_square_occupied(square::f1) && !board.is_square_occupied(square::g1) &&
                    !board.is_square_attacked(square::f1, color::BLACK))
                    list.emplace_back(square::e1, square::g1, Move::CASTLEK);
            }
            if (board.get_castle() & Board::Castle::WQ) {
                if (!board.is_square_occupied(square::d1) && !board.is_square_occupied(square::c1) &&
                    !board.is_square_occupied(square::b1) &&
                    !board.is_square_attacked(square::d1, color::BLACK) &&
                    !board.is_square_attacked(square::c1, color::BLACK))
                    list.emplace_back(square::e1, square::c1, Move::CASTLEQ);
            }
        }
    } else {
        if (!board.is_square_attacked(square::e8, color::WHITE)) {
            if (board.get_castle() & Board::Castle::BK) {
                if (!board.is_square_occupied(square::f8) && !board.is_square_occupied(square::g8) &&
                    !board.is_square_attacked(square::f8, color::WHITE))
                    list.emplace_back(square::Square::e8, square::Square::g8, Move::CASTLEK);
            }
            if (board.get_castle() & Board::Castle::BQ) {
                if (!board.is_square_occupied(square::d8) && !board.is_square_occupied(square::c8) &&
                    !board.is_square_occupied(square::b8) &&
                    !board.is_square_attacked(square::d8, color::WHITE) &&
                    !board.is_square_attacked(square::c8, color::WHITE))
                    list.emplace_back(square::e8, square::c8, Move::CASTLEQ);
            }
        }
    }
}

std::ostream &operator<<(std::ostream &os, const MoveList &list) {
    os << "Size: " << std::dec << list.size() << "\n";
    for (const Move move : list.list) {
        os << move << "\n";
    }
    return os;
}
