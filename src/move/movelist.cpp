#include "movelist.hpp"
#include "piece.hpp"
#include <iomanip>

#define pawn_canPromote(color, source)                                                                       \
    ((color == Color::WHITE && source >= Square::a7 && source <= Square::h7) ||                              \
     (color == Color::BLACK && source >= Square::a2 && source <= Square::h2))

#define pawn_onStart(color, source)                                                                          \
    ((color == Color::BLACK && source >= Square::a7 && source <= Square::h7) ||                              \
     (color == Color::WHITE && source >= Square::a2 && source <= Square::h2))

using piece::Type::BISHOP;
using piece::Type::KING;
using piece::Type::KNIGHT;
using piece::Type::NONE;
using piece::Type::PAWN;
using piece::Type::QUEEN;
using piece::Type::ROOK;

void MoveList::generate(const Board &board) {
    this->clear();

    uint8_t src_i, tgt_i;

    Color color = board.get_side();
    Color colorOther = color == Color::BLACK ? Color::WHITE : Color::BLACK;

    // pawn moves
    const int add = (color == Color::WHITE) ? +8 : -8;

    U64 bitboard = board.get_bitboard_piece(PAWN, color);
    bitboard_for_each_bit(src_i, bitboard) {
        const Square src = static_cast<Square>(src_i);
        const Square tgt = static_cast<Square>(tgt_i = src_i + add);
        if (!board.is_square_occupied(tgt)) {
            if (pawn_canPromote(color, src)) {
                list.push_back({src, tgt, Move::PKNIGHT});
                list.push_back({src, tgt, Move::PBISHOP});
                list.push_back({src, tgt, Move::PROOK});
                list.push_back({src, tgt, Move::PQUEEN});
            } else {
                list.push_back({src, tgt, Move::QUIET});

                // two ahead
                const Square tgt = static_cast<Square>(tgt_i + add);
                if (pawn_onStart(color, src) && !board.is_square_occupied(tgt))
                    list.push_back({src, tgt, Move::DOUBLE});
            }
        }

        // capture
        U64 attack =
            board.get_bitboard_piece_attacks(PAWN, color, src) & board.get_bitboard_color(colorOther);
        bitboard_for_each_bit(tgt_i, attack) {
            const Square tgt = static_cast<Square>(tgt_i);
            if (pawn_canPromote(color, src)) {
                list.push_back({src, tgt, Move::PCKNIGHT});
                list.push_back({src, tgt, Move::PCBISHOP});
                list.push_back({src, tgt, Move::PCROOK});
                list.push_back({src, tgt, Move::PCQUEEN});
            } else {
                list.push_back({src, tgt, Move::CAPTURE});
            }
        }

        // en passant
        const Square enpassant = board.get_enpassant();
        if (enpassant != Square::no_sq && board.is_piece_attack_square(PAWN, color, src, enpassant))
            list.push_back({src, enpassant, Move::ENPASSANT});
    }

    // All piece move
    for (const piece::Type type : ++piece::TypeIter()) {
        U64 bitboard = board.get_bitboard_piece(type, color);
        bitboard_for_each_bit(src_i, bitboard) {
            const Square src = static_cast<Square>(src_i);
            U64 attack = board.get_bitboard_piece_moves(type, color, src);
            bitboard_for_each_bit(tgt_i, attack) {
                const Square tgt = static_cast<Square>(tgt_i);
                list.push_back({src, tgt, board.is_square_occupied(tgt) ? Move::CAPTURE : Move::QUIET});
            }
        }
    }

    // Castling
    if (color == Color::WHITE) {
        if (!board.is_square_attacked(Square::e1, Color::BLACK)) {
            if (board.get_castle() & to_underlying(Board::Castle::WK)) {
                if (!board.is_square_occupied(Square::f1) && !board.is_square_occupied(Square::g1) &&
                    !board.is_square_attacked(Square::f1, Color::BLACK))
                    list.push_back({Square::e1, Square::g1, Move::CASTLEK});
            }
            if (board.get_castle() & to_underlying(Board::Castle::WQ)) {
                if (!board.is_square_occupied(Square::d1) && !board.is_square_occupied(Square::c1) &&
                    !board.is_square_occupied(Square::b1) &&
                    !board.is_square_attacked(Square::d1, Color::BLACK) &&
                    !board.is_square_attacked(Square::c1, Color::BLACK))
                    list.push_back({Square::e1, Square::c1, Move::CASTLEQ});
            }
        }
    } else {
        if (!board.is_square_attacked(Square::e8, Color::WHITE)) {
            if (board.get_castle() & to_underlying(Board::Castle::BK)) {
                if (!board.is_square_occupied(Square::f8) && !board.is_square_occupied(Square::g8) &&
                    !board.is_square_attacked(Square::f8, Color::WHITE))
                    list.push_back({Square::e8, Square::g8, Move::CASTLEK});
            }
            if (board.get_castle() & to_underlying(Board::Castle::BQ)) {
                if (!board.is_square_occupied(Square::d8) && !board.is_square_occupied(Square::c8) &&
                    !board.is_square_occupied(Square::b8) &&
                    !board.is_square_attacked(Square::d8, Color::WHITE) &&
                    !board.is_square_attacked(Square::c8, Color::WHITE))
                    list.push_back({Square::e8, Square::c8, Move::CASTLEQ});
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
