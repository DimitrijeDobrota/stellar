#include "board.hpp"
#include "moves.hpp"
#include "piece.hpp"
#include "utils_cpp.hpp"

#define pawn_canPromote(color, source)                                                                       \
    ((color == Color::WHITE && source >= Square::a7 && source <= Square::h7) ||                              \
     (color == Color::BLACK && source >= Square::a2 && source <= Square::h2))

#define pawn_onStart(color, source)                                                                          \
    ((color == Color::BLACK && source >= Square::a7 && source <= Square::h7) ||                              \
     (color == Color::WHITE && source >= Square::a2 && source <= Square::h2))

#define pawn_promote(source, target, piece, capture)                                                         \
    res.push_back(                                                                                           \
        {move_encode(source, target, &piece, capture, &piece::get(piece::Type::KNIGHT, color), 0, 0, 0),     \
         0});                                                                                                \
    res.push_back(                                                                                           \
        {move_encode(source, target, &piece, capture, &piece::get(piece::Type::BISHOP, color), 0, 0, 0),     \
         0});                                                                                                \
    res.push_back(                                                                                           \
        {move_encode(source, target, &piece, capture, &piece::get(piece::Type::ROOK, color), 0, 0, 0), 0});  \
    res.push_back(                                                                                           \
        {move_encode(source, target, &piece, capture, &piece::get(piece::Type::QUEEN, color), 0, 0, 0), 0});

std::vector<MoveE> move_list_generate(const Board &board) {
    uint8_t src_i, tgt_i;

    Color color = board.get_side();
    Color colorOther = color == Color::BLACK ? Color::WHITE : Color::BLACK;

    std::vector<MoveE> res;
    res.reserve(256);

    // pawn moves
    const piece::Piece &piece = piece::get(piece::Type::PAWN, color);
    U64 bitboard = board.get_bitboard_piece(piece);
    bitboard_for_each_bit(src_i, bitboard) {
        // quiet
        int add = (color == Color::WHITE) ? +8 : -8;
        tgt_i = src_i + add;
        Square src = static_cast<Square>(src_i);
        Square tgt = static_cast<Square>(tgt_i);
        if (!board.is_square_occupied(tgt)) {
            if (pawn_canPromote(color, src)) {
                pawn_promote(src_i, tgt_i, piece, nullptr);
            } else {
                res.push_back({move_encode(src_i, tgt_i, &piece, 0, 0, 0, 0, 0), 0});

                // two ahead
                Square tgt = static_cast<Square>(tgt_i + add);
                if (pawn_onStart(color, src) && !board.is_square_occupied(tgt))
                    res.push_back({move_encode(src_i, tgt_i + add, &piece, 0, 0, 1, 0, 0), 0});
            }
        }

        // capture
        U64 attack = board.get_bitboard_piece_attacks(piece, src) & board.get_bitboard_color(colorOther);
        bitboard_for_each_bit(tgt_i, attack) {
            Square tgt = static_cast<Square>(tgt_i);
            const piece::Piece *capture = board.get_square_piece(tgt);
            if (pawn_canPromote(color, src)) {
                pawn_promote(src_i, tgt_i, piece, capture);
            } else {
                res.push_back({move_encode(src_i, tgt_i, &piece, capture, 0, 0, 0, 0), 0});
            }
        }

        // en passant
        if (board.get_enpassant() != Square::no_sq &&
            board.get_bitboard_piece_attacks(piece, static_cast<Square>(src_i)) &
                (C64(1) << to_underlying(board.get_enpassant())))
            res.push_back({move_encode(src_i, to_underlying(board.get_enpassant()), &piece,
                                       &piece::get(piece::Type::PAWN, colorOther), 0, 0, 1, 0),
                           0});
    }

    // All piece move
    auto type_it = piece::TypeIter().begin();
    for (++type_it; type_it != type_it.end(); ++type_it) {
        const piece::Piece &piece = piece::get(*type_it, color);
        U64 bitboard = board.get_bitboard_piece(piece);
        bitboard_for_each_bit(src_i, bitboard) {
            Square src = static_cast<Square>(src_i);
            U64 attack = board.get_bitboard_piece_attacks(piece, src) & ~board.get_bitboard_color(color);
            bitboard_for_each_bit(tgt_i, attack) {
                Square tgt = static_cast<Square>(tgt_i);
                res.push_back(
                    {move_encode(src_i, tgt_i, &piece, board.get_square_piece(tgt), 0, 0, 0, 0), 0});
            }
        }
    }

    // Castling
    if (color == Color::WHITE) {
        static const piece::Piece &piece = piece::get(piece::Type::KING, Color::WHITE);
        if (board.get_castle() & to_underlying(Board::Castle::WK)) {
            if (!board.is_square_occupied(Square::f1) && !board.is_square_occupied(Square::g1) &&
                !board.is_square_attacked(Square::e1, Color::BLACK) &&
                !board.is_square_attacked(Square::f1, Color::BLACK))
                res.push_back(
                    {move_encode(to_underlying(Square::e1), to_underlying(Square::g1), &piece, 0, 0, 0, 0, 1),
                     0});
        }
        if (board.get_castle() & to_underlying(Board::Castle::WQ)) {
            if (!board.is_square_occupied(Square::d1) && !board.is_square_occupied(Square::c1) &&
                !board.is_square_occupied(Square::b1) &&
                !board.is_square_attacked(Square::e1, Color::BLACK) &&
                !board.is_square_attacked(Square::d1, Color::BLACK))
                res.push_back(
                    {move_encode(to_underlying(Square::e1), to_underlying(Square::c1), &piece, 0, 0, 0, 0, 1),
                     0});
        }
    } else {
        static const piece::Piece &piece = piece::get(piece::Type::KING, Color::BLACK);
        if (board.get_castle() & to_underlying(Board::Castle::BK)) {
            if (!board.is_square_occupied(Square::f8) && !board.is_square_occupied(Square::g8) &&
                !board.is_square_attacked(Square::e8, Color::WHITE) &&
                !board.is_square_attacked(Square::f8, Color::WHITE))
                res.push_back(
                    {move_encode(to_underlying(Square::e8), to_underlying(Square::g8), &piece, 0, 0, 0, 0, 1),
                     0});
        }
        if (board.get_castle() & to_underlying(Board::Castle::BQ)) {
            if (!board.is_square_occupied(Square::d8) && !board.is_square_occupied(Square::c8) &&
                !board.is_square_occupied(Square::b8) &&
                !board.is_square_attacked(Square::e8, Color::WHITE) &&
                !board.is_square_attacked(Square::d8, Color::WHITE))
                res.push_back(
                    {move_encode(to_underlying(Square::e8), to_underlying(Square::c8), &piece, 0, 0, 0, 0, 1),
                     0});
        }
    }

    res.resize(res.size());
    return res;
}
