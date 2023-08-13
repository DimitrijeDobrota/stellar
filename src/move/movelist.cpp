#include "movelist.hpp"

#define pawn_canPromote(color, source)                                                                       \
    ((color == Color::WHITE && source >= Square::a7 && source <= Square::h7) ||                              \
     (color == Color::BLACK && source >= Square::a2 && source <= Square::h2))

#define pawn_onStart(color, source)                                                                          \
    ((color == Color::BLACK && source >= Square::a7 && source <= Square::h7) ||                              \
     (color == Color::WHITE && source >= Square::a2 && source <= Square::h2))

void MoveList::generate(const Board &board) {
    this->clear();

    uint8_t src_i, tgt_i;

    Color color = board.get_side();
    Color colorOther = color == Color::BLACK ? Color::WHITE : Color::BLACK;

    // pawn moves
    const piece::Piece &pawn = piece::get(piece::Type::PAWN, color);
    const int add = (color == Color::WHITE) ? +8 : -8;

    U64 bitboard = board.get_bitboard_piece(pawn);
    bitboard_for_each_bit(src_i, bitboard) {
        const Square src = static_cast<Square>(src_i);
        const Square tgt = static_cast<Square>(tgt_i = src_i + add);
        if (!board.is_square_occupied(tgt)) {
            if (pawn_canPromote(color, src)) {
                push_back(Move(src, tgt, &pawn, nullptr, &piece::get(piece::Type::KNIGHT, color), 0, 0, 0));
                push_back(Move(src, tgt, &pawn, nullptr, &piece::get(piece::Type::BISHOP, color), 0, 0, 0));
                push_back(Move(src, tgt, &pawn, nullptr, &piece::get(piece::Type::ROOK, color), 0, 0, 0));
                push_back(Move(src, tgt, &pawn, nullptr, &piece::get(piece::Type::QUEEN, color), 0, 0, 0));
            } else {
                push_back(Move(src, tgt, &pawn, 0, 0, 0, 0, 0));

                // two ahead
                const Square tgt = static_cast<Square>(tgt_i + add);
                if (pawn_onStart(color, src) && !board.is_square_occupied(tgt))
                    push_back(Move(src, tgt, &pawn, 0, 0, 1, 0, 0));
            }
        }

        // capture
        U64 attack = board.get_bitboard_piece_attacks(pawn, src) & board.get_bitboard_color(colorOther);
        bitboard_for_each_bit(tgt_i, attack) {
            const Square tgt = static_cast<Square>(tgt_i);
            const piece::Piece *capture = board.get_square_piece(tgt);
            if (pawn_canPromote(color, src)) {
                push_back(Move(src, tgt, &pawn, capture, &piece::get(piece::Type::KNIGHT, color), 0, 0, 0));
                push_back(Move(src, tgt, &pawn, capture, &piece::get(piece::Type::BISHOP, color), 0, 0, 0));
                push_back(Move(src, tgt, &pawn, capture, &piece::get(piece::Type::ROOK, color), 0, 0, 0));
                push_back(Move(src, tgt, &pawn, capture, &piece::get(piece::Type::QUEEN, color), 0, 0, 0));
            } else {
                push_back(Move(src, tgt, &pawn, capture, 0, 0, 0, 0));
            }
        }

        // en passant
        const Square enpassant = board.get_enpassant();
        if (enpassant != Square::no_sq && board.is_piece_attack_square(pawn, src, enpassant))
            push_back(Move(src, enpassant, &pawn, &piece::get(piece::Type::PAWN, colorOther), 0, 0, 1, 0));
    }

    // All piece move
    for (const piece::Type type : ++piece::TypeIter()) {
        const piece::Piece &piece = piece::get(type, color);
        U64 bitboard = board.get_bitboard_piece(piece);
        bitboard_for_each_bit(src_i, bitboard) {
            const Square src = static_cast<Square>(src_i);
            U64 attack = board.get_bitboard_piece_moves(piece, src);
            bitboard_for_each_bit(tgt_i, attack) {
                const Square tgt = static_cast<Square>(tgt_i);
                push_back(Move(src, tgt, &piece, board.get_square_piece(tgt), 0, 0, 0, 0));
            }
        }
    }

    // Castling
    if (color == Color::WHITE) {
        static const piece::Piece &piece = piece::get(piece::Type::KING, Color::WHITE);
        if (!board.is_square_attacked(Square::e1, Color::BLACK)) {
            if (board.get_castle() & to_underlying(Board::Castle::WK)) {
                if (!board.is_square_occupied(Square::f1) && !board.is_square_occupied(Square::g1) &&
                    !board.is_square_attacked(Square::f1, Color::BLACK))
                    push_back(Move(Square::e1, Square::g1, &piece, 0, 0, 0, 0, 1));
            }
            if (board.get_castle() & to_underlying(Board::Castle::WQ)) {
                if (!board.is_square_occupied(Square::d1) && !board.is_square_occupied(Square::c1) &&
                    !board.is_square_occupied(Square::b1) &&
                    !board.is_square_attacked(Square::d1, Color::BLACK) &&
                    !board.is_square_attacked(Square::c1, Color::BLACK))
                    push_back(Move(Square::e1, Square::c1, &piece, 0, 0, 0, 0, 1));
            }
        }
    } else {
        static const piece::Piece &piece = piece::get(piece::Type::KING, Color::BLACK);
        if (!board.is_square_attacked(Square::e8, Color::WHITE)) {
            if (board.get_castle() & to_underlying(Board::Castle::BK)) {
                if (!board.is_square_occupied(Square::f8) && !board.is_square_occupied(Square::g8) &&
                    !board.is_square_attacked(Square::f8, Color::WHITE))
                    push_back(Move(Square::e8, Square::g8, &piece, 0, 0, 0, 0, 1));
            }
            if (board.get_castle() & to_underlying(Board::Castle::BQ)) {
                if (!board.is_square_occupied(Square::d8) && !board.is_square_occupied(Square::c8) &&
                    !board.is_square_occupied(Square::b8) &&
                    !board.is_square_attacked(Square::d8, Color::WHITE) &&
                    !board.is_square_attacked(Square::c8, Color::WHITE))
                    push_back(Move(Square::e8, Square::c8, &piece, 0, 0, 0, 0, 1));
            }
        }
    }
}

std::ostream &operator<<(std::ostream &os, const MoveList &list) {
    os << "Size: " << list.list.size();
    for (const auto &moveE : list.list) {
        os << moveE.score << ": " << moveE.move << "\n";
    }
    return os;
}
