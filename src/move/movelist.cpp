#include "movelist.hpp"
#include "piece.hpp"
#include <iomanip>

#define pawn_canPromote(color, source)                                                                       \
    ((color == WHITE && source >= Square::a7 && source <= Square::h7) ||                                     \
     (color == BLACK && source >= Square::a2 && source <= Square::h2))

#define pawn_onStart(color, source)                                                                          \
    ((color == BLACK && source >= Square::a7 && source <= Square::h7) ||                                     \
     (color == WHITE && source >= Square::a2 && source <= Square::h2))

using piece::Type::PAWN;

void MoveList::generate(const Board &board, bool attacks_only) {
    uint8_t src_i = 0, tgt_i = 0;

    const Color color = board.get_side(), colorOther = other(color);

    // pawn moves
    const int add = (color == WHITE) ? +8 : -8;

    U64 bitboard = board.get_bitboard_piece(PAWN, color);
    bitboard_for_each_bit(src_i, bitboard) {
        const auto src = static_cast<Square>(src_i);
        const auto tgt = static_cast<Square>(tgt_i = src_i + add);
        if (!attacks_only && !board.is_square_occupied(tgt)) {
            if (pawn_canPromote(color, src)) {
                list.emplace_back(src, tgt, Move::PKNIGHT);
                list.emplace_back(src, tgt, Move::PBISHOP);
                list.emplace_back(src, tgt, Move::PROOK);
                list.emplace_back(src, tgt, Move::PQUEEN);
            } else {
                list.emplace_back(src, tgt, Move::PQUIET);

                // two ahead
                const auto tgt = static_cast<Square>(tgt_i + add);
                if (pawn_onStart(color, src) && !board.is_square_occupied(tgt))
                    list.emplace_back(src, tgt, Move::DOUBLE);
            }
        }

        // capture
        U64 attack =
            board.get_bitboard_piece_attacks(PAWN, color, src) & board.get_bitboard_color(colorOther);
        bitboard_for_each_bit(tgt_i, attack) {
            const auto tgt = static_cast<Square>(tgt_i);
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
        const Square enpassant = board.get_enpassant();
        if (enpassant != Square::no_sq && board.is_piece_attack_square(PAWN, color, src, enpassant))
            list.emplace_back(src, enpassant, Move::ENPASSANT);
    }

    // All piece move
    for (piece::Type type = piece::KNIGHT; type <= piece::KING; ++type) {
        U64 bitboard = board.get_bitboard_piece(type, color);
        bitboard_for_each_bit(src_i, bitboard) {
            const auto src = static_cast<Square>(src_i);
            U64 attack = board.get_bitboard_piece_moves(type, color, src);
            bitboard_for_each_bit(tgt_i, attack) {
                const auto tgt = static_cast<Square>(tgt_i);
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
    if (color == WHITE) {
        if (!board.is_square_attacked(Square::e1, BLACK)) {
            if (board.get_castle() & Board::Castle::WK) {
                if (!board.is_square_occupied(Square::f1) && !board.is_square_occupied(Square::g1) &&
                    !board.is_square_attacked(Square::f1, BLACK))
                    list.emplace_back(Square::e1, Square::g1, Move::CASTLEK);
            }
            if (board.get_castle() & Board::Castle::WQ) {
                if (!board.is_square_occupied(Square::d1) && !board.is_square_occupied(Square::c1) &&
                    !board.is_square_occupied(Square::b1) && !board.is_square_attacked(Square::d1, BLACK) &&
                    !board.is_square_attacked(Square::c1, BLACK))
                    list.emplace_back(Square::e1, Square::c1, Move::CASTLEQ);
            }
        }
    } else {
        if (!board.is_square_attacked(Square::e8, WHITE)) {
            if (board.get_castle() & Board::Castle::BK) {
                if (!board.is_square_occupied(Square::f8) && !board.is_square_occupied(Square::g8) &&
                    !board.is_square_attacked(Square::f8, WHITE))
                    list.emplace_back(Square::e8, Square::g8, Move::CASTLEK);
            }
            if (board.get_castle() & Board::Castle::BQ) {
                if (!board.is_square_occupied(Square::d8) && !board.is_square_occupied(Square::c8) &&
                    !board.is_square_occupied(Square::b8) && !board.is_square_attacked(Square::d8, WHITE) &&
                    !board.is_square_attacked(Square::c8, WHITE))
                    list.emplace_back(Square::e8, Square::c8, Move::CASTLEQ);
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
